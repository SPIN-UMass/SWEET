/******************************************************************************

 @File Name : {PROJECT_DIR}/test-client-connect.cc

 @Creation Date : 07-02-2012

 @Last Modified : Wed 08 Feb 2012 02:27:02 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/

#include "debug.h"
#include "sock_utils.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
  int cfd = create_tcp_client_socket_v4("localhost", "8888");

  char init_msg[] = {0x04, 0x01, 0x00, 0x50, 0xa2, 0x44, 0x6f, 0xa6, 0x00};


  if (send(cfd, init_msg, 9, MSG_NOSIGNAL) != 9) {
    ERR("send error\n");
    ASSERT(0);
  }

  if (recv(cfd, init_msg, 8, 0) != 8) {
    ERR("recv error\n");
    ASSERT(0);
  }

  ASSERT(init_msg[0] == 0);
  ASSERT(init_msg[1] == 0x5a || init_msg[1] == 0x5b);


  char buf[1024];
  int ret =recv(cfd, buf, 1024, 0);

  while (ret > 0) {
    for (int i = 0; i < ret; i++) {
      printf("%x ", buf[i]);
    }
    printf("\n\n");
    ret = recv(cfd, buf, 1024, 0);
  }
  printf("received: %d\n", ret);

  sleep(30);

  printf("check /tmp/in-mails/server to see if test passed\n");
  return 0;
}

