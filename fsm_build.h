/*
 *
 * Description: - 提供建立并初始化三个有限状态机（fsm）的接口，
 *                这三个fsm分别是：
 *                1. 主fsm，管理从套接字接收到的字符；
 *                2. 套接字输出fsm，管理从键盘输入的字符；
 *                3. 子选项协商fsm，管理子选项协商。
 *                这三个fsm使Telnet协议可以有序地工作。
 */   
  
#ifndef _FSM_BUILD_H_
#define _FSM_BUILD_H_

/**
 * fsmbuild - 对有限状态机进行初始化
 *
 *
 * 分别对主有限状态机、套接字输出状态机、子选项协商状态机进行初始化
 *
 * @Return : 
 *          void
 */
void fsmbuild();

#endif /* _FSM_BUILD_H_ */

