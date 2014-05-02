/*
 * Description: - 提供主有限状态机私有函数库的接口。
 */
   
#ifndef _MAIN_FSM_H_
#define _MAIN_FSM_H_

#include <stdio.h>

extern char        doecho;        /* 记录远程的回显状态，如果已回显，此值非零 */
extern char        rcvbinary;     /* 记录远程的二进制发送状态，如果是二进制发送，此值非零 */
extern char        sndbinary;     /* 记录远程的二进制传输状态，如果是二进制传输，此值非零 */
extern char        termtype;      /* 记录终端协商，如果已收到 "DO TERMTYPE"，此值非零 */
extern char        *myterm;       /* 本地终端类型 */
extern int         noga;          /* 记录Telnet通信的操作方式，此值为1则为一次一个字符的操作方式 */
extern int         cr_lf;         /* 记录二进制传输时回车键的解释，此值不为零时，回车是CR-LF，为零时是LF */

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
int do_echo(FILE *rfp, FILE *tfp, int c);

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
int do_noga(FILE *sfp, FILE *tfp, int c);

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
int do_notsup(FILE *sfp, FILE *tfp, int c);

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
int will_notsup(FILE *sfp, FILE *tfp, int c);

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
 * 如果回车(CR)和换行(LF)同时出现则作为unix系统的换行处理；
 * 如果出现单个回车或换行或其他特殊字符，则调用tcout完成该字符相关联的动作；
 * 符合打印的条件(c >= ' ' && c < TCIAC)，则直接打印。
 *
 * @Return : 
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int ttputc(FILE *sfp, FILE *tfp, int c);

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
int do_txbinary(FILE *sfp, FILE *tfp, int c);

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
int will_txbinary(FILE *sfp, FILE *tfp, int c);

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
int will_termtype(FILE *sfp, FILE *tfp, int c);

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
int subend(FILE *sfp, FILE *tfp, int c);

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
int subopt(FILE *sfp, FILE *tfp, int c);

#endif /* _MAIN_FSM_H_ */
