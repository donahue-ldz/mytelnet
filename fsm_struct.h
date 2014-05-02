/*
 * Description: - 定义描述三个有限状态机（fsm）的压缩转移矩阵的符号。
 */

#ifndef _FSM_STRUCT_H_
#define _FSM_STRUCT_H_

/* 主fsm: */
#define    TSDATA       0           /* 正常数据处理状态 */
#define    TSIAC        1           /* 接收到 IAC 后进入到状态，接下来处理命令 */
#define    TSWOPT       2           /* 接收到 IAC-{WILL/WONT} 进入的状态 */
#define    TSDOPT       3           /* 接收到 IAC-{DO/DONT} 进入的状态 */
#define    TSSUBNEG     4           /* 接收到 IAC-SB 进入的状态，该状态中运行子选项协商fsm */
#define    TSSUBIAC     5           /* 接收到 IAC-SB-...-IAC，如果再接收到SE则退出子协商 */

#define    NTSTATES     6           /* 主fsm的状态数目 */

/* 套接字输出fsm: */
#define    KSREMOTE     0           /* 键入数据发送到套接字 */
#define    KSLOCAL      1           /* 键入数据用于处理本地函数 */

#define    NKSTATES     3           /* 套接字输出fsm的状态数目 */

/* 子选项协商fsm: */
#define    SS_START     0           /* 初始状态 */
#define    SS_TERMTYPE  1           /* 处理终端类型的状态 */
#define    SS_END       2           /* 子选项协商结束状态 */

#define    NSSTATES     3           /* 子选项协商fsm的状态数目 */


#define    FSINVALID    0xff        /* 无效状态 */

#define    NCHRS        256         /* 有效的字符个数 */
#define    TCANY        (NCHRS+1)   /* 未定义的字符 */

/* 有限状态机转移矩阵复用结构 */
struct fsm_trans {
    unsigned char   ft_state;                                   /* 当前状态 */
    short           ft_char;                                    /* 输入字符 */
    unsigned char   ft_next;                                    /* 下一个状态 */
    int             (*ft_action)(FILE *sfp, FILE *tfp, int c);  /* 对应的操作 */
};

/* 特殊字符: */
extern char    t_flushc;                        /* 中止输出 */
extern char    t_intrc;                         /* 中断键 */
extern char    t_quitc;                         /* 退出键 */
extern char    sg_erase;                        /* 转义键 */
extern char    sg_kill;                         /* 删除行 */

extern struct   fsm_trans ttstab[];             /* 主fsm转移矩阵的压缩表示 */
extern int      ttstate;                        /* 记录主fsm的当前状态 */
extern u_char   ttfsm[NTSTATES][NCHRS];         /* 定义主fsm的转移矩阵 */

extern struct   fsm_trans substab[];            /* 子选项协商fsm转移矩阵的压缩表示 */
extern int      substate;                       /* 记录子选项协商fsm的当前状态 */
extern u_char   subfsm[NSSTATES][NCHRS];        /* 定义子选项协商fsm的转移矩阵 */

extern struct   fsm_trans sostab[];             /* 套接字输出fsm转移矩阵的压缩表示 */
extern int      sostate;                        /* 记录套接字输出fsm的当前状态 */
extern u_char   sofsm[NKSTATES][NCHRS];         /* 定义套接字输出fsm的转移矩阵 */

#endif /* _FSM_STRUCT_H_ */
