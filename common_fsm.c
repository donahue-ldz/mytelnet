/*
 *
 * Description: - 有限状态机（fsm）共用函数库。
 *                包含三个fsm共用的函数，和本telnet客户端会用到的基础函数。
 */

#include <sys/types.h>
#include <termios.h>  //查
#include <term.h>
#include <curses.h>   //查
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>    //查

#include "telnet.h"
#include "fsm_struct.h"
#include "common_fsm.h"
#include "main_fsm.h"

#define    TBUFSIZE    2048

unsigned char   option_cmd;     /* 保存接收到的命令，可能的值为 WILL, WONT, DO, 或 DONT */
char            synching = 0;   /* 作为同步信号，同步的时候此值为非零 */

/*
 * recopt - 记录命令
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 记录下接收到的命令，保存到全局变量option_cmd中
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int recopt(FILE *sfp, FILE *tfp, int c)
{

    option_cmd = c;

    return SUCCESS;

}

/*
 * no_op - 空操作
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 不进行任何操作
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int no_op(FILE *sfp, FILE *tfp, int c)
{

    return SUCCESS;

}

/*
 *
 * dcon - 断开远程连接
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 断开远程连接。恢复原来的终端设置，并直接退出程序
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int dcon(FILE *sfp, FILE *tfp, int c)
{

    (void)fprintf(tfp, "disconnecting.\n\n");
    (void)tcsetattr(0, TCSADRAIN, &oldtty);  //tcsetattr()函数实现

    exit(0);

}

/*
 *
 * tnabort - 在fsm的无效状态时退出telnet
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 在fsm的无效状态时退出telnet
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int tnabort(FILE *sfp, FILE *tfp, int c)
{

    (void)errexit("invalid state reached (aborting)\n");

    return SUCCESS;

}

/*
 *
 * errexit - 退出程序
 *
 * @format : 错误信息
 *
 * 打印错误信息并退出程序
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int errexit(const char *format, ...)
{

    va_list    args;             //可变参数列表

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);

}

/*
 *
 * cerrexit - 退出程序并恢复终端设置
 *
 * @format : 错误信息
 *
 * 打印错误信息，并恢复终端设置，然后退出程序
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int cerrexit(const char *format, ...)
{

    va_list    args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    (void)tcsetattr(0, TCSADRAIN, &oldtty);//
    va_end(args);

    exit(1);

}

/*
 *
 * tcdm - 同步结束
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 处理同步结束信号"DATA MARK"，处理方法是把同步标志synching减一
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int tcdm(FILE *sfp, FILE *tfp, int c)
{

    if (synching > 0) {
        synching--;
    }

    return SUCCESS;
}

/*
 *
 * rcvurg - 开始同步
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 同步开始的处理，处理方法是把同步标志synching加一
 *
 * @Return :
 *          void
 */
void rcvurg(int sig)
{

    synching++;

}

/*
 *
 * ttwrite - 解释执行从socket接收到的字符
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @buf : 保存接收到的数据的缓冲区
 * @cc  : 缓冲区中有效数据的字节数
 *
 * 解释执行从socket接收到的字符。
 * 此函数运行主有限状态机，根据接收到的字符和当前状态查找转移矩阵，
 * 并执行相应的操作，然后更新当前状态。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int ttwrite(FILE *sfp, FILE *tfp, unsigned char *buf, int cc)
{

    struct fsm_trans    *pt;
    int                 i;
    int                 ti;
    int                 c;

    for (i = 0; i < cc; ++i) {
        c = buf[i];

        ti = ttfsm[ttstate][c];
        pt = &ttstab[ti];

        (pt->ft_action)(sfp, tfp, c);
        ttstate = pt->ft_next;
    }

    return SUCCESS;

}

/*
 *
 * sowrite - 解释执行从键盘输入的字符
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @buf : 保存接收到的数据的缓冲区
 * @cc  : 缓冲区中有效数据的字节数
 *
 * 解释执行从键盘输入的字符。
 * 此函数运行套接字输出有限状态机，
 * 根据接收到的字符和当前状态查找转移矩阵，
 * 并执行相应的操作，然后更新当前状态。
 *
 * @Return :
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int sowrite(FILE *sfp, FILE *tfp, unsigned char *buf, int cc)
{

    struct fsm_trans    *pt;
    int                 i;
    int                 ki;
    int                 c;

    for (i = 0; i < cc; ++i) {
        c = buf[i];

        ki = sofsm[sostate][c];
        pt = &sostab[ki];

        if ((pt->ft_action)(sfp, tfp, c) != SUCCESS) {
            sostate = KSREMOTE;                 /* 发生错误 */
        }
        else {
            sostate = pt->ft_next;
        }
    }

    return SUCCESS;

}

/*
 *
 * tcout - 使用termcap控制用户终端
 *
 * @cap : 标准的termcap终端兼容名
 * @tfp : 显示终端
 *
 * 遇到特殊字符，将调用此函数完成该字符相关联的动作。
 * 它使用函数getenv( )从环境变量TERM中提取终端类型，
 * 然后调用过程tgetstr( )查找为在用户终端上获得指明的效果所需
 * 的字符序列。最后，它调用fputs( )将所生成的字符序列写到终端上。
 *
 * @Return :
 *          是终端控制命令   : 1
 *          不是终端控制命令 : 0
 */
int tcout(char *cap, FILE *tfp)
{

    static int  init;
    static char *term = "name";
    static char buf[TBUFSIZE];
    static char tbuf[TBUFSIZE];
    static char *bp = buf;
    char        *sv;

    if (!init) {
        init = 1;
        term = getenv("TERM");
    }

    if (term == 0 || tgetent(&tbuf[0], term) != 1) {
        return 0;
    }

    if ((sv = tgetstr(cap, &bp))) {
        fputs(sv, tfp);
        return 1;
    }

    return 0;

}

