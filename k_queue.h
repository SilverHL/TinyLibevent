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
        struct type **tqe_prev; /*上一个节点的next 即entry本身*/ \
}

#define TAILQ_INIT(head) do {               \
    (head)->tqh_first = NULL;               \
    (head)->tqh_first = &(head)->tqh_first; \
} while (0)

#define TAILQ_INSERT_TAIL(head, elm, field) do {        \
    (elm)->field.tqe_next = NULL;                   \
    (elm)->field.tqe_prev = (head)->tqh_last;       \
    *(head)->tqh_last = (elm);                      \
    (head)->tqh_last = &(elm)->field.tqe_next;      \
} while (0)

#define TAILQ_REMOVE(head, elm, field) do {     \
    if (((elm)->field.tqe_next) ! =NULL)        \
        (elm)->field.tqe_next->field.tqe_prev =     \
            (elm)->field.tqe_prev;              \
    else                        \
        (head)->tqh_last = (elm)->field.tqe_prev;   \
    *(elm)->field.tqe_prev = (elm)->field.tqe_next; \
} while(0)
#endif
