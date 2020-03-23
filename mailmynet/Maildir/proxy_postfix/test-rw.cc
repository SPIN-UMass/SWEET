/******************************************************************************

 @File Name : {PROJECT_DIR}/test_rw.cc

 @Creation Date : 07-02-2012

 @Last Modified : Wed 08 Feb 2012 10:45:29 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/

#ifndef _DEBUG
#define _DEBUG
#endif

#include "debug.h"
#include "tunnel_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static void comp(ProxyData* p1, ProxyData* p2)
{
  ASSERT (
    p1->size == p2->size &&
    p1->tag == p2->tag &&
    p1->type == p2->type &&
    p1->seq == p2->seq &&
    memcmp(p1->bytes, p2->bytes, p1->size) == 0
  );
}


int main()
{
  char bytes[] = "test small bytes, we love tunnels";
  char large_bytes[4096];
  int tag[] = {3, 31, 511, 4097, 65535};
  int type[] = {1, 2, 3, 4};
  memset(large_bytes, 'z', 4095);
  large_bytes[4095] = '\0';

  ProxyDataSet ns;
  ns.dataset = (ProxyData**) malloc(sizeof(ProxyData*) * 10);

  ProxyData* d0 = new_proxy_data(0);
  d0->size = 0;
  d0->tag = tag[0];
  d0->type = type[0];
  d0->seq  = 1111;
  ProxyData* d1 = new_proxy_data(1);
  d1->size = 1;
  d1->tag = tag[1];
  d1->type = type[1];
  d1->seq = 2222;
  memcpy(d1->bytes, bytes, 1);

  ProxyData* d2 = new_proxy_data(10);
  d2->size = 10;
  d2->tag = tag[2];
  d2->type = type[2];
  d2->seq = 3333;
  memcpy(d2->bytes, bytes, 10);

  ProxyData* d3 = new_proxy_data(2000);
  d3->size = 2000;
  d3->tag = tag[3];
  d3->type = type[3];
  d3->seq = 4444;
  memcpy(d3->bytes, large_bytes, 2000);

  //
  // test write
  //
  ns.dataset[0] = d0;
  ns.dataset[1] = d1;
  ns.dataset[2] = d2;
  ns.dataset[3] = d3;
  ns.data_cnt = 4;

  if (proxy_data_to_file("test1_tmp", &ns) < 0) {
    ERR("failed write\n");
    ASSERT(0);
  }
  
  struct stat buf;
  if (stat("test1_tmp", &buf) < 0) {
    ERR("failed stat\n");
    ASSERT(0);
  }
  if (buf.st_size != (2 * sizeof(int) + 0 + 1 + 10 + 2000 + 4 * 5 * sizeof(int))) {
    ERR("size mismatch for writing\n");
    ASSERT(0);
  }

  ProxyDataSet ns1;
  int ret = proxy_data_from_file("test1_tmp", &ns1);
  if (ret < 0){
    DEBUG("failed read\n");
    ASSERT(0);
  }

  //
  // test read
  //
  ASSERT(ns.data_cnt == ns1.data_cnt);
  for (int i = 0; i < ns.data_cnt; i++) {
    for (int j = 0; j < ns1.data_cnt; j++) if (ns.dataset[i]->tag == ns1.dataset[j]->tag) {
      comp(ns.dataset[i], ns1.dataset[j]);
      break;
    }
  }

  for (int i = 0; i < ns1.data_cnt; i++) {
    free_proxy_data(ns1.dataset[i]);
    free_proxy_data(ns.dataset[i]);
  }
  free(ns1.dataset);
  free(ns.dataset);
  unlink("test1_tmp");

  fprintf(stderr, "test passed\n");
  return 0;
}
