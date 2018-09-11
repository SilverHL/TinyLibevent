/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/

#ifndef _K_EVSIGNAL_H_
#define _K_EVSIGNAL_H_

#include <signal.h>
#include "k_event-internal.h"

struct evsignal_info
{
    struct event ev_signal;
    int ev_signal_pair[2];      //用于通信的socketpair管道
    int ev_signal_added;        //标志signal事件是否已经注册
    volatile sig_atomic_t evsignal_caught;  //是够有信号发生的标记
    struct event_list evsigevents[NSIG];    //信号事件管理 结构体数组
    sig_atomic_t evsigcaught[NSIG];     //记录每个信号触发的次数

    struct sigaction **sh_old;      //记录原来signal处理的函数指针 
                                    //当信号注册的处理函数被清除 
                                    //需要重新设置处理函数
    int sh_old_max;
};

int evsignal_init(struct event_base *);
void evsignal_process(struct event_base *);
int evsignal_add(struct event *);
int evsignal_del(struct event *);
void evsignal_dealloc(struct event_base *);

#endif /*end of _K_EVSIGNAL_H_*/
