/******************************************************************************

 @File Name : {PROJECT_DIR}/sender.h

 @Creation Date : 30-01-2012

 @Last Modified : Mon 13 Feb 2012 12:50:43 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _SENDER_H
#define _SENDER_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include "sync.h"
#include "tunnel_data.h"
#include <list>
#include <string>
using std::list;
using std::string;


class ProxySender {

  public:
    ProxySender() { buf_lock_ = new_lock(); }

    ~ProxySender() { free_lock(buf_lock_); }

    //
    //  For those who needs a daemon thread
    //
    virtual int start() {
      return 0;
    }
    //
    //  Allow connect the other side
    //
    virtual int connect(ProxyData* pdata){
      return 0;
    }
    virtual int putdata(ProxyData* pdata) {
      syn_lock(buf_lock_);
      snd_buf_.push_back(pdata);
      syn_unlock(buf_lock_);
      return 0;
    }

    virtual int putdata(ProxyData* pdata, int seq)
    {
      pdata->seq = seq;
      putdata(pdata);
      return 0;
    }

    virtual int close(ProxyData* pdata) {
      return 0;
    }
    //
    //  This method would flush the buffer;
    //
    virtual void flushdata() { }

  protected:

    ProxyData* fetch_data() {
      ProxyData* ret = NULL;
      syn_lock(buf_lock_);
      if (snd_buf_.size() != 0) {
        ret = snd_buf_.front();
        snd_buf_.pop_front();
      }
      syn_unlock(buf_lock_);
      return ret;
    }

    size_t buf_size() const {
      return snd_buf_.size();
    }

    LockObject* buf_lock_;

  private:
    list<ProxyData*> snd_buf_;
};

class MailSender:public ProxySender {

  public:
  MailSender(const string& addr, const string& user, const string& pass,
      int att_bound):
    ProxySender(), mail_addr(addr), mail_user(user), mail_pass(pass),
    attachment_size_max(att_bound), attachment_cnt_max(512)
  {
    wait_lock = new_wait(0);
  }

  //
  //  The data should not be referenced any more, since they will be released
  //  later by sender
  //
  int connect(ProxyData* pdata);
  int putdata(ProxyData* pdata);
  int close(ProxyData* pdata);
  void flushdata();
  int start();

  private:
  void do_flush_data();
  string mail_addr;
  string mail_user;
  string mail_pass;
  int attachment_size_max;
  int attachment_cnt_max;
  WaitObject* wait_lock;
};

class SockSender: public ProxySender {

  public:
    SockSender(): ProxySender() {}

    virtual int connect(ProxyData* pdata);
    int putdata(ProxyData* pdata);
    int close(ProxyData* pdata);
    void flushdata() { }

  private:
};

class ProxySockSender: public SockSender {
  public:
    ProxySockSender(): SockSender() {
    }
    int connect(ProxyData* pdata);

};

extern ProxySender* get_sender(int source);


#endif
