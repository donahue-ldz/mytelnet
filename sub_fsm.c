/*
 *
 * Description: - 子选项协商有限状态机私有函数库。
 *
 *
 */

#include <sys/types.h>
#include <stdio.h>

#include "telnet.h"
#include "main_fsm.h"

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
int subtermtype(FILE *sfp, FILE *tfp, int c)
{

    (void)putc(TCIAC, sfp);
    (void)putc(TCSB, sfp);
    (void)putc(TOTERMTYPE, sfp);
    (void)putc(TT_IS, sfp);
    (void)fputs(myterm, sfp);
    (void)putc(TCIAC, sfp);
    (void)putc(TCSE, sfp);

    return SUCCESS;

}

