/******************************************************************************

 @File Name : {PROJECT_DIR}/sender.h

 @Creation Date : 30-01-2012

 @Last Modified : Mon 13 Feb 2012 12:46:52 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/


#include "debug.h"
#include "sender.h"
#include "sock_utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>

/*static double gettime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + 1e-6 * t.tv_usec;
}
*/
//
//  the size section must be 6 for sock sender to extract the port and 
//  the s_addr. This version just supports ipv4.
//
//  The worker delegates the connect function to sender
//
int SockSender::connect(ProxyData* data)
{
  ASSERT(data->size == 6);
  struct sockaddr_in connect_end;
  {
    connect_end.sin_family = AF_INET;
    connect_end.sin_port = *(uint16_t*) data->bytes;
    connect_end.sin_addr.s_addr = *(uint32_t*) (data->bytes + sizeof(uint16_t));
  };
  if (::connect(data->fd, (struct sockaddr*) &connect_end, sizeof(sockaddr_in)) < 0) {
    DEBUG("connect: %s\n", strerror(errno));
    return -1;
  }
  free_proxy_data(data);
  return 0;
}


int SockSender::putdata(ProxyData* data)
{
  char* start = data->bytes;
  int left = data->size;
  int block_upper = 1000;
  int result = 0;
  int fd = data->fd;

  while (left > 0) {
    ssize_t ret = send(fd, start, left, MSG_NOSIGNAL);
    if (ret < 0) {
      if (errno == EAGAIN) {
        if (--block_upper <= 0) {
          usleep(1000);
          DEBUG("deadly blocking, give up\n");
          result = -1;
          break;
        }
        continue;
      } else if (errno == EPIPE) {
        result = -1;
        break;
      }
      ERR("error in sending, %s\n", strerror(errno));
      free_proxy_data(data);
      return -1;
    }

    left -= ret;
    start += ret;
  }

  free_proxy_data(data);
  return result;
}

int SockSender::close(ProxyData* data)
{
      free_proxy_data(data);
  return 0;
}

int MailSender::putdata(ProxyData* data)
{
  return ProxySender::putdata(data);
}

void MailSender::flushdata()
{
  wake_cond(wait_lock);
}

int MailSender::connect(ProxyData* data)
{
  putdata(data);
  return 0;
}

int MailSender::close(ProxyData* data)
{
  putdata(data);
  return 0;
}

void MailSender::do_flush_data()
{
  ProxyData* data = NULL;
  ProxyDataSet ds;

  if (buf_size() == 0)
    return ;

  ds.dataset = (ProxyData**) malloc(attachment_cnt_max * sizeof(ProxyData*));
  if (!ds.dataset) {
    DEBUG("no memory, should restart!\n");
    return ;
  }
  ssize_t upper_size = attachment_size_max * 1024;
  ssize_t current_size = 0;
  int current_cnt = 0;
  while (current_size < upper_size && current_cnt < attachment_cnt_max) {
    if ((data = fetch_data()) == NULL)
      break;
    //  Rough estimate
    current_size += data->size + sizeof(*data);
    ds.dataset[current_cnt++] = data;
  }

  ds.data_cnt = current_cnt;
  char tmp_file[1024];

  time_t t = time(0);
  sprintf(tmp_file, "mail_%u_%lu", getpid(), t);
  if (proxy_data_to_file(tmp_file, &ds) < 0) {
    ERR("error in flushing data\n");
    goto end;
  }

  DEBUG("%d packets to sent out\n", ds.data_cnt);

#ifndef USE_MAIL_DAEMON
  char cmd[1024];
  sprintf(cmd, "./sendmail.sh %s %d", tmp_file, 1);
  system(cmd);
#else
  {
    int tmp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in daemon_addr;
    daemon_addr.sin_family = AF_INET;
    daemon_addr.sin_port = htons(8889);
    daemon_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    DEBUG("sending address %x\n", daemon_addr.sin_addr.s_addr);
    if (sendto(tmp_sock, tmp_file, strlen(tmp_file), 0,
          (struct sockaddr*) &daemon_addr, sizeof(daemon_addr)) < 0) {
      ERR("error in sending mail to daemon. Simple solution is"
          "to restart everything. No retry will be performed: %s\n", strerror(errno));
      goto end;
    }
struct timespec t;
clock_gettime(CLOCK_REALTIME, &t);
fprintf(stderr,"send: %lf\n",
    t.tv_sec + 1E-9 * t.tv_nsec);

//	printf("\nsend: %lf\t", gettime());
    ::close(tmp_sock);
  }
#endif

end:
  for (int i = 0; i < ds.data_cnt; i++)
    free_proxy_data(ds.dataset[i]);
  free(ds.dataset);
}

int MailSender::start()
{
  while (1) {
    do_flush_data();
    wait_cond_to(wait_lock, 500000000);
  }
  return 0;
}

//
//  FIXME:
//    use configurable method to access this local port rather than
//    hard code!
//
static const uint16_t kLocalPort = 8900;

int ProxySockSender::connect(ProxyData* pdata)
{
  struct sockaddr_in connect_end;
  {
    connect_end.sin_family = AF_INET;
    connect_end.sin_port = htons(kLocalPort);
    connect_end.sin_addr.s_addr = inet_addr("127.0.0.1");
  };

  if (::connect(pdata->fd, (struct sockaddr*) &connect_end, sizeof(sockaddr_in)) < 0) {
    DEBUG("connect: %s\n", strerror(errno));
    return -1;
  }
  return SockSender::putdata(pdata);
}


