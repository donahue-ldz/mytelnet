/*
 *
 * Description: - telnet连接的具体实现，通过调用TCP套接字实现连接。
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "telnet.h"
#include "connect.h"
#include "common_fsm.h"

#ifndef    INADDR_NONE
#define    INADDR_NONE    0xffffffff    /* 广播地址 */
#endif    /* INADDR_NONE */

/*
 * connectsock - 调用套接字连接到特定主机的特定服务
 *
 * @host : 指定的连接主机
 * @service : 指定的服务
 * @transport : 协议类型，可以是"TCP"或"UDP"
 *
 * 调用套接字连接到特定主机的特定服务
 *
 * @Return :
 *              成功 : TCP端口号
 *              失败 : 无
 */
static int connectsock(const char *host, const char *service, const char *transport )
{

    struct hostent      *phe;       /* 服务器IP地址 */
    struct servent      *pse;       /* 端口号 */
    struct protoent     *ppe;       /* 协议类型 */
    struct sockaddr_in  sin;        /* 服务器网络地址 */
    int                 s;          /* socket 文件描述符 */
    int                 type;       /* socket 类型 */
    char                *buf;       /* 服务器IP地址的十进制形式 */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* 把服务名转换成端口号 */
    if ((pse = getservbyname(service, transport))) {
        sin.sin_port = pse->s_port;
    } else if (!(sin.sin_port=htons((unsigned short)atoi(service)))) {
        errexit("can't get \"%s\" service entry\n", service);
    }

    /* 把主机名转换成IP地址，允许主机名为点分十进制形式 */
    if ((phe = gethostbyname(host))) {
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    } else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE ) {
        errexit("can't get \"%s\" host entry\n", host);
    }

    /* 把协议名字转换成代表该协议的数字 */
    if ((ppe = getprotobyname(transport)) == 0) {
        errexit("can't get \"%s\" protocol entry\n", transport);
    }

    /* 由协议类型决定套接字类型 */
    if (strcmp(transport, "udp") == 0) {
        type = SOCK_DGRAM;
    } else {
        type = SOCK_STREAM;
    }

    /* 分配一个套接字 */
    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0) {
        errexit("can't create socket: %s\n", strerror(errno));
    }

    /* 连接此套接字 */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        errexit("can't connect to %s.%s: %s\n", host, service, strerror(errno));
    }
    
    /* 提示连接成功 */
    buf = inet_ntoa(sin.sin_addr);
    printf("Connected to \"%s\" (%s).\n", host, buf); 
    
    return s;
    
}

/**
 * connectTCP - 用TCP连接到一个特定主机的特定服务
 *
 * @host : 指定的连接主机
 * @service : 指定的服务
 *
 * 使用TCP连接到远程主机的特定服务
 *
 * @Return :
 *              成功 : TCP端口号
 *              失败 : 无
 */
int connectTCP(const char *host, const char *service )
{

    return connectsock(host, service, "tcp");

 }
 
