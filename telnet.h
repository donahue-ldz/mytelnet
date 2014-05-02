/*
 * Description: - telnet客户端用到的命令的宏定义，以及终端、主机的声明。
 *
 */
 
#ifndef _TELNET_H_
#define _TELNET_H_

typedef unsigned char u_char;

#define     SUCCESS     0
#define     FAIL        -1
#define     VALUE       1             /* 值有效 */

/* TELNET 命令字符: */
#define    TCSB        (u_char)250    /* 开始子选项协商*/
#define    TCSE        (u_char)240    /* 结束子选项协商*/
#define    TCNOP       (u_char)241    /* 无操作*/
#define    TCDM        (u_char)242    /* Data Mark (同步标志) */
#define    TCIP        (u_char)244    /* 中断进程 */
#define    TCAO        (u_char)245    /* 中止输出 */
#define    TCEC        (u_char)247    /* 删除字符*/
#define    TCEL        (u_char)248    /* 删除行*/
#define    TCGA        (u_char)249    /* Go Ahead，用于激活一次一个字符的操作方式 */
#define    TCWILL      (u_char)251    /* Will 选项 */
#define    TCWONT      (u_char)252    /* Wont选项 */
#define    TCDO        (u_char)253    /* Do选项 */
#define    TCDONT      (u_char)254    /* Dont选项 */
#define    TCIAC       (u_char)255    /* 命令开始标志（Interpret As Command Escape） */

/* Telnet 选项字符: */
#define    TOTXBINARY  (u_char)0      /* 二进制传送 */
#define    TOECHO      (u_char)1      /* 回显选项 */
#define    TONOGA      (u_char)3      /* 禁止Go Ahead */
#define    TOTERMTYPE  (u_char)24     /* 终端类型选项 */

/*  特殊字符: */
#define    VPLF        '\n'           /* 换行 */
#define    VPCR        '\r'           /* 回车 */
#define    VPBEL       '\a'           /* Bell 注意信号 */
#define    VPBS        '\b'           /* 退格 */
#define    VPHT        '\t'           /* 水平制表符 */
#define    VPVT        '\v'           /* 垂直制表符 */
#define    VPFF        '\f'           /* 换页 */

/*  键盘命令: */
#define    KCESCAPE    035            /* 转义字符('^]') */
#define    KCDCON      '.'            /* 断开命令 */
#define    KCSUSP      032            /* 挂起会话('^Z') */
#define    KCSTATUS    024            /* 打印状态信息 ('^T') */
#define    KCNL        '\n'           /* 换行 */
#define    KCCHANGE_CR 'l'            /* 转换换行方式LF--LF-CR */

#define    KCANY       (NCHRS+1)      /* 未定义的字符 */

/* 子选项: */
#define    TT_IS       0              /* TERMINAL_TYPE 选项的 "IS" 命令 */
#define    TT_SEND     1              /* TERMINAL_TYPE 选项的 "SEND" 命令 */

extern struct termios   oldtty;       /* 该结构保存原来的终端设置 */
extern struct termios   tntty;        /* 设置telnet终端的结构体 */
extern char             *host;        /* 服务端地址 */

#endif /* _TELNET_H_ */

