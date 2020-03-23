/******************************************************************************

 @File Name : {PROJECT_DIR}/tunnel_data.cc

 @Creation Date : 31-01-2012

 @Last Modified : Wed 08 Feb 2012 11:46:49 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/

#include "debug.h"
#include "tunnel_data.h"

#include <vector>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>

using std::vector;

const static int32_t kProxyDataTag = 0x19850920;
const static int32_t kProxyDataSetTag = 0x19871123;

static int stable_write(int fd, char* bytes, int sz)
{
  char* start = bytes;
  ssize_t left = sz;
  while (left > 0) {
    ssize_t ret = write(fd, start, left);
    if (ret < 0 && errno != EAGAIN && errno != EINTR) {
      DEBUG("writing: %s\n", strerror(errno));
      return -1;
    } else if (ret == 0) {
      DEBUG("writing 0 byte\n");
    }
    left -= ret;
    start += ret;
  }
  return 1;
}

static int stable_read(int fd, char* bytes, int sz)
{
  char* start = bytes;
  ssize_t left = sz;
  while (left > 0) {
    ssize_t ret = read(fd, start, left);
    if (ret < 0 && errno != EAGAIN && errno != EINTR) {
      DEBUG("reading: %s\n", strerror(errno));
      return -1;
    } else if (ret < 0) {
      continue;
    } else if (ret == 0) {
      return 0;
    }
    left -= ret;
    start += ret;
  }
  return 1;
}


//
//  When modifying the ProxyData, modify 4 here
//
static const int kProxyDataMember = 5;

static int do_write_proxy_data(int fd, const ProxyData* data)
{
  char meta_buf[64] = {0};
  int32_t* write_ptr = (int32_t*) meta_buf;

  *write_ptr++ = htonl(kProxyDataTag);
  *write_ptr++ = htonl(data->size);
  *write_ptr++ = htonl(data->type);
  *write_ptr++ = htonl(data->tag);
  *write_ptr++ = htonl(data->seq);

  int meta_size = (char*) write_ptr - meta_buf;
  if (stable_write(fd, meta_buf, meta_size) < 0)
    return -1;

  if (stable_write(fd, (char*) data->bytes, data->size) < 0)
    return -1;

  return 0;
}

static int do_write_proxy_data(const ProxyData* data, char** mem, int* sz)
{
  char* mem_buf = (char*) malloc(data->size + kProxyDataMember * sizeof(int32_t));
  if (mem_buf == NULL) {
    DEBUG("no memory\n");
    return -1;
  }

  char* meta_buf = mem_buf;
  *meta_buf++ = htonl(kProxyDataTag);
  *meta_buf++ = htonl(data->size);
  *meta_buf++ = htonl(data->type);
  *meta_buf++ = htonl(data->tag);
  *meta_buf++ = htonl(data->seq);
  memcpy(meta_buf, data->bytes, data->size);
  *mem = mem_buf;
  *sz = data->size + kProxyDataMember * sizeof(int32_t);

  return 0;
}

static int do_read_proxy_data(int fd, ProxyData** data_ptr)
{
  char meta_buf[128];

  //
  //  == 0 means EOF, that's not correct
  //
  if (stable_read(fd, meta_buf, kProxyDataMember * sizeof(int32_t)) <= 0) {
    return -1;
  }

  int32_t* read_ptr = (int32_t*) meta_buf;

  int keytag    = ntohl(*read_ptr++);
  if (keytag != kProxyDataTag) {
    DEBUG("tag not matching\n");
    return -1;
  }
  int data_size = ntohl(*read_ptr++);

  DEBUG("allocated memory: %d\n", data_size);
  ProxyData* pdata = new_proxy_data(data_size);
  if (pdata == NULL) {
    DEBUG("no memory\n");
    return -1;
  }
  pdata->type    = ntohl(*read_ptr++);
  pdata->tag     = ntohl(*read_ptr++);
  pdata->seq     = ntohl(*read_ptr++);
  pdata->size    = data_size;

  int is_end = stable_read(fd, (char*) pdata->bytes, pdata->size);
  if (is_end < 0) {
    DEBUG("end of file mark: %d\n", is_end);
    free_proxy_data(pdata);
    return -1;
  }
  *data_ptr = pdata;

  return is_end;
}

static int do_read_proxy_data(char* mem, int sz, ProxyData** data)
{
  int32_t* read_ptr = (int32_t*) mem;

  int keytag    = ntohl(*read_ptr++);
  if (keytag != kProxyDataTag) {
    DEBUG("tag not matching\n");
    return -1;
  }
  int data_size = ntohl(*read_ptr++);

  if (sz != kProxyDataMember * sizeof(int32_t) + data_size) {
    DEBUG("size mismatch!\n");
    return -1;
  }

  ProxyData* pdata = new_proxy_data(data_size);
  if (pdata == NULL) {
    DEBUG("no memory\n");
    return -1;
  }
  pdata->type    = ntohl(*read_ptr++);
  pdata->tag     = ntohl(*read_ptr++);
  pdata->seq     = ntohl(*read_ptr++);
  pdata->size    = data_size;

  memcpy(pdata->bytes, (char*) (read_ptr), data_size);
  *data = pdata;

  return 1;
}

int proxy_data_to_file(const char* file_name, const ProxyDataSet* s)
{
  int fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if (fd < 0) {
    DEBUG("can not open %s: %s\n", file_name, strerror(errno));
    return -1;
  }

  int32_t header[2] = {
    htonl(kProxyDataSetTag),
    htonl(s->data_cnt)
  };
  if (stable_write(fd, (char*) header, sizeof(header)) < 0) {
    DEBUG("writing data header: %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  for (int i = 0; i < s->data_cnt; i++) {
    if (do_write_proxy_data(fd, s->dataset[i]) < 0) {
      DEBUG("writing %d item\n", i);
      close(fd);
      return -1;
    }
  }
  close(fd);
  return 0;
}

int proxy_data_from_file(const char* file_name, ProxyDataSet* ns)
{
  int fd = open(file_name, O_RDONLY);
  if (fd < 0) {
    DEBUG("can not open %s: %s\n", file_name, strerror(errno));
    return -1;
  }

  int32_t header[2];

  int ret = stable_read(fd, (char*) header, sizeof(header));
  if (ret < 0) {
    ERR("reading data set header: %s\n", strerror(errno));
    close(fd);
    return -1;
  } else if(ret == 0) {
    ERR("reading data set header: unexpected EOF\n");
    close(fd);
    return -1;
  }

  header[0] = ntohl(header[0]);
  header[1] = ntohl(header[1]);

  if (header[0] != kProxyDataSetTag) {
    ERR("data set header tag mismatch!\n");
    close(fd);
    return -1;
  }

  if (header[1] < 0) {
    ERR("data set format error, negative number of data\n");
    close(fd);
    return -1;
  }

  ns->data_cnt = header[1];
  if (ns->data_cnt == 0) {
    DEBUG("no data read\n");
    close(fd);
    return 0;
  }

  ns->dataset = (ProxyData**) malloc(sizeof(ProxyData*) * ns->data_cnt);
  if (!ns->dataset) {
    ERR("no memory\n");
    close(fd);
    return -1;
  }

  for (int i = 0; i < ns->data_cnt; i++) {
    int ret = do_read_proxy_data(fd, &ns->dataset[i]);
    if (ret < 0 || (i < ns->data_cnt - 1 && ret == 0)) {
      DEBUG("error reading one data");
      if (ret == 0) DEBUG(": unexpected EOF");
      DEBUG("\n");
      while (i >= 0)
        free_proxy_data(ns->dataset[i--]);
      free(ns->dataset);
      close(fd);
      return -1;
    }
  }
  close(fd);

  return 0;
}

int proxy_data_from_sock(int fd, ProxyData** data_ptr)
{
  //
  //  FIXME: A piece of SHIT. The logic to read things should be determined
  //  by the workers, rather than a general style
  //

  //
  //  Try you best to read in something. MTU is usually 1500, so we try
  //  something looks good
  //
  char buf[1400];
  int ret = recv(fd, buf, 1400, 0);
  if (ret < 0) {
    //
    //  Don't know if it's error, might just be nonblocking case
    //
    return -1;
  } else if (ret == 0) {
    DEBUG("other side has closed\n");
    return 0;
  }
  ProxyData* pdata = new_proxy_data(ret);
  if (pdata == NULL) {
    ERR("no memory\n");
    return -1;
  }
  pdata->seq = 0;
  pdata->type = 0;
  pdata->size = ret;
  pdata->fd   = fd;
  memcpy(pdata->bytes, buf, ret);
  *data_ptr = pdata;
  return ret;
}


//int proxy_data_to_mem(const ProxyDataSet* s, char** mem_ptr, int* sz)
//{
//}
//int proxy_data_from_mem(const char* mem, int sz, ProxyDataSet* ns);
