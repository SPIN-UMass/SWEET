/******************************************************************************

 @File Name : {PROJECT_DIR}/sock_utils.h

 @Creation Date : 30-01-2012

 @Last Modified : Tue 07 Feb 2012 10:53:59 PM CST

 @Created By: Zhai Yan

 @Purpose :

*******************************************************************************/
#ifndef ZY_SOCK_UTILS_H
#define ZY_SOCK_UTILS_H


#include "debug.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _DEBUG
static inline void socket_check_type(int s, int type)
{
  int given_type;
  socklen_t type_size = sizeof(int);
  if (getsockopt(s, SOL_SOCKET, SO_TYPE, &given_type, &type_size) < 0 ||
      given_type != type) {
    DEBUG("socket type mismatching!: %d-%d\n", type, given_type);
    exit(-1);
  }
}

#else
#define socket_check_type(s, type) 0
#endif

static inline int socket_set_reuse(int s, int flag)
{
  return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
}

static inline void tcp_check_type(int s)
{
  socket_check_type(s, SOCK_STREAM);
}

static inline void udp_check_type(int s)
{
  socket_check_type(s, SOCK_DGRAM);
}


static inline struct addrinfo* create_addr_info(const char* node_info,
                                                const char* service_info,
                                                int         family,
                                                int         sock_type,
                                                int         protocol,
                                                int         flag)
{
  struct addrinfo hints;
  bzero(&hints, sizeof(hints));
  hints.ai_family         = family;
  hints.ai_socktype       = sock_type;
  hints.ai_protocol       = protocol;

  struct addrinfo* result_info = NULL;
  if (getaddrinfo(node_info, service_info, &hints, &result_info) < 0) {
    ERR("creating the address information: %s\n", strerror(errno));
    return NULL;
  }
  return result_info;
}

static int create_client_socket(int family, int socktype, const char* node_info,
                             const char* service_info)
{
  int s = socket(family, socktype, 0);
  if (s < 0) {
    ERR("creating the socket: %s\n", strerror(errno));
    return -1;
  }

  struct addrinfo* result_info = create_addr_info(node_info,
      service_info,
      family,
      socktype,
      0,
      0
      );
  if (result_info == NULL) {
    return -1;
  }

  if (connect(s, result_info->ai_addr, result_info->ai_addrlen) < 0) {
    freeaddrinfo(result_info);
    ERR("client creating: %s\n", strerror(errno));
    return -1;
  }
  freeaddrinfo(result_info);
  return s;
}

/*
 *  Construct a tcp client socket, the socket type is checked
 */
static int create_tcp_client_socket_v4(const char* node_info,
                                           const char* service_info)
{
    return create_client_socket(AF_INET, SOCK_STREAM, node_info, service_info);
}

static int create_tcp_client_socket_v6(const char* node_info,
                                           const char* service_info)
{
    return create_client_socket(AF_INET6, SOCK_STREAM, node_info, service_info);
}

static int create_udp_client_socket_v4(const char* node_info,
                                           const char* service_info)
{
    return create_client_socket(AF_INET, SOCK_DGRAM, node_info, service_info);
}

static int create_udp_client_socket_v6(const char* node_info,
                                           const char* service_info)
{
    return create_client_socket(AF_INET6, SOCK_DGRAM, node_info, service_info);
}


static int create_tcp_server_socket(int family, int socktype, int backlog,
    const char* service_info)
{
  int s = socket(family, socktype, 0);
  if (s < 0) {
    DEBUG("error in creating the socket: %s\n", strerror(errno));
    return -1;
  }

  struct addrinfo* result_info = create_addr_info(NULL,
                                                  service_info,
                                                  family,
                                                  socktype,
                                                  0,
                                                  AI_PASSIVE
                                                  );

  if (result_info == NULL) {
    DEBUG("tcp server creating: %s\n", strerror(errno));
    return -1;
  }

  /*
   *  We default to reuse the address!
   */
  if (socket_set_reuse(s, 1) < 0) {
    DEBUG("tcp server reusing address: %s\n", strerror(errno));
    return -1;
  }

  if (bind(s, result_info->ai_addr, result_info->ai_addrlen) < 0) {
    freeaddrinfo(result_info);
    DEBUG("tcp server creating: %s\n", strerror(errno));
    return -1;
  }

  freeaddrinfo(result_info);
  if (listen(s, backlog) < 0) {
    DEBUG("tcp listening: %s\n", strerror(errno));
    return -1;
  }
  return s;
}

static int create_udp_server_socket(int family, int socktype,
                                    const char* service_info)
{
  int s = socket(family, socktype, 0);
  if (s < 0) {
    ERR("error in creating the socket: %s\n", strerror(errno));
    return -1;
  }

  struct addrinfo* result_info = create_addr_info(NULL,
                                                  service_info,
                                                  family,
                                                  socktype,
                                                  0,
                                                  AI_PASSIVE
                                                  );
  if (result_info == NULL) {
    ERR("tcp server creating: %s\n", strerror(errno));
    return -1;
  }
  if (bind(s, result_info->ai_addr, result_info->ai_addrlen) < 0) {
    freeaddrinfo(result_info);
    ERR("tcp server creating: %s\n", strerror(errno));
    return -1;
  }
  freeaddrinfo(result_info);
  return s;
}

