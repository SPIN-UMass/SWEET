/******************************************************************************

 @File Name : {PROJECT_DIR}/worker.h

 @Creation Date : 30-01-2012

 @Last Modified : Mon 13 Feb 2012 12:50:19 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _WORKER_H
#define _WORKER_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#include "tunnel_data.h"
#include "sender.h"
#include "sync.h"

#include <map>

using std::map;

class ProxySender;

class ProxyWorker {

  public:

  //
  // packet type for socks4 worker, must be kCommand or kData
  //
  enum {
    kPacketData = 1,
    kPacketInit = 2,
    kPacketAck = 4,
    kPacketErr = 8, // notify an error status, tear down connection whatever
    kPacketFini = 16
  };

  ProxyWorker(int tag, int client): tag_(tag), is_client_(client) {
    errcode_ = 0;
    lock = new_lock();
    cond = new_wait(tag);
    sender[0] = get_sender(0);
    sender[1] = get_sender(1);
  }

  virtual ~ProxyWorker() {
    free_lock(lock);
    free_wait(cond);
  }

  int tag() const { return tag_; } 
  int state() const { return state_; }
  int errcode() const { return errcode_; }
  bool is_client() const { return is_client_; }
  virtual int process(ProxyData* pdata, int source) { return 0; }

  //
  // return value on processing
  //
  enum {
    kMoreData = 1,
    kOK = 2,
    kFin = 3,
    kErr = 4
  };


  protected:
  LockObject* lock;
  WaitObject* cond;
  ProxySender* sender[2];
  int tag_;
  int state_;
  int errcode_;
  bool is_client_;

  private:
  ProxyWorker(const ProxyWorker& other) {};
  ProxyWorker& operator = (const ProxyWorker& p) { return *this; }

};

class Socks4Worker: public ProxyWorker {

public:
  const static int kUnInit  = 1;
  const static int kGranted = 2; // access allowed
  const static int kClosed   = 3; // it's a closed object waiting for recycle
    
  Socks4Worker(int tag, int client): ProxyWorker(tag, client) { 
    state_ = kUnInit;
    out_order_data_ = NULL;
    //
    //  For deterministic. Could use random format
    //
    seq_[0] = 0;
    seq_[1] = 0;
  }

  ~Socks4Worker() {
    if (out_order_data_) {
      delete out_order_data_;
    }
  }

  int process(ProxyData* pdata, int source);

private:
  int socks4_init(ProxyData* pdata, int source);
  int do_socks4_client_init(ProxyData* pdata, int source);
  int do_socks4_server_init(ProxyData* pdata, int source);
  int socks4_connected(ProxyData* pdata, int source);
  int socks4_close(ProxyData* pdata, int source);
  map <int, ProxyData* > * out_order_data_;
  //
  //  We just use this seq_ at connection time? Can mail system
  //  ensure the order of our packet??
  //
  int32_t seq_[2];
};

class GeneralWorker: public ProxyWorker {
  const static int kUnInit  = 1;
  const static int kGranted = 2; // access allowed
  const static int kClosed   = 3; // it's a closed object waiting for recycle
  
  public:
  GeneralWorker(int tag, int client): ProxyWorker(tag, client) {
    state_ = kUnInit;
  }
  int process(ProxyData* pdata, int source);
};



#endif
