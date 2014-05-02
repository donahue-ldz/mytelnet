/*
 * Description: - 初始化描述三个有限状态机（fsm）的压缩转移矩阵。
 *
 *
 *
 */

#include <sys/types.h>
#include <stdio.h>

#include "telnet.h"
#include "fsm_struct.h"
#include "main_fsm.h"
#include "key_fsm.h"
#include "sub_fsm.h"
#include "common_fsm.h"

/* 特殊字符: */
char    t_flushc;   /* 中止输出 */
char    t_intrc;    /* 中断键 */
char    t_quitc;    /* 退出键 */
char    sg_erase;   /* 转义键 */
char    sg_kill;    /* 删除行 */

/***********************以下为主fsm的压缩转移矩阵**********************/

/* 主fsm转移矩阵的压缩表示 */
struct fsm_trans ttstab[] = {
    /* State    Input       Next State    Action */
    { TSDATA,   TCIAC,      TSIAC,       no_op },
    { TSDATA,   TCANY,      TSDATA,      ttputc },
    { TSIAC,    TCIAC,      TSDATA,      ttputc },
    { TSIAC,    TCSB,       TSSUBNEG,    no_op },
    /* Telnet Commands */
    { TSIAC,    TCNOP,      TSDATA,      no_op },
    { TSIAC,    TCDM,       TSDATA,      tcdm },
    /* Option Negotiation */
    { TSIAC,    TCWILL,     TSWOPT,      recopt },
    { TSIAC,    TCWONT,     TSWOPT,      recopt },
    { TSIAC,    TCDO,       TSDOPT,      recopt },
    { TSIAC,    TCDONT,     TSDOPT,      recopt },
    { TSIAC,    TCANY,      TSDATA,      no_op },
    /* Option Subnegotion */
    { TSSUBNEG, TCIAC,      TSSUBIAC,    no_op },
    { TSSUBNEG, TCANY,      TSSUBNEG,    subopt },
    { TSSUBIAC, TCSE,       TSDATA,      subend },
    { TSSUBIAC, TCANY,      TSSUBNEG,    subopt },

    { TSWOPT,   TOECHO,     TSDATA,      do_echo },
    { TSWOPT,   TONOGA,     TSDATA,      do_noga },
    { TSWOPT,   TOTXBINARY, TSDATA,      do_txbinary },
    { TSWOPT,   TCANY,      TSDATA,      do_notsup },

    { TSDOPT,   TOTERMTYPE, TSDATA,      will_termtype },
    { TSDOPT,   TOTXBINARY, TSDATA,      will_txbinary },
    { TSDOPT,   TCANY,      TSDATA,      will_notsup },

    { FSINVALID, TCANY,     FSINVALID,   tnabort},
};

int       ttstate;                                      /* 记录主fsm的当前状态 */
u_char    ttfsm[NTSTATES][NCHRS];                       /* 定义主fsm的转移矩阵 */

/***********************以下为字选项协商fsm的压缩转移矩阵**********************/

/* 子选项协商fsm转移矩阵的压缩表示 */
struct fsm_trans substab[] = {
    /* State       Input       Next State   Action */
    { SS_START,    TOTERMTYPE, SS_TERMTYPE, no_op },
    { SS_START,    TCANY,      SS_END,      no_op },

    { SS_TERMTYPE, TT_SEND,    SS_END,      subtermtype },
    { SS_TERMTYPE, TCANY,      SS_END,      no_op },

    { SS_END,      TCANY,      SS_END,      no_op },
    { FSINVALID,   TCANY,      FSINVALID,   tnabort },
};

int       substate;                         /* 记录子选项协商fsm的当前状态 */
u_char    subfsm[NSSTATES][NCHRS];          /* 定义子选项协商fsm的转移矩阵 */

/**********************以下为套接字输出fsm的压缩转移矩阵**********************/

/* 套接字输出fsm转移矩阵的压缩表示 */
struct fsm_trans sostab[] = {
    /* State       Input       Next State   Action  */
    /* Data Input */
    { KSREMOTE,    KCESCAPE,   KSLOCAL,     no_op },
    { KSREMOTE,    KCANY,      KSREMOTE,    soputc },
    /* Local Escape Commands */
    { KSLOCAL,     KCESCAPE,   KSREMOTE,    soputc },
    { KSLOCAL,     KCDCON,     KSREMOTE,    dcon },
    { KSLOCAL,     KCSUSP,     KSREMOTE,    suspend },
    { KSLOCAL,     KCCHANGE_CR,KSREMOTE,    change_lf },
    { KSLOCAL,     KCSTATUS,   KSREMOTE,    status },
    { KSLOCAL,     'o',        KSREMOTE,    change_oprea },
    { KSLOCAL,     KCANY,      KSREMOTE,    prompts },

    { FSINVALID,   KCANY,      FSINVALID,   tnabort },
};

int       sostate;                          /* 记录套接字输出fsm的当前状态 */
u_char    sofsm[NKSTATES][NCHRS];           /* 定义套接字输出fsm的转移矩阵 */