static inline int create_tcp_server_socket_v4(const char* service_info,
    int backlog)
{
  return create_tcp_server_socket(AF_INET, SOCK_STREAM, backlog, service_info);
}

static inline int create_tcp_server_socket_v6(const char* service_info,
    int backlog)
{
  return create_tcp_server_socket(AF_INET6, SOCK_STREAM, backlog, service_info);
}

static inline int create_udp_server_socket_v4(const char* service_info)
{
  return create_udp_server_socket(AF_INET, SOCK_DGRAM, service_info);
}

static inline int create_udp_server_socket_v6(const char* service_info)
{
  return create_udp_server_socket(AF_INET6, SOCK_DGRAM, service_info);
}

static inline int socket_simple_accept_v4(int fd)
{
  struct sockaddr_in addr;
  socklen_t slen = sizeof(addr);
  return accept(fd, (struct sockaddr*) &addr, &slen);
}

static inline int socket_simple_accept_v6(int fd)
{
  struct sockaddr_in6 addr;
  socklen_t slen = sizeof(addr);
  return accept(fd, (struct sockaddr*) &addr, &slen);
}

/*
 *  Options
 */

static inline int tcp_set_linger(int s, int second)
{
  tcp_check_type(s);
  struct linger ltime;
  {
    ltime.l_onoff  = 1;
    ltime.l_linger = second;
  };
  return setsockopt(s, SOL_SOCKET, SO_LINGER, &ltime, sizeof(ltime));
}

static inline int tcp_set_keepalive(int s, int flag)
{
  tcp_check_type(s);
  return setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
}

static inline int tcp_set_oob(int s, int flag)
{
  tcp_check_type(s);
  return setsockopt(s, SOL_SOCKET, SO_OOBINLINE, &flag, sizeof(flag));
}

static inline int tcp_set_nagle(int s, int flag)
{
  tcp_check_type(s);
  return setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}


static inline int socket_set_sndtimo(int s, int second)
{
  struct timeval timeo;
  {
    timeo.tv_sec   = second;
    timeo.tv_usec  = 0;
  };
  return setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
}

static inline int socket_set_rcvtimo(int s, int second)
{
  struct timeval timeo;
  {
    timeo.tv_sec   = second;
    timeo.tv_usec  = 0;
  };
  return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
}

static inline int socket_set_bind(int s, const char* device_name)
{
  return 0;
}

static inline int socket_set_rcvbuf(int s, int size)
{
  return setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

static inline int socket_set_sndbuf(int s, int size)
{
  return setsockopt(s, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

static inline int socket_set_nonblocking(int s)
{
  int fstatus = fcntl(s, F_GETFL);
  return fcntl(s, F_SETFL, fstatus | O_NONBLOCK);
}

static int inline epoll_add_excl_fd(int efd, int new_fd)
{
  struct epoll_event e;
  e.events  = EPOLLIN | EPOLLONESHOT;
  e.data.fd = new_fd;
  return epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &e);
}

static int inline epoll_add_incl_fd(int efd, int new_fd)
{
  struct epoll_event e;
  e.events  = EPOLLIN;
  e.data.fd = new_fd;
  return epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &e);
}

static int inline epoll_add_excl_ptr(int efd, int new_fd, void* ptr)
{
  struct epoll_event e;
  e.events   = EPOLLIN | EPOLLONESHOT;
  e.data.ptr = ptr;
  return epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &e);
}

static int inline epoll_add_incl_ptr(int efd, int new_fd, void* ptr)
{
  struct epoll_event e;
  e.events   = EPOLLIN;
  e.data.ptr = ptr;
  return epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &e);
}

static int inline epoll_mod_incl_fd(int efd, int fd)
{
  struct epoll_event e;
  e.events   = EPOLLIN;
  e.data.fd  = fd;
  return epoll_ctl(efd, EPOLL_CTL_MOD, fd, &e);
}

static int inline epoll_mod_excl_fd(int efd, int fd)
{
  struct epoll_event e;
  e.events   = EPOLLIN | EPOLLONESHOT;
  e.data.fd  = fd;
  return epoll_ctl(efd, EPOLL_CTL_MOD, fd, &e);
}

static int inline epoll_mod_incl_ptr(int efd, int fd, void* ptr)
{
  struct epoll_event e;
  e.events   = EPOLLIN;
  e.data.ptr = ptr;
  return epoll_ctl(efd, EPOLL_CTL_MOD, fd, &e);
}

static int inline epoll_mod_excl_ptr(int efd, int fd, void* ptr)
{
  struct epoll_event e;
  e.events   = EPOLLIN | EPOLLONESHOT;
  e.data.ptr = ptr;
  return epoll_ctl(efd, EPOLL_CTL_MOD, fd, &e);
}

static int inline epoll_del_fd(int efd, int fd)
{
  struct epoll_event e; /* compatiable with 2.6.9 or before */
  return epoll_ctl(efd, EPOLL_CTL_DEL, fd, &e);
}



#ifdef __cplusplus
}
#endif

#endif
