/*
 * Description: - 主有限状态机私有函数库。
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>

#include "telnet.h"
#include "fsm_struct.h"
#include "common_fsm.h"
#include "main_fsm.h"

char        doecho    = 0; /* 记录服务器的回显状态，如果已回显，此值非零。默认本地回显，故初值为0 */
char        rcvbinary = 0; /* 记录远程的二进制发送状态，如果是二进制发送，此值非零。默认以NVT格式传送数据，故赋初值0 */
char        sndbinary = 0; /* 记录远程的二进制传输状态，如果是二进制传输，此值非零。默认以NVT格式传送数据，故赋初值0 */
char        termtype  = 0; /* 记录终端协商，如果已收到 "DO TERMTYPE"，此值非零 */
char        *myterm;       /* 本地终端类型 */
int         noga      = 0; /* 记录Telnet通信的操作方式，此值为1则为一次一个字符的操作方式 */
int         cr_lf     = 0; /* 记录二进制传输时回车键的解释，此值不为零时，回车是CR-LF，为零时是LF */

/**
 * do_echo - 处理WILL/WON'T ECHO选项
 *
 * @rfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 处理WILL/WON'T ECHO选项，规则如下：
 * 如果远程主机已标记为回显状态，并且又收到WILL通告，
 * 则认为本机与远程主机的需求一致，则直接返回成功；
 * 如果远程主机未标记为回显状态，而又收到WONT通告，
 * 则认为本机与远程主机的需求一致，也直接返回成功。
 * 远程主机希望的动作与本机状态不符时，本机改变终端设置。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int do_echo(FILE *rfp, FILE *tfp, int c)
{

    struct termios  tio;
    static char     savec[2];
    int             ok = 0;
    int             tfd = fileno(tfp);

    if (doecho) {
        if (option_cmd == TCWILL) {
            return SUCCESS;
        }
    } else if (option_cmd == TCWONT) {
        return SUCCESS;
    }

    /* 获取本地终端状态成功才进行以下操作 */
    if (tcgetattr(tfd, &tio) == 0) {
        if (option_cmd == TCWILL) {
            tio.c_lflag &= ~(ECHO | ICANON);

            /* 根据VMIN于VTIME的设置，每出现一个字符都进行回显 */
            savec[0] = tio.c_cc[VMIN];
            savec[1] = tio.c_cc[VTIME];
            tio.c_cc[VMIN] = 1;
            tio.c_cc[VTIME] = 0;
        } else {
        tio.c_lflag |= (ECHO | ICANON);
        tio.c_cc[VMIN] = savec[0];
        tio.c_cc[VTIME] = savec[1];
        }
        ok = (tcsetattr(tfd, TCSADRAIN, &tio) == 0);
    }

    /* 设置本地终端成功，则改变全局变量doecho的值，记录回显状态 */
    if (ok) {
        doecho = !doecho;
    }

    /* 发送DO/DONT命令作为应答 */
    (void)putc(TCIAC, rfp);
    if (doecho) {
        (void)putc(TCDO, rfp);
    } else {
        (void)putc(TCDONT, rfp);
    }
    (void)putc((char)c, rfp);

    return SUCCESS;

}

/**
 * do_noga - 处理WILL/WON'T No Go Ahead选项
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 处理WILL/WON'T No Go Ahead选项，规则如下：
 * 如果远程主机已标记为noga状态，并且又收到WILL通告，
 * 则认为本机与远程主机的需求一致，则直接返回成功；
 * 如果远程主机未标记为noga状态，而又收到WONT通告，
 * 则认为本机与远程主机的需求一致，也直接返回成功。
 * 远程主机希望的动作与本机状态不符时，本机改变终端设置。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int do_noga(FILE *sfp, FILE *tfp, int c)
{

    if (noga) {
        if (option_cmd == TCWILL) {
            return SUCCESS;
        }
    } else if (option_cmd == TCWONT) {
        return SUCCESS;
    }

    /* 改变静态变量noga的值，记录本机Go Ahead状态 */
    noga = !noga;

    /* 发送DO/DONT命令作为应答 */
    (void)putc(TCIAC, sfp);
    if (noga) {
        (void)putc(TCDO, sfp);
    } else {
        (void)putc(TCDONT, sfp);
    }
    (void)putc((char)c, sfp);

    return SUCCESS;

}

