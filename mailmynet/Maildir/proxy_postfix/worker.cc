/******************************************************************************

 @File Name : {PROJECT_DIR}/worker.cc

 @Creation Date : 30-01-2012

 @Last Modified : Mon 13 Feb 2012 12:50:00 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/

#include "debug.h"
#include "sender.h"
#include "worker.h"
#include "tunnel_data.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <utility>
using std::make_pair;

//
//  Socks4 utilities
//
static inline void fill_socks4_init_grant(char* bytes)
{
  bytes[0] = 0x00;
  bytes[1] = 0x5a;
  //
  //  Should be ignored, but we set for tracking
  //
  bytes[2] = 0xee;
  bytes[3] = 0xee;
  *(int*)(&bytes[4]) = 0x19850920;
}

static inline void fill_socks4_init_reject(char* bytes)
{
  bytes[0] = 0x00;
  bytes[1] = 0x5b;
  //
  //  Should be ignored, but we set for tracking
  //
  bytes[2] = 0xee;
  bytes[3] = 0xee;
  *(int*)(&bytes[4]) = 0x19850920;
}

static bool valid_socks4_init_req(char* bytes, int size)
{
  if (size < 9) return false;
  if (bytes[0] != 0x04) return false;
  if (bytes[1] != 0x01) {
    DEBUG("only support connect command\n");
    return false;
  }
  return true;
}

//
//  Don't reference this pdata any more
//
static inline void reject_socks4_request(ProxyData* pdata, int tag,
    ProxySender* sender)
{
  if (pdata->size < 8) {
    free_proxy_data(pdata);
    pdata = new_proxy_data(8);
    if (!pdata) {
      DEBUG("no memory\n");
      return;
    }
  }
  fill_socks4_init_reject(pdata->bytes);
  pdata->size = 8;
  pdata->type = Socks4Worker::kPacketErr;
  pdata->tag = tag;
  pdata->seq = -1;
  sender->putdata(pdata);
}

static inline void grant_socks4_request(ProxyData* pdata, int tag,
    ProxySender* sender)
{
  if (pdata->size < 8) {
    free_proxy_data(pdata);
    pdata = new_proxy_data(8);
    if (!pdata) {
      DEBUG("no memory\n");
      return;
    }
  }
  fill_socks4_init_grant(pdata->bytes);
  pdata->size = 8;
  pdata->type = Socks4Worker::kPacketAck;
  pdata->tag = tag;
  pdata->seq = -1;
  sender->putdata(pdata);
}

int Socks4Worker::process(ProxyData* pdata, int source)
{
  switch (state()) {
    case kUnInit:
      return socks4_init(pdata, source);
    case kGranted:
      return socks4_connected(pdata, source);
    case kClosed: default:
      DEBUG("received data in closed state\n");
      return kErr;
  }
}

int Socks4Worker::socks4_init(ProxyData* pdata, int source)
{
  if (is_client()) pdata->tag = tag_;
  else tag_ = pdata->tag;

  //
  // valid client request
  //
  if (!valid_socks4_init_req(pdata->bytes, pdata->size)) {
    //
    //  Workaround for server side order problem
    //
    if (!is_client()) {
      DEBUG("out of order receive! Are you sure it's safe??\n");
      out_order_data_ = new map<int, ProxyData*>();
      out_order_data_->insert(make_pair(pdata->fd, pdata));
      return kOK;
    }

    DEBUG("can not valid socks4 init\n");
    reject_socks4_request(pdata, tag_, sender[source]);
    state_ = kClosed;
    return kErr;
  }

  ProxyData* ack_data = new_proxy_data(8);
  if (ack_data == NULL) {
    DEBUG("no memory\n");
    reject_socks4_request(pdata, tag_, sender[source]);
    state_ = kClosed;
    return kErr;
  }
  ack_data->size = 8;
  ack_data->fd = pdata->fd;

  //
  //  ACK the initializing side
  //
  grant_socks4_request(ack_data, tag_, sender[source]);

  //
  //  Grant it for performance. Later we withdraw this by violent
  //  approach if it really could not establish
  //

  return is_client()? do_socks4_client_init(pdata, source):
    do_socks4_server_init(pdata, source);
}


int Socks4Worker::do_socks4_client_init(ProxyData* pdata, int source)
{
  int ret = kOK;
  int other = 1 - source;
  pdata->type = kPacketInit;
  pdata->seq = seq_[other]++;
  if (sender[other]->connect(pdata) < 0) {
    DEBUG("can not connect to destination\n");
    state_ = kClosed;
    ret = kErr;
  }
  state_ = kGranted;

  //
  //  Flush out the out of order data
  //
  if (out_order_data_) {
    for (map<int, ProxyData*>::iterator it = out_order_data_->begin();
        it != out_order_data_->end(); ++it) {
      sender[other]->putdata(it->second, seq_[other]++);
    }
    delete out_order_data_;
    out_order_data_ = NULL;
  }

  //free_proxy_data(pdata);
  return ret;
}

int Socks4Worker::do_socks4_server_init(ProxyData* pdata, int source)
{
  //
  //  Compat the sender data to make it valid for sock connect. We
  //  must use memmove, since this is an overlapped copy
  //
  int ret = kOK;
  int other = 1 - source;
  memmove(pdata->bytes, pdata->bytes + 2, 6);
  pdata->size = 6;
  pdata->seq = seq_[other]++;
  //
  //  Connect will free the data
  //
  if (sender[other]->connect(pdata) < 0) {
    DEBUG("can not connect to the destination\n");
    state_ = kClosed;
    ret = kErr;
  }
  state_ = kGranted;
  //free_proxy_data(pdata);
  return ret;
}

int Socks4Worker::socks4_connected(ProxyData* pdata, int source)
{
  //
  //  Must set the tag_ field before processing
  //
  ASSERT(pdata->tag == tag_);

  int ret = kOK;
  //
  //  control data
  //
  if ((pdata->type & (~kPacketData)) != 0) {

    if (pdata->type & kPacketFini)
      ret = kFin;

    if (pdata->type & kPacketErr)
      ret = kErr;

    DEBUG("ret = %d\n", ret);
    if (ret != kOK)
      socks4_close(pdata, source);
    else
      free_proxy_data(pdata);

    //
    //  We have to manually free the data
    //

  } else {
    sender[1 - source]->putdata(pdata, seq_[1 - source]++);
  }
  return ret;
}

int Socks4Worker::socks4_close(ProxyData* pdata, int source)
{
  state_ = kClosed;
  pdata->seq = seq_[1 - source]++;
  return sender[1 - source]->close(pdata);
}

int GeneralWorker::process(ProxyData* pdata, int source)
{
  int other = 1 - source;
  switch (state()) {
    case kUnInit:
      if (sender[other]->connect(pdata) < 0) {
        state_ = kClosed;
        return kErr;
      } else {
        state_ = kGranted;
        return kOK;
      }
    case kClosed: default:
      DEBUG("received data in closed state\n");
      free_proxy_data(pdata);
      return kErr;
    case kGranted:
      break;
  }

  int ret = kOK;
  if ((pdata->type & (~kPacketData)) != 0) {
    if (pdata->type & kPacketFini)
      ret = kFin;

    if (pdata->type & kPacketErr)
      ret = kErr;

    DEBUG("ret = %d\n", ret);
    if (ret != kOK) {
      sender[other]->close(pdata);
      state_ = kClosed;
    } else {
      free_proxy_data(pdata);
    }
  } else {
    sender[other]->putdata(pdata);
  }
  return ret;
}


