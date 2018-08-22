#ifndef _K_QUEUE_H_
#define _K_QUEUE_H_

#define TAILQ_HEAD(name, type)          \
    struct name                         \
    {                                   \
        struct type *tqh_first; /*第一个节点的地址 */       \
        struct type **tqh_last; /*链表最后一个节点的next指针的地址 *tqh_last = next*/ \
};

#define TAILQ_ENTRY(type)       \
    struct                      \
    {                           \
        struct type *tqe_next;  /*指向下一个节点*/  \
        struct type **tqe_prev; /*上一个节点的nex*/

}
#endif
