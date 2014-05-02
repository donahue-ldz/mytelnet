/*
 *
 * Description: - telnet客户端的主函数，管理人机界面。
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "telnet.h"
#include "fsm_struct.h"
#include "fsm_build.h"
#include "connect.h"
#include "common_fsm.h"
#include "main_fsm.h"

#define BUFSIZE         2048            /* 缓冲区大小 */

char *host  = "localhost";              /* 服务端地址，本机地址 */

struct termios  oldtty;                 /* 该结构保存原来的终端设置 */
struct termios  tntty;                 /* 设置telnet终端的结构体 */

static int ttysetup(void);
static int telnet(const char *host, const char *service);

/*
 *
 * main - telnet客户端主程序
 *
 * @argc : 传入参数个数
 * @argv : 传入的参数，分别是服务器端地址、服务（或端口）
 *
 * telnet客户端主程序，输入服务器地址及端口进行连接，
 * 默认的服务器地址是本机地址，默认服务为telnet(23端口)
 *
 * @Return :
 *              成功 : 0
 *              失败 : 退出程序
 */
int main(int argc, char *argv[])
{

    char    *service = "telnet";    /* 服务器上telnet服务默认的名字*/

    printf("Build by CHEN Jun-pei(cjpthee@126.com) 2011.5.1\n");

    if (argc > 1) {
        /* 打印使用提示信息 */
        if ((argv[1][0] == 'h' && argv[1][1] == 'e' && argv[1][2] == 'l' && argv[1][3] == 'p')
            || (argv[1][0] == '-' && argv[1][1] == 'h')) {
            printf("\nusage: telnet [host [port]]\n");
            printf("default host is %s\n", host);
            printf("default port/service is %s\n", service);
            printf("\n");
            return 0;
        }
    }

    switch (argc) {
    case 1:    break;
    case 3:
        service = argv[2];
        /* FALL THROUGH */
    case 2:
        host = argv[1];
        break;
    default:
        (void)errexit("\nusage: telnet [host [port]]\n");
    }

    (void)telnet(host, service);

    return 0;

}

/*
 *
 * ttysetup - 设置终端
 *
 * 先取得原始的终端设置并保存原始设置，
 * 再进行具体设置，然后用设置好的终端参数改变终端属性。
 * 设置后的终端将禁能中断键^c、退出键^\、挂起键^z
 * 等特殊键功能。
 *
 * @Return :
 *              成功 : SUCCESS
 *              失败 : 退出程序
 */
static int ttysetup(void)
{

     /* 备份原始的终端设置  */
    if (tcgetattr(0, &oldtty) < 0) {
        errexit("can't get tty modes: %s\n", strerror(errno));
    }

    sg_erase = oldtty.c_cc[VERASE];
    sg_kill  = oldtty.c_cc[VKILL];
    t_intrc  = oldtty.c_cc[VINTR];
    t_quitc  = oldtty.c_cc[VQUIT];
    t_flushc = oldtty.c_cc[VDISCARD];

    tntty = oldtty;                             /* 复制原终端设置  */

    /*禁能某些特殊字符*/
    tntty.c_cc[VINTR] = _POSIX_VDISABLE;        /* ^c */
    tntty.c_cc[VQUIT] = _POSIX_VDISABLE;        /* ^\ */
    tntty.c_cc[VSUSP] = _POSIX_VDISABLE;        /* ^z */

    if (tcsetattr(0, TCSADRAIN, &tntty) < 0) {  /* 设置telnet终端环境 */
        errexit("can't set tty modes: %s\n", strerror(errno));
    }

    return SUCCESS;

}

/*
 *
 * telnet - telnet连接
 *
 * @host : 服务器地址
 * @service : 服务器端口
 *
 * 根据服务器地址和端口号设置telnet连接，用select()同时监视套接字
 * 输入和键盘输入，有套接字输入或键盘输入则调用对应的有限
 * 状态机进行处理。
 *
 * @Return :
 *              成功 : SUCCESS
 *              失败 : FAIL
 */
static int telnet(const char *host, const char *service)
{

    unsigned char    buf[BUFSIZE];
    int     s;          /* socket 文件描述符 */
    int     nfds;
    int     cc;
    int     on = 1;
    fd_set  arfds;
    fd_set  awfds;
    fd_set  rfds;
    fd_set  wfds;
    FILE    *sfp;

    if (!host || !service) {
        printf("host is NULL or service is NULL\n");
        return FAIL;
    }

    printf("Trying \"%s\" ...\n", host);
    s = connectTCP(host, service);                  /* 设置TCP连接 */
    if (s < 0) {
        printf("connetctTCP to \"%s\" error\n", host);
        return FAIL;
    }

    if (ttysetup() != SUCCESS) {                    /* 设置终端 */
        printf("set up tty error\n");
        return FAIL;
    }
    printf("Escape character is '^]'.\n");
    printf("\n");

    (void)fsmbuild();                               /* 设置有限状态机 */

    (void)signal(SIGURG, rcvurg);                   /* 设置紧急信号处理机制 */

    if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE,    /* 设置telnet参数，允许在常规数据流中接收带外数据 */
                      (char *)&on, sizeof(on))) {
        return FAIL;
    }

    nfds = getdtablesize();                         /* 取得本系统的文件描述符最大值 */

    /* 为select()设置要监视的文件描述符 */
    FD_ZERO(&arfds);
    FD_ZERO(&awfds);
    FD_SET(s, &arfds);
    FD_SET(0, &arfds);

    sfp = fdopen(s, "w");

    /* Telnet通信的状态中 */
    while (1) {
        memcpy(&rfds, &arfds, sizeof(rfds));
        memcpy(&wfds, &awfds, sizeof(rfds));

        /* 监视文件集，对socket端口和键盘输入进行读监视 */
        if (select(nfds, &rfds, &wfds, (fd_set *)0, (struct timeval *)0) < 0) {
            if (errno == EINTR) {
                continue;                           /* just a signal */
            }
            cerrexit("select: %s\n", strerror(errno));
        }

        /* 如果socket端口有数据到达，则读取数据并调用主fsm对数据进行处理 */
        if (FD_ISSET(s, &rfds)) {
            cc = read(s, (char *)buf, sizeof(buf));
            if (cc < 0) {
                cerrexit("socket read: %s\n", strerror(errno));
            } else if (cc == 0) {
                printf("\nconnection closed.\n");
                if (tcsetattr(0, TCSADRAIN, &oldtty) < 0) {
                    errexit("tcsetattr: %s\n", strerror(errno));
                }
                exit(0);
            } else {
                ttwrite(sfp, stdout, buf, cc);
            }
        }

        /* 如果键盘输入有数据到达，则读取数据并调用套接字输出fsm对数据进行处理 */
        if (FD_ISSET(0, &rfds)) {
            cc = read(0, (char *)buf, sizeof(buf));
            if (cc < 0) {
                cerrexit("tty read: %s\n", strerror(errno));
            } else if (cc == 0) {
                FD_CLR(0, &arfds);
                (void) shutdown(s, 1);
            } else {
                sowrite(sfp, stdout, buf, cc);
            }
        }

        /* 刷新缓冲区 */
        (void)fflush(sfp);
        (void)fflush(stdout);
    }

    return SUCCESS;

}
