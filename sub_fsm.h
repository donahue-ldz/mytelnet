/*
 *
 * Description: - 提供子选项协商有限状态机私有函数库的接口。
 *    
 *         
 */

#ifndef _SUB_FSM_H_
#define _SUB_FSM_H_

#include <stdio.h>

/**
 * subtermtype - 发送终端类型信息
 *
 * @sfp : 用于发送信息的文件描述符
 * @tfp : 显示终端
 * @c   : 命令字符
 *
 * 发送终端类型信息，规则如下：
 * 在子选项有限状态机中，接收到IAC.SB.TERMTYPE.SEND信息后，调用此过程。
 * 直接发送IAC.SB.IS."TERMTYPE".IAC.SEND作为应答
 *
 * @Return : 
 *          成功 : SUCCESS
 *          失败 : FAIL
 */
int subtermtype(FILE *sfp, FILE *tfp, int c);

#endif /* _SUB_FSM_H_ */

