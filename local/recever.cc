/******************************************************************************

 @File Name : {PROJECT_DIR}/recever.cc

 @Creation Date : 30-01-2012

 @Last Modified : Mon 13 Feb 2012 12:30:57 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/

#include "debug.h"
#include "recever.h"
#include "worker.h"
#include "tunnel_data.h"
#include "tag.h"
#include "sock_utils.h"

#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string>
#include <sys/inotify.h>
#include <stdint.h>

using std::vector;
using std::string;

/*static double gettime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + 1e-6 * t.tv_usec;
}
*/

static int extract_paths(int ifd, vector<string>& paths)
{
  char buffer[4096];
  int ret = read(ifd, buffer, 4096);

  if (ret < 0 && errno == EAGAIN) {
    return 0;
  }
  if (ret < 0 && errno != EAGAIN) {
    return -1;
  }

  char* start = buffer;
  struct inotify_event* ev = (struct inotify_event*) start;
  int len = ret;
  while (len > 0) {
    start += ev->len + sizeof(*ev);
    len -= ev->len + sizeof(*ev);
    paths.push_back(string((char*) ev->name, ev->len));
    ev = (struct inotify_event*) start;
  }
  return paths.size();
}

//
//  FIXME:
//    These things try to make a pair. That's not correct. We should allow
//    multiple sources and destinations without knowing what they are.
//
static inline int socks_source(int client)
{
  return client? 0: 1;
}
static inline int fs_source(int client)
{
  return client? 1: 0;
}

void ProxyRecver::add_new_meta(int fd, int tag, int type)
{
  metas_[fd] = new MonitorMeta;
  metas_[fd]->fd = fd;
  metas_[fd]->type = type;
  metas_[fd]->tag = tag;
  metas_[fd]->path = NULL;
  metas_[fd]->sent_bytes = 0;
  metas_[fd]->recv_bytes = 0;
  clock_gettime(CLOCK_REALTIME, &metas_[fd]->start_ts);

  //fprintf(stderr, "recv: %lf\n", 
  //  metas_[fd]->start_ts.tv_sec + 1E-9 * metas_[fd]->start_ts.tv_nsec);
  clock_gettime(CLOCK_REALTIME, &metas_[fd]->last_ts);
  tag_to_fd_[tag] = fd;
}

//
//  del_meta must be called before close the fd
//
void ProxyRecver::del_meta(int fd)
{
  MonitorMeta* m = metas_[fd];
  tag_to_fd_.erase(m->tag);

  struct sockaddr_in s_addr;
  socklen_t s_len = sizeof(s_addr);
  if (getsockname(fd, (struct sockaddr*) &s_addr, &s_len) < 0) {
    ERR("can not get socket name: %s\n", strerror(errno));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY),
    s_addr.sin_port = htons(0);
  }

  //struct timespec end_ts;
//  clock_gettime(CLOCK_REALTIME, &end_ts);

  //printf("conn: %x %u %.3lf %"PRId64" %"PRId64"\n", 
  /*printf("conn: %x %u %lf %lf %"PRId64" %"PRId64"\n", 
    ntohl(s_addr.sin_addr.s_addr),
    ntohs(s_addr.sin_port),
    m->start_ts.tv_sec + 1E-9 * m->start_ts.tv_nsec,
    end_ts.tv_sec + 1E-9 * end_ts.tv_nsec,
    m->sent_bytes,
    m->recv_bytes);*/
  delete m;
}


