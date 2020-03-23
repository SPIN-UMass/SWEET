/******************************************************************************

 @File Name : {PROJECT_DIR}/client.cc

 @Creation Date : 30-01-2012

 @Last Modified : Wed 08 Feb 2012 01:42:21 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/


#include "debug.h"
#include "recever.h"
#include "sender.h"
#include "worker.h"
#include <pthread.h>
#include <stdio.h>

static const char* kPort = "8888";
static const char* kMailDir = "/tmp/in_mails/client/";
static const char* kMailAddr = "mail.google.com";
static const char* kMailUser = "wzhou10";
static const char* kMailPass = "test";

static SockSender* ssender = NULL;
static MailSender* msender = NULL;

ProxySender* get_sender(int source)
{
  ASSERT(msender && ssender);
  if (source == 0)
    return ssender;
  else
    return msender;
}

static void* recever_thread(void* arg)
{
  ProxyRecver r(1, kPort, kMailDir);
  if (r.start() < 0)
    DEBUG("receiver has error!\n");
  return NULL;
}

static void* mail_sender_thread(void* arg)
{
  if (msender->start() < 0)
    DEBUG("mail sender has error!\n");
  return NULL;
}

static void* sock_sender_thread(void* arg)
{
  ssender->start();
  return NULL;
}

int main()
{
  char buf[1024];
  sprintf(buf, "mkdir -p %s", kMailDir);
  system(buf);

  ssender = new SockSender();
  msender = new MailSender(kMailAddr, kMailUser, kMailPass, 2000);

  pthread_t p1, p2, p3;
  if (pthread_create(&p1, NULL, recever_thread, NULL) < 0) {
    ERR("can not start recever thread\n");
    return -1;
  }

  if (pthread_create(&p2, NULL, mail_sender_thread, NULL) < 0) {
    ERR("can not start mail sender thread\n");
    return -1;
  }

  if (pthread_create(&p3, NULL, sock_sender_thread, NULL) < 0) {
    ERR("can not start sock sender thread\n");
    return -1;
  }

  //sleep(40);
  //exit(0);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  pthread_join(p3, NULL);

  return msender->start();
}




