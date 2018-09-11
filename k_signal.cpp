/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/
#include <sys/types.h>
#include <sys/time.h>
#include <k_queue.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstdlib>
#include <fcntl.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <memory.h>
#include <unistd.h>

#include "k_event.h"
#include "k_evutil.h"
#include "k_event-internal.h"

struct event_base *evsignal_base = NULL;
static void evsignal_handler(int sig);
static int _evsignal_set_handler(struct event_base *base, 
                                 int evsignal, void (*handler)(int));
static int _evsignal_restort_handler(struct event_base *base, int evsignal);

int _evsignal_restore_handler(struct event_base *base, int evsignal)
{
    int ret = 0;
    struct evsignal_info *sig = &base->sig;
    struct sigaction *sh;

    sh = sig->sh_old[evsignal];
    if (sigaction(evsignal, sh, NULL) == -1)
    {
        std::cout << "sigaction error\n" ;
        ret = -1;
    }

    free(sh);
    return ret;
}

/***
 * 为evsignal设置信号处理程序
 * **/
int _ev_signal_set_handler(struct event_base *base, int evsignal, void(*handler)(int))
{
    struct sigaction sa;
    struct evsignal_info *sig = &base->sig;
    void *p;
    if (evsignal >= sig->sh_old_max)
    {
        int new_max = evsignal+1;
        //增加信号量就要重新分配信号存储的空间
        p = realloc(sig->sh_old, new_max * sizeof(*sig->sh_old));
        if (p == NULL)
        {
            std::cout << "realloc error\n";
            return -1;
        }
        memset((char*)p + sig->sh_old_max * sizeof(*sig->sh_old), 
               0, (new_max - sig->sh_old_max) * sizeof(*sig->sh_old));
        sig->sh_old_max = new_max;
        sig->sh_old = reinterpret_cast<struct sigaction **>(p);
    }
    sig->sh_old[evsignal] = static_cast<struct sigaction*>
                            (malloc(sizeof(*sig->sh_old[evsignal])));
    if (sig->sh_old[evsignal] == NULL)
    {
        perror("malloc");
        return -1;
    }
    //保存以前的处理程序和设置新的处理程序
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(evsignal, &sa, sig->sh_old[evsignal]) == -1)
    {
        std::cout << "sigaction error\n";
        free(sig->sh_old[evsignal]);
        sig->sh_old[evsignal] = NULL;
        return -1;
    }
    return 0;
}

void evsignal_handler(int sig)
{
    int save_errno = errno;
    if (evsignal_base == NULL)
    {
        std::cout << __func__ << ": received signal " << 
            sig << ", but have no base configured\n";
        return ;
    }
    evsignal_base->sig.evsigcaught[sig]++;          //对应信号被捕捉 
    evsignal_base->sig.evsignal_caught = 1;         //有信号发生了
    /*唤醒通知机制*/
    send(evsignal_base->sig.ev_signal_pair[0], "a", 1, 0);
    errno = save_errno;
}

static void evsignal_cb(int fd, short what, void *arg)
{
    static char signals[1];
    ssize_t n;
    n = recv(fd, signals, sizeof(signals), 0); //从fd 接受信息
    if (n == -1)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            std::cout << "recv\n";
            return;
        }
    }
}

#define FD_CLOSEONEXEC(x) do {      \
    if (fcntl(x, F_SETFD, 1) == -1) \
        std::cout << "fcntl\n"; \
} while (0)

int evsignal_init(struct event_base *base)
{
    int i;
    if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, base->sig.ev_signal_pair) == -1)
    {
        std::cout << "evutil_socketpair errno\n";
        return -1;
    }

    FD_CLOSEONEXEC(base->sig.ev_signal_pair[0]);
    FD_CLOSEONEXEC(base->sig.ev_signal_pair[1]);
    base->sig.sh_old = NULL;
    base->sig.sh_old_max = 0;
    base->sig.evsignal_caught = 0;
    memset(&base->sig.evsigcaught, 0, sizeof(sig_atomic_t)*NSIG);

    for (i = 0; i < NSIG; ++i)
        TAILQ_INIT(&base->sig.evsigevents[i]);
    evutil_make_socket_nonblocking(base->sig.ev_signal_pair[0]);
    evutil_make_socket_nonblocking(base->sig.ev_signal_pair[1]);

    event_set(&base->sig.ev_signal, base->sig.ev_signal_pair[1], 
              EV_READ | EV_PERSIST, evsignal_cb, &base->sig.ev_signal);
    base->sig.ev_signal.ev_base = base;
    base->sig.ev_signal.ev_flags |= EVLIST_INTERNAL;    

    return 0;
}

int evsignal_add(struct event *ev)
{
    int evsignal;
    struct event_base *base = ev->ev_base;
    struct evsignal_info *sig = &ev->ev_base->sig;

    if (ev->ev_events & (EV_READ | EV_WRITE))
    {
        std::cout << " this event is not signal event";
        return -1;
    }

    evsignal = EVENT_SIGNAL(ev);
    assert(evsignal > 0 && evsignal < NSIG);
    if (TAILQ_EMPTY(&sig->evsigevents[evsignal]))
    {
        if (_evsignal_set_handler(base, evsignal, evsignal_handler) == -1)
        {
            return -1;
        }
        evsignal_base = base;
        if (!sig->ev_signal_added)
        {
            if (event_add(&sig->ev_signal, NULL))
                return -1;
            sig->ev_signal_added = 1;
        }
    }

    TAILQ_INSERT_TAIL(&sig->evsigevents[evsignal], ev, ev_signal_next);
    return 0;
}

void evsignal_process(struct event_base *base)
{
    struct evsignal_info *sig = &base->sig;
    struct event *ev, *next_ev;
    sig_atomic_t ncalls; 
    int i;
    
    base->sig.evsignal_caught = 0;
    for (i = 1; i < NSIG; ++i)
    {
        ncalls = sig->evsigcaught[i];
        if (ncalls == 0)
            continue;

        sig->evsigcaught[i] -= ncalls;
        for (ev = TAILQ_FIRST(&sig->evsigevents[i]); ev != NULL; ev = next_ev)
        {
            next_ev = TAILQ_NEXT(ev, ev_signal_next);
            if (!(ev->ev_events & EV_PERSIST))
                event_del(ev);
            event_active(ev, EV_SIGNAL, ncalls);
        }
    }
}

int evsignal_del(struct event *ev)
{
    struct event_base *base = ev->ev_base;
    struct evsignal_info *sig = &base->sig;
    int evsignal = EVENT_SIGNAL(ev);

    assert(evsignal >= 0 && evsignal < NSIG);

    TAILQ_REMOVE(&sig->evsigevents[evsignal], ev, ev_signal_next);

    if (!TAILQ_EMPTY(&sig->evsigevents[evsignal]))
        return 0;

    std::cout << __func__ << ev << "restoring signal handler\n";

    return (_evsignal_restore_handler(ev->ev_base, EVENT_SIGNAL(ev)));
}

void evsignal_dealloc(struct event_base *base)
{
    int i = 0;
    if (base->sig.ev_signal_added)
    {
        event_del(&base->sig.ev_signal);
        base->sig.ev_signal_added = 0;
    }
    for (i = 0; i < NSIG; ++i)
    {
        if (i < base->sig.sh_old_max && base->sig.sh_old[i] != NULL)
            _evsignal_restort_handler(base, i);
    }
}
