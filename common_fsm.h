/*
 *
 * Description: - 提供有限状态机（fsm）共用函数库的接口。
 *                包含三个fsm共用的函数，和本telnet客户端会用到的基本函数。
 * 
 */
 
#ifndef _COMMON_FSM_H_
#define _COMMON_FSM_H_

#include <stdio.h>

extern unsigned char    option_cmd;    /* 保存接收到的命令，可能的值为 WILL, WONT, DO, 或 DONT */
extern char             synching;      /* 作为同步信号，同步的时候此值为非零 */

/**
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
int recopt(FILE *sfp, FILE *tfp, int c);

/**
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
int no_op(FILE *sfp, FILE *tfp, int c);

/**
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
int dcon(FILE *sfp, FILE *tfp, int c);

/**
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
int tnabort(FILE *sfp, FILE *tfp, int c);

/**
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
int errexit(const char *format, ...);

/**
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
int cerrexit(const char *format, ...);

/**
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
int tcdm(FILE *sfp, FILE *tfp, int c);

/**
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
void rcvurg(int sig);

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
int ttwrite(FILE *sfp, FILE *tfp, unsigned char *buf, int cc);

/**
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
int sowrite(FILE *sfp, FILE *tfp, unsigned char *buf, int cc);

/**
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
int tcout(char *cap, FILE *tfp);

#endif /* _COMMON_FSM_H_ */