ProxyRecver::ProxyRecver(int client, const char* port, const char* path):is_client_(client), monitor_path_(path)
{
  monitor_set_ = epoll_create(100);

  //
  //  FIXME: Should clean up this function. Too dirty. Separate them in different
  //  helpers
  //
  int iwatch_fd = inotify_init1(IN_NONBLOCK);
  inotify_fd_ = iwatch_fd;
  if (iwatch_fd < 0) {
    ERR("can not create iwatch fd! %s\n", strerror(errno));
    exit(1);
  }

  int wd = inotify_add_watch(iwatch_fd, path, IN_CLOSE_WRITE | IN_MOVED_TO);
  if (wd < 0) {
    ERR("can not create watch fd! %s\n", strerror(errno));
    exit(1);
  }

  //
  //  TODO: Shall we set nonblocking here for watch fd?
  //
  //  Besides, tags for this must be -1. A special tag reserved
  //
  add_new_meta(iwatch_fd, -1, F_CREATE);
  //struct timespec t;
  //clock_gettime(CLOCK_REALTIME, &t);
  //fprintf(stderr, "recv: %lf\n", t.tv_sec + 1E-9 * t.tv_nsec);

  ASSERT(epoll_add_excl_ptr(monitor_set_, iwatch_fd, metas_[iwatch_fd]) == 0);

  if (is_client_) {
    int server_fd = create_tcp_server_socket_v4(port, 64);
    if (server_fd < 0) {
      ERR("can not create server socket! %s\n", strerror(errno));
      exit(1);
    }
    socket_set_reuse(server_fd, 1);
    socket_set_nonblocking(server_fd);

    //
    //  tags for this must be -2. A sepcial tag reserved
    //
    add_new_meta(server_fd, -2, S_ACCEPT);
    server_fd_ = server_fd;

    ASSERT(epoll_add_excl_ptr(monitor_set_, server_fd, metas_[server_fd]) == 0);
  }
}

ProxyRecver::~ProxyRecver()
{
  close(monitor_set_);
  close(inotify_fd_);
  close(server_fd_);
}


//
//  Start a monitor
//
int ProxyRecver::start()
{
  struct epoll_event events[128];
  while (true) {
    int nevent = epoll_wait(monitor_set_, events, 128, 1000);
    if (nevent < 0 && errno != EAGAIN && errno != EINTR) {
      ERR("error in receiving thread: %s\n", strerror(errno));
      return -1;
    }

    for (int i = 0; i < nevent; i++) {
      MonitorMeta* m = (MonitorMeta*) events[i].data.ptr;
      ASSERT(m != NULL);
      on_event(m->fd, m->type, m->tag);
    }
  }
  return 0;
}

int ProxyRecver::on_event(int fd, int type, int tag)
{
  int ret;
  vector<string> paths;
  switch(type)  {
    case F_CREATE:
      ASSERT(tag == -1);
      ret = extract_paths(fd, paths);
      for (int i = 0; i < ret; i++) {
        string abs_path(monitor_path_);
        abs_path.append(paths[i]);
        process_fs_event(abs_path.c_str());
      }
      epoll_mod_excl_ptr(monitor_set_, fd, metas_[fd]);
      break;

    case S_ACCEPT:
      ASSERT(tag == -2);
      process_sock_acc(fd);
      break;

    case S_DATA:
      process_sock_data(fd, tag);
      break;
  }

  return 0;
}

int ProxyRecver::process_fs_event(const char* path)
{
  ProxyDataSet pset;
  if (proxy_data_from_file(path, &pset) < 0) {
    DEBUG("error in reading data set from file\n");
    return -1;
  }
  for (int i = 0; i < pset.data_cnt; i++) {
    int tag = pset.dataset[i]->tag;
    int new_worker = 0;
    int new_fd = -1;
    ProxyWorker* pworker = NULL;
    if (worker_map_.find(tag) == worker_map_.end()) {

      if (is_client_) {
        DEBUG("no tag found in client. Omit this data directly\n");
        free_proxy_data(pset.dataset[i]);
        continue;
      }

      new_worker = 1;
      //
      //  FIXME: Allocate a new fd. It should not happen here. The logic is in chaos
      //
      new_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (new_fd < 0) {
        DEBUG("can not allocate the file descriptor\n");
        continue;
      }

      add_new_meta(new_fd, tag, S_DATA);
      pset.dataset[i]->fd = new_fd;

      //
      //  FIXME: remove this explicit allocate of Socks4Worker. Use
      //  more elegant methods
      //
      //pworker = new Socks4Worker(tag, is_client_);
      pworker = new GeneralWorker(tag, is_client_);
      
      //
      //  omit no memory thing here
      //
      process_new_req(pworker);

    } else {
      pworker = worker_map_[tag];
      pset.dataset[i]->fd = tag_to_fd_[tag];
      metas_[pset.dataset[i]->fd]->recv_bytes += pset.dataset[i]->size;
    }

    //
    //  Process the data. The client use fs as 0, use sock as 1
    //  This whole thing should be redesigned if we want better architecture.
    //
    //  The data would be eaten by worker. Don't reference it anymore
    //
    int ret = pworker->process(pset.dataset[i], fs_source(is_client_));
    if (ret == ProxyWorker::kErr) {
      int fd = tag_to_fd_[tag];
      process_err(fd, socks_source(is_client_));
    } else if (ret == ProxyWorker::kFin) {
      int fd = tag_to_fd_[tag];
      process_fin(fd, socks_source(is_client_));
    }

    if (new_worker) {
      socket_set_nonblocking(new_fd);
      epoll_add_excl_ptr(monitor_set_, new_fd, metas_[new_fd]);
    }

  }
  return 0;
}

