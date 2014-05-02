#ifndef _CONNECT_H_
#define _CONNECT_H_

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
int connectTCP(const char *host, const char *service );

#endif /* _CONNECT_H_ */
