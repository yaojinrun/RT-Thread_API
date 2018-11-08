/**
 * @file
 * NETDB API (sockets)
 */

/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_NETDB_H
#define LWIP_HDR_NETDB_H

#include "lwip/opt.h"

#if LWIP_DNS && LWIP_SOCKET

#include "lwip/arch.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

/* some rarely used options */
#ifndef LWIP_DNS_API_DECLARE_H_ERRNO
#define LWIP_DNS_API_DECLARE_H_ERRNO  1
#endif

#ifndef LWIP_DNS_API_DEFINE_ERRORS
#define LWIP_DNS_API_DEFINE_ERRORS    1
#endif

#ifndef LWIP_DNS_API_DEFINE_FLAGS
#define LWIP_DNS_API_DEFINE_FLAGS     1
#endif

#ifndef LWIP_DNS_API_DECLARE_STRUCTS
#define LWIP_DNS_API_DECLARE_STRUCTS  1
#endif

#if LWIP_DNS_API_DEFINE_ERRORS
/** Errors used by the DNS API functions, h_errno can be one of them */
#define EAI_NONAME      200
#define EAI_SERVICE     201
#define EAI_FAIL        202
#define EAI_MEMORY      203
#define EAI_FAMILY      204

#define HOST_NOT_FOUND  210
#define NO_DATA         211
#define NO_RECOVERY     212
#define TRY_AGAIN       213
#endif /* LWIP_DNS_API_DEFINE_ERRORS */

#if LWIP_DNS_API_DEFINE_FLAGS
/* input flags for struct addrinfo */
#define AI_PASSIVE      0x01
#define AI_CANONNAME    0x02
#define AI_NUMERICHOST  0x04
#define AI_NUMERICSERV  0x08
#define AI_V4MAPPED     0x10
#define AI_ALL          0x20
#define AI_ADDRCONFIG   0x40
#endif /* LWIP_DNS_API_DEFINE_FLAGS */

#if LWIP_DNS_API_DECLARE_STRUCTS
/**
 * @ingroup NET
 *
 * @brief 主机信息
 */
struct hostent {
    char  *h_name;      /**< @brief 主机名，即官方域名 */
    char **h_aliases;   /**< @brief 主机所有别名构成的字符串数组，同一IP可绑定多个域名 */
    int    h_addrtype;  /**< @brief 主机IP地址的类型，例如IPV4（AF_INET）还是IPV6 */
    int    h_length;    /**< @brief 主机IP地址长度，IPV4地址为4，IPV6地址则为16 */
    char **h_addr_list; /**< @brief 主机的ip地址，以网络字节序存储。若要打印出这个IP，需要调用inet_ntoa()。 */
#define h_addr h_addr_list[0] /**< @brief 为了向后兼容 */
};

/**
 * @ingroup NET
 *
 * @brief 地址信息
 */
struct addrinfo {
    int               ai_flags;      /**< @brief 输入标志 */
    int               ai_family;     /**< @brief 地址族套接字 */
    int               ai_socktype;   /**< @brief 套接字类型 */
    int               ai_protocol;   /**< @brief 套接字协议 */
    socklen_t         ai_addrlen;    /**< @brief 套接字地址的长度 */
    struct sockaddr  *ai_addr;       /**< @brief 套接字的套接字地址 */
    char             *ai_canonname;  /**< @brief 服务地点的规范名称 */
    struct addrinfo  *ai_next;       /**< @brief 指向下一个列表的指针 */
};
#endif /* LWIP_DNS_API_DECLARE_STRUCTS */

#define NETDB_ELEM_SIZE           (sizeof(struct addrinfo) + sizeof(struct sockaddr_storage) + DNS_MAX_NAME_LENGTH + 1)

#if LWIP_DNS_API_DECLARE_H_ERRNO
/* application accessible error code set by the DNS API functions */
extern int h_errno;
#endif /* LWIP_DNS_API_DECLARE_H_ERRNO*/

struct hostent *lwip_gethostbyname(const char *name);
int lwip_gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                size_t buflen, struct hostent **result, int *h_errnop);
void lwip_freeaddrinfo(struct addrinfo *ai);
int lwip_getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res);

#if LWIP_COMPAT_SOCKETS
/** @ingroup netdbapi */
#define gethostbyname(name) lwip_gethostbyname(name)
/** @ingroup netdbapi */
#define gethostbyname_r(name, ret, buf, buflen, result, h_errnop) \
       lwip_gethostbyname_r(name, ret, buf, buflen, result, h_errnop)
/** @ingroup netdbapi */
#define freeaddrinfo(addrinfo) lwip_freeaddrinfo(addrinfo)
/** @ingroup netdbapi */
#define getaddrinfo(nodname, servname, hints, res) \
       lwip_getaddrinfo(nodname, servname, hints, res)
#endif /* LWIP_COMPAT_SOCKETS */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_DNS && LWIP_SOCKET */

#endif /* LWIP_HDR_NETDB_H */