/**
 * do_notsup - 处理不支持的WILL/WON'T选项
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 处理不支持的WILL/WON'T选项，规则如下：
 * 直接回复[IAC + DONT + COMMON]。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int do_notsup(FILE *sfp, FILE *tfp, int c)
{
    (void)putc(TCIAC, sfp);
    (void)putc(TCDONT, sfp);
    (void)putc((char)c, sfp);

    return SUCCESS;

}

/**
 * will_notsup - 处理不支持的DO/DON'T选项
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 处理不支持的DO/DON'T选项，规则如下：
 * 直接回复[IAC + WONT + COMMON]。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int will_notsup(FILE *sfp, FILE *tfp, int c)
{

    (void)putc(TCIAC, sfp);
    (void)putc(TCWONT, sfp);
    (void)putc((char)c, sfp);

    return SUCCESS;

}

/**
 * ttputc - 打印从套接字接收到的单个字符
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 打印从套接字接收到的单个字符，规则如下：
 * 如果字符未经NVT编码，则直接打印；
 * 如果当前在同步状态(synching非零)，则不显示字符；
 * 如果回车(CR)和换行(LF)同时出现则作为Unix系统的换行处理；
 * 如果出现单个回车或换行或其他特殊字符，则调用tcout完成该字符相关联的动作；
 * 符合打印的条件(c >= ' ' && c < TCIAC)，则直接打印。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int ttputc(FILE *sfp, FILE *tfp, int c)
{
    static   int    last_char;      /* 保存前一个字符的副本 */
    int             tc;             /* 值为VALUE时是控制终端的命令 */

    if (rcvbinary) {
        /* 遇到CR于LF同时出现，都作为换行解释 */
        if (cr_lf && (c == VPCR)) {
            return SUCCESS;
        }

        (void)putc(c, tfp);
        return SUCCESS;
    }/* rcvbinary end */

    if (synching) {                 /* 同步 */
        return FAIL;
    }

    /* NVT编码 */
    if ((last_char == VPCR && c == VPLF) || (last_char == VPLF && c == VPCR)) {
        (void)putc(VPLF, tfp);
        last_char = 0;
        return SUCCESS;
    }
    if (last_char == VPCR) {
        (void)tcout("cr", tfp);
    } else if (last_char == VPLF) {
        (void)tcout("do", tfp);
    }
    if (c >= ' ' && c < TCIAC) {
        (void)putc(c, tfp);
    } else {                        /* 特殊的NVT编码 */
        switch (c) {
        case VPLF:                  /* 出现了换行，查看是否回车跟随其后 */
        case VPCR:                  /* 出现了回车，查看是否换行跟随其后 */
            tc = VALUE;
            break;
        case VPBEL:
            tc = tcout("bl", tfp);
            break;
        case VPBS:
            tc = tcout("bc", tfp);
            break;
        case VPHT:
            tc = tcout("ta", tfp);
            break;
        case VPVT:
            tc = tcout("do", tfp);
            break;
        case VPFF:
            tc = tcout("cl", tfp);
            break;
        default:
            tc = VALUE;
            break;
        } /* switch end */

        if (!tc) {                  /* 如果不是控制终端的命令，也就是可打印的编码 */
            (void)putc(c, tfp);
        }
    }
    last_char = c;

    return SUCCESS;

}

