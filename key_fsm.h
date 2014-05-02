/* 
 * Description: - 提供套接字输出有限状态机私有函数库的接口。
 *                （注意：之所以称之为套接字输出有限状态机，是因为键盘输入的数据经过本Telnet客户端处理
 *                  后，都是要输出的，输出到套接字或者客户终端上。）
 */

#ifndef _KEY_FSM_H_
#define _KEY_FSM_H_

#include <stdio.h>

/**
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
int status(FILE *sfp, FILE *tfp, int c);

/**
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
int change_lf(FILE *sfp, FILE *tfp, int c);

/**
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
int change_oprea(FILE *sfp, FILE *tfp, int c);

/**
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
int prompts(FILE *sfp, FILE *tfp, int c);

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
int soputc(FILE *sfp, FILE *tfp, int c);

/**
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
int suspend(FILE *sfp, FILE *tfp, int c);

#endif /* _KEY_FSM_H_ */
