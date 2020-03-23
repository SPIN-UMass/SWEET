/******************************************************************************

 @File Name : {PROJECT_DIR}/recever.h

 @Creation Date : 30-01-2012

 @Last Modified : Wed 08 Feb 2012 11:38:41 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _RECEVER_H
#define _RECEVER_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include <time.h>
#include <map>
#include <string>
using std::map;
using std::string;

class ProxyWorker;
struct ProxyDataSet;

class ProxyRecver {

  struct MonitorMeta {
    int fd;
    int tag;
    int type;
    char* path;
    //
    //  Statistics
    //
    struct timespec start_ts;
    struct timespec last_ts;
    int64_t sent_bytes;
    int64_t recv_bytes;
  };

  public:
    ProxyRecver(int is_client, const char* port, const char* monitor_path);
    ~ProxyRecver(); 

    // this method never returns. It is a daemon like
    int start(); 

    //
    //  Possible types on receiving source
    //
    //  FIXME: this is actually code like style. Can use some other
    //  approach to decouple the logic. Receiver monitor actually
    //  needs know nothing how data is received and parserd. Call
    //  back function might be a good try
    //
    enum {
      F_CREATE,
      S_ACCEPT,
      S_DATA
    };


  protected:
    virtual int on_event(int fd, int type, int tag);

  private:

    int process_fs_event(const char* path);
    int process_sock_acc(int fd);
    int process_sock_data(int fd, int tag);
    int process_new_req(ProxyWorker* pworker);
    int process_err(int fd, int source, bool notify=false);
    int process_fin(int fd, int source, bool notify=false);
    int do_process_shutdown(int fd, int source, int reason, bool notify = false);
    void add_new_meta(int fd, int tag, int type);
    void del_meta(int fd);

    map <int, ProxyWorker*> worker_map_;
    //
    //  TODO: In the future, if the load is too heavy, define
    //  some load balance argument to make this run better
    //
    int monitor_set_;
    int is_client_;
    int inotify_fd_;
    int server_fd_;
    string monitor_path_;
    map <int, int> tag_to_fd_;
    map <int, MonitorMeta*> metas_;
};

#endif