int ProxyRecver::process_sock_acc(int fd)
{
  int client_fd = socket_simple_accept_v4(fd);
  if (client_fd < 0 && errno == EAGAIN) {
    return 0;
  } else if (client_fd < 0) {
    ERR("can not accept client connection! %s\n", strerror(errno));
    return 0;
  }
  DEBUG("connection established\n");
  socket_set_nonblocking(client_fd);
  int new_tag = ::allocate_tag();
  //ProxyWorker* pworker = new Socks4Worker(new_tag, is_client_);
  ProxyWorker* pworker = new GeneralWorker(new_tag, is_client_);
  process_new_req(pworker);
  add_new_meta(client_fd, new_tag, S_DATA);
  if (epoll_mod_excl_ptr(monitor_set_, fd, metas_[fd]) < 0) {
    ERR("FATAL, can not add server fd to monitor set: %s\n",
        strerror(errno));
    exit(1);
  }
  if (epoll_add_excl_ptr(monitor_set_, client_fd, metas_[client_fd]) < 0) {
    ERR("can not add new connection to monitor set, close it\n");
    del_meta(client_fd);
    delete pworker;
    worker_map_.erase(new_tag);
    close(client_fd);
    return -1;
  }
  return 0;
}

int ProxyRecver::process_err(int fd, int source, bool notify)
{
  ERR("error in processing proxy data\n");
  return do_process_shutdown(fd, source, ProxyWorker::kPacketErr, notify);
}

int ProxyRecver::process_fin(int fd, int source, bool notify)
{
  DEBUG("process is going to close\n");
  return do_process_shutdown(fd, source, ProxyWorker::kPacketFini, notify);
}

int ProxyRecver::do_process_shutdown(int fd, int source, int reason, bool notify)
{
  MonitorMeta* meta = metas_[fd];
  ASSERT(meta != NULL);
  int tag = meta->tag;
  //
  //  Should notify both side the error. For sock issue, we still just
  //  close it.
  //
  del_meta(fd);
  close(fd);
  ProxyWorker* pworker = worker_map_[tag];
  worker_map_.erase(tag);

  //
  //  NOTE:
  //    the notification has been done in the worker logic if the 
  //    notify field is false
  //
  if (notify) {
    ProxyData* pdata = new_proxy_data(0);
    if (!pdata) {
      DEBUG("no memory\n");
      return ProxyWorker::kErr;
    }
    pdata->type = reason;
    pdata->tag = tag;
    pdata->size = 0;
    pworker->process(pdata, source);
  }
  delete pworker;
  return 0;
}

int ProxyRecver::process_sock_data(int fd, int tag)
{
  ProxyData* pdata = NULL;
  //
  //  FIXME:
  //
  //    This version successfully tightly coupled the whole
  //    logics. It can work, but no one can modify or extend
  //    it. Push it down, redesign the structure.
  //
  int ret = proxy_data_from_sock(fd, &pdata);
  int source = socks_source(is_client_);
  if (ret < 0) {
    if (errno == EINTR || errno == EAGAIN) return 0;
    DEBUG("error in receiving one data: %s\n", strerror(errno));
    return process_err(fd, source, true);
  } else if (ret == 0) {
    free_proxy_data(pdata);
    return process_fin(fd, source, true);
  }

  pdata->type |= ProxyWorker::kPacketData;
  MonitorMeta* meta = metas_[fd];
  pdata->tag = meta->tag;
  ProxyWorker* pworker = worker_map_[meta->tag];
  epoll_mod_excl_ptr(monitor_set_, fd, metas_[fd]);
  meta->recv_bytes += pdata->size;

  ret = pworker->process(pdata, source);
  if (ret == ProxyWorker::kErr) {
    process_err(fd, source);
  } else if (ret == ProxyWorker::kFin) {
    process_fin(fd, source);
  }
  return 0;
}

int ProxyRecver::process_new_req(ProxyWorker* pworker)
{
  worker_map_[pworker->tag()] = pworker;
  return 0;
}