/**
 * do_txbinary - 响应二进制传输的WILL/WON'T通告
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 响应二进制传输的WILL/WON'T通告，规则如下：
 * 如果已标记rcvbinary状态，并且又收到WILL通告，
 * 则认为本机与远程主机的需求一致，则直接返回成功；
 * 如果未标记rcvbinary状态，而又收到WONT通告，
 * 则认为本机与远程主机的需求一致，也直接返回成功。
 * 远程主机希望的动作与本机状态不符时，
 * 本机改变传输方式设置，并发送响应信息。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int do_txbinary(FILE *sfp, FILE *tfp, int c)
{

    if (rcvbinary) {
        if (option_cmd == TCWILL) {
            return SUCCESS;
        }
    } else if (option_cmd == TCWONT) {
        return SUCCESS;
    }

    rcvbinary = !rcvbinary;

    (void) putc(TCIAC, sfp);
    if (rcvbinary) {
        (void)putc(TCDO, sfp);
    } else {
        (void)putc(TCDONT, sfp);
    }
    (void) putc((char)c, sfp);

    return SUCCESS;

}

/**
 * do_txbinary - 响应二进制传输的DO/DON'T请求
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 响应二进制传输的DO/DON'T请求，规则如下：
 * 如果已标记sndbinary状态，并且又收到WILL通告，
 * 则认为本机与远程主机的需求一致，则直接返回成功；
 * 如果未标记sndbinary状态，而又收到WONT通告，
 * 则认为本机与远程主机的需求一致，也直接返回成功。
 * 远程主机希望的动作与本机状态不符时，
 * 本机改变传输方式设置，并发送响应信息。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int will_txbinary(FILE *sfp, FILE *tfp, int c)
{

    if (sndbinary) {
        if (option_cmd == TCDO) {
            return SUCCESS;
        }
    } else if (option_cmd == TCDONT) {
        return SUCCESS;
    }

    sndbinary = !sndbinary;

    (void)putc(TCIAC, sfp);
    if (sndbinary) {
        (void)putc(TCWILL, sfp);
    } else {
        (void)putc(TCWONT, sfp);
    }
    (void)putc((char)c, sfp);

    return SUCCESS;

}

/**
 * will_termtype - 响应终端类型的DO/DON'T请求
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 响应终端类型的DO/DON'T请求，规则如下：
 * 如果已标记termtype状态，并且又收到DO请求，
 * 则认为本机与服务器的需求一致，则直接返回成功；
 * 如果未标记termtype状态，而又收到DONT请求，
 * 则认为本机与远程主机的需求一致，也直接返回成功。
 * 远程主机希望的动作与本机状态不一致时，
 * 本机改变termtype标记状态，获取本机终端类型，并发送响应信息，
 * 然后请求使用二进制传输接下来的数据（子选项协商）。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int will_termtype(FILE *sfp, FILE *tfp, int c)
{

    if (termtype) {
        if (option_cmd == TCDO) {
            return SUCCESS;
        }
    } else if (option_cmd == TCDONT) {
        return SUCCESS;
    }

    termtype = !termtype;

    if (termtype) {
        if (!myterm && !(myterm = getenv("TERM"))) {
            termtype = !termtype;
        }
    }

    (void)putc(TCIAC, sfp);
    if (termtype) {
        (void) putc(TCWILL, sfp);
    } else {
        (void) putc(TCWONT, sfp);
    }
    (void) putc((char)c, sfp);

    /* 终端类型的子选项协商序列不能使用NvT编码发送，
     * 所以宣告客户要使用二进制传输的意愿，
     * 并发送DO报文请求服务器使用二进制传输。
     */
    if (termtype) {
       if (!sndbinary) {
           (void)putc(TCIAC, sfp);
            (void)putc(TCWILL, sfp);
            (void)putc((char)TOTXBINARY, sfp);

            (void)putc(TCIAC, sfp);
            (void)putc(TCDO, sfp);
            (void)putc((char)TOTXBINARY, sfp);
        }

    }

    return SUCCESS;

}

/**
 * subend - 结束子选项协商的处理
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 结束子选项协商的处理，规则如下：
 * 把子选项有限状态机的状态设为初始状态。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int subend(FILE *sfp, FILE *tfp, int c)
{

    substate = SS_START;

    return SUCCESS;

}

/**
 * subopt - 子选项协商的处理
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 子选项协商处理，规则如下：
 * 主fsm接收到子选项命令序列后，便进入TSSUBNEG状态，在该状态下，
 * 客户端每接收到一个字符，就调用一次本函数，本函数运行着子选项协商
 * fsm来处理子选项，直到接收到子选项结束命令才通过调用subend()退出
 * TSSUBNEG状态。
 * 函数通过转移矩阵，找到对应的操作并调用，然后把子选项的状态设为下一个状态。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int subopt(FILE *sfp, FILE *tfp, int c)
{

    struct    fsm_trans *pt;
    int                 ti;

    ti = subfsm[substate][c];
    pt = &substab[ti];
    (pt->ft_action)(sfp, tfp, c);
    substate = pt->ft_next;

    return SUCCESS;

}
