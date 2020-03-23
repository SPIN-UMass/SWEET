/******************************************************************************

 @File Name : {PROJECT_DIR}/tunnel_data.h

 @Creation Date : 30-01-2012

 @Last Modified : Wed 08 Feb 2012 10:37:45 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _TUNNEL_DATA_H
#define _TUNNEL_DATA_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct ProxyData {
    int tag;
    int type;
    int fd;
    int seq;
    int size;
    char bytes[0];
  } ProxyData;

  typedef struct ProxyDataSet {
    struct ProxyData** dataset;
    int data_cnt;
  } ProxyDataSet;

  static inline ProxyData* new_proxy_data(int size)
  {
    return (ProxyData*) malloc(sizeof(ProxyData) + size);
  }

  static inline ProxyDataSet* new_proxy_data_set()
  {
    return (ProxyDataSet*) malloc(sizeof(ProxyDataSet));
  }

  static inline void free_proxy_data(ProxyData* pdata)
  {
    free(pdata);
  }

  static inline void free_proxy_data_set(ProxyDataSet* pset)
  {
    free(pset);
  }

  int proxy_data_to_file(const char* file_name, const ProxyDataSet* s);
  int proxy_data_from_file(const char* file_name, ProxyDataSet* ns);
  int proxy_data_from_sock(int fd, ProxyData** ns);

//  need not these two currently
//  int proxy_data_to_mem(const ProxyDataSet* s, char** mem_ptr, int* sz);
//  int proxy_data_from_mem(const char* mem, int sz, ProxyDataSet* ns);

#ifdef __cplusplus
}
#endif

#endif
