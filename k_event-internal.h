#ifndef _K_EVENTINTERNAL_H_
#define _K_EVENTINTERNAL_H_

#include "k_min_heap.h"
#include "k_evsignal.h"

class eventop
{
protected:

    virtual void init(struct event_base *);
    virtual int add(void *, struct event *);
    virtual int del(void *, struct event *);
    virtual int dispatch(struct event_base *, void *, struct timeval *);
    virtual void dealloc(struct event_base *, void *);

    int need_reinit;
};

class event_base
{
private:
    const eventop *evsel;       //父类指针
    void *evbase;
    int event_count; 
    int event_count_active;
    int event_gotterm;
    int event_break;
    struct event_list **avtivequeues;
    int nactivequeues;
    struct evsignal_info sig;
    struct event_list eventqueue;       //事件队列 双向链表
    struct timeval  event_tv;           //dispatch返回的时间  即IO就绪的时间
    min_heap timeheap;           //管理定时事件的小跟堆
};

#define TAILQ_FIRST(head)       ((heap)->tqh_first)
#define TAILQ_END(head)         NULL
#define TAILQ_NEXT(elm, field)  ((elm)->field.tqe_next)
#define TAILQ_EMPTY(head)       \
    (TAILQ_FIRST(head) == TAILQ_END(head))


#endif
