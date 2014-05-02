/*
 *
 * Description: - 套接字输出有限状态机私有函数库。
 *               （注意：之所以称之为套接字输出有限状态机，是因为键盘输入的数据经过本Telnet客户端处理
 *                 后，都是要输出的，输出到套接字或者客户终端上。）
 *
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "telnet.h"
#include "fsm_struct.h"
#include "common_fsm.h"
#include "main_fsm.h"
#include "key_fsm.h"

/*
 * status - 打印连接状态信息
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 打印连接状态信息
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int status(FILE *sfp, FILE *tfp, int c)
{

    struct sockaddr_in   sin;
    int                  sinlen;

    fprintf(tfp, "\nconnected to \"%s\" ", host);

    sinlen = sizeof(sin);
    if (getsockname(fileno(sfp), (struct sockaddr *)&sin, (unsigned int *)&sinlen) == 0) {
        fprintf(tfp, "local port %d ", ntohs(sin.sin_port));
    }
    sinlen = sizeof(sin);
    if (getpeername(fileno(sfp), (struct sockaddr *)&sin, (unsigned int *)&sinlen) == 0) {
        fprintf(tfp, "remote port %d \n", ntohs(sin.sin_port));
    }
    (void) putc('\n', tfp);

    fprintf(tfp, "options in effect: \n");
    if (doecho) {
        fprintf(tfp, "echo mode:           remote_echo \n");
    }
    else {
        fprintf(tfp, "echo mode:           local_echo \n");
    }

    if (sndbinary) {
        fprintf(tfp, "Transmission mode:   send_binary \n");
    }
    if (rcvbinary) {
        fprintf(tfp, "                     receive_binary \n");
    }
    if (!sndbinary && !rcvbinary) {
        fprintf(tfp, "Transmission mode:   NVT mode\n");
    }

    if (noga) {
        fprintf(tfp, "Operating mode:      once a character\n");
    }
    else {
        fprintf(tfp, "Operating mode:      line mode\n");
    }

    if (cr_lf) {
        fprintf(tfp, "line feed mode(bin): LF-CR\n");
    }
    else {
        fprintf(tfp, "line feed mode(bin): LF\n");
    }

    fprintf(tfp, "\n");

    return SUCCESS;

}

/*
 *
 * change_lf - 改变回车键的发送方式
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 改变回车键的发送方式，在LF与LF-CR之间转换
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int change_lf(FILE *sfp, FILE *tfp, int c)
{

    cr_lf = !cr_lf;

    //(void)fprintf(tfp, "\nhave changed the mode to %s\n", cr_lf ? "LF-CR" : "CR");

    return SUCCESS;

}

/*
 *
 * change_oprea - 改变操作方式
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 改变操作方式，在一次一个字符和行方式之间转换，同时改变回显方式
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int change_oprea(FILE *sfp, FILE *tfp, int c)
{

    /* 以下两项必须同时设置 */
    option_cmd = TCWONT;
    do_echo(sfp, tfp, TOECHO);
    option_cmd = TCWONT;
    do_noga(sfp, tfp, TONOGA);

    return SUCCESS;

}

/*
 *
 * prompts - 转义模式下的输入提示
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 转义模式下的输入提示
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int prompts(FILE *sfp, FILE *tfp, int c)
{

    fprintf(tfp, "\n");
    fprintf(tfp, ".  - disconnect\n");
    fprintf(tfp, "^Z - suspend\n");
    fprintf(tfp, "^T - print status\n");
    fprintf(tfp, "l -  change line feed mode(LF/LF-CR)\n");
    fprintf(tfp, "o -  change operating mode\n");

    return SUCCESS;

}

/**
 * soputc - 把键盘输入数据发送到套接字
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 把键盘输入数据发送到套接字，使用二进制发送则直接发送，使用NVT则对特殊字符
 * 进行处理后发送。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int soputc(FILE *sfp, FILE *tfp, int c)
{

    /* 二进制发送 */
    if (sndbinary) {
        if (c == TCIAC) {
            (void) putc(TCIAC, sfp);       /* 填充IAC字节 */
        }
        if (c == VPLF) {                   /* 换行键 */
            if (cr_lf) {
                (void)putc(VPCR, sfp);
                (void)putc(VPLF, sfp);
            } else {
                (void)putc(VPLF, sfp);
            }
        } else {
            (void) putc(c, sfp);
        }

        return SUCCESS;
    }

    /* NVT发送 */
    c &= 0x7f;                              /* 保证为7位的 ASCII 编码 */
    if (c == t_intrc || c == t_quitc) {     /* 中断字符 */
        (void)putc(TCIAC, sfp);
        (void)putc(TCIP, sfp);
    } else if (c == sg_erase) {             /* 转义字符 */
        (void)putc(TCIAC, sfp);
        (void)putc(TCEC, sfp);
    } else if (c == sg_kill) {              /* 删除行 */
        (void)putc(TCIAC, sfp);
        (void)putc(TCEL, sfp);
    } else if (c == t_flushc) {             /* 中止输出 */
        (void)putc(TCIAC, sfp);
        (void)putc(TCAO, sfp);
    } else if (c == VPLF) {                 /* 换行键 */
        (void)putc(VPCR, sfp);
        (void)putc(VPLF, sfp);
    }else {                                 /* 正常的数据输出 */
        (void)putc(c, sfp);
    }

    return SUCCESS;

}

/*
 *
 * suspend - 挂起会话
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 临时挂起会话，先保存当前终端状态信息，然后挂起，重新恢复时恢复挂起前终端状态
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int suspend(FILE *sfp, FILE *tfp, int c)
{

    if (tcgetattr(0, &tntty) < 0) {
        errexit("can't get tty modes: %s\n", strerror(errno));
    }
    if (tcsetattr(0, TCSADRAIN, &oldtty) < 0) {
        errexit("can't set tty modes: %s\n", strerror(errno));
    }

    (void) kill(0, SIGTSTP);

    if (tcgetattr(0, &oldtty) < 0) {
        errexit("can't get tty modes: %s\n", strerror(errno));
    }
    if (tcsetattr(0, TCSADRAIN, &tntty) < 0) {
        errexit("can't set tty modes: %s\n", strerror(errno));
    }

    return SUCCESS;

}
