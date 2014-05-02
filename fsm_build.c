/*
 *
 * Description: - 建立并初始化三个有限状态机（fsm），分别是：
 *                1. 主fsm，管理从套接字接收到的字符；
 *                2. 套接字输出fsm，管理从键盘输入的字符；
 *                3. 子选项协商fsm，管理子选项协商。
 *                这三个fsm使Telnet协议可以有序地工作。
 *
 */

#include <sys/types.h>
#include <stdio.h>

#include "telnet.h"
#include "fsm_struct.h"

#define    TINVALID    0xff    /* 无效的转移项 */

/*
 * fsminit - 用压缩表示来生成转移矩阵
 *
 * @fsm : 指明须被初始化的转移矩阵
 * @ttab: 压缩有限状态机表示的地址
 * @nstates: 有限状态机的状态数目
 *
 * 首先把转移矩阵的全部项初始化成TINVALID。
 * 然后循环扫描压缩表示的每个元素，
 * 并把由该元素所指定的状态转移加到转移矩阵中去。
 * 最后，它再次扫描整个转移矩阵，
 * 修改还没有填充的转移，使它们指向压缩表示末尾的非法状态转移项。
 *
 */
static void fsminit(unsigned char fsm[][NCHRS], struct fsm_trans ttab[], int nstates)
{
    struct fsm_trans    *pt;
    int                 sn;
    int                 ti;
    int                 cn;

    for (cn = 0; cn < NCHRS; ++cn) {
        for (ti=0; ti < nstates; ++ti) {
            fsm[ti][cn] = TINVALID;
        }
    }

    for (ti = 0; ttab[ti].ft_state != FSINVALID; ++ti) {
        pt = &ttab[ti];
        sn = pt->ft_state;
        if (pt->ft_char == TCANY) {
            for (cn = 0; cn < NCHRS; ++cn) {
                if (fsm[sn][cn] == TINVALID) {
                    fsm[sn][cn] = ti;
                }
            }
        } else {
            fsm[sn][pt->ft_char] = ti;
        }
    }

    /* 填充为非法转移项 */
    for (cn=0; cn < NCHRS; ++cn) {
        for (sn = 0; sn < nstates; ++sn) {
            if (fsm[sn][cn] == TINVALID) {
                fsm[sn][cn] = ti;
            }
        }
    }

}

/**
 * fsmbuild - 对有限状态机进行初始化
 *
 *
 * 分别对主有限状态机、套接字输出状态机、子选项协商状态机进行初始化
 *
 * @Return :
 *          void
 */
void fsmbuild()
{

    fsminit(ttfsm, ttstab, NTSTATES);
    ttstate = TSDATA;

    fsminit(sofsm, sostab, NKSTATES);
    sostate = KSREMOTE;

    fsminit(subfsm, substab, NSSTATES);
    substate = SS_START;

}

