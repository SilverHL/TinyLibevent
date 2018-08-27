#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/param.h>

#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#include "k_queue.h"
#include "k_event.h"
#include "k_event-internal.h"
#include "k_evsignal.h"

struct evepoll
{
    struct event *evread;
    struct event *evwrite;
};

class epollop :
    public eventop
{
private:
    struct evepoll *fds;
    int nfds;
    struct epoll_event *events;
    int nevents;
    int epollfd;

private:
    int epoll_recalc(struct event_base *base, int max);
    void epoll_dealloc();
    void epoll_init();

public:
    epollop();
    explicit epollop(struct event_base*);
    ~epollop();
    int epoll_add(struct event *);
    int epoll_del(struct event *);
    int epoll_dispatch(struct timeval *);

};

#define FD_CLOSEONEXEC(x) do { \
    if (fcntl(x, F_SETFD, 1) == -1) \
        perror("fcntl");    \
} while (0)

#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)

#define INITIAL_NFILES  32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS     4096

epollop::epollop()
{
    need_reinit = 1;
    std::cout << "wrong" << std::endl;

}

epollop::epollop(struct event_base* base)
{
    need_reinit = 1;
    this->base = base;
    epoll_init();
}

epollop::~epollop()
{
    epoll_dealloc();
}

void 
epollop::epoll_init()
{
    if ((epollfd = epoll_create(32000)) == -1)
    {
       if (errno != ENOSYS)
           std::cout << "epoll_create function not implemented!\n";
       return;
    }

    FD_CLOSEONEXEC(epollfd);
    events = (struct epoll_event*)malloc
                (INITIAL_NEVENTS * sizeof(struct epoll_event));
    if (events == NULL)
    {
        perror("malloc");
        return;
    }
    nevents = INITIAL_NEVENTS;
    fds = (struct evepoll*)calloc(INITIAL_NFILES, sizeof(struct evepoll));
    if (fds == NULL)
    {
        free(events);
        return;
    }

    nfds = INITIAL_NFILES;
    evsignal_init(base);
}

int
epollop::epoll_add(struct event *ev)
{
    struct epoll_event epev = {0, {0}};
    struct evepoll *evep;
    int fd, op, events;

    if (ev->ev_events & EV_SIGNAL)
    {
        return evsignal_add(ev);
    }

    //首先获得当前event的fd
    fd = ev->event_fd;
    if (fd > nfds)
    {
        if (epoll_recalc(ev->ev_base, fd) == -1)
            return -1;
    }

    evep = &fds[fd];  //将其取出
    op = EPOLL_CTL_ADD;
    events = 0;
    if (evep->evread != NULL)  //可读
    {
        events |= EPOLLIN;
        op = EPOLL_CTL_MOD;
    }

    if (evep->evwrite != NULL)
    {
        events |=  EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }

    if (ev->ev_events & EV_READ)
        events |= EPOLLIN;
    if (ev->ev_events & EV_WRITE)
        events |= EPOLLOUT;

    epev.data.fd = fd;
    epev.events = events;

    if (epoll_ctl(epollfd, op, ev->event_fd, &epev) == -1)
        return -1;

    if (ev->ev_events & EV_READ)
        evep->evread = ev;
    if (ev->ev_events & EV_WRITE)
        evep->evwrite = ev;

    return 0;
}

int 
epollop::epoll_del(struct event *ev)
{
    struct epoll_event epev = {0, {0}};
    struct evepoll *evep;
    int fd, op, events;
    int need_write_delete = 1, need_read_delete = 1;

    if (ev->ev_events & EV_SIGNAL)
        return evsignal_del(ev);

    fd = ev->event_fd;
    if (fd >= nfds)
        return 0;
    evep = &fds[fd];
    op = EPOLL_CTL_DEL;
    events = 0;
    
    if (ev->ev_events | EV_READ)
        events |= EPOLLIN;
    if (ev->ev_events | EV_WRITE)
        events |= EPOLLOUT;

    if ((events & (EPOLLIN | EPOLLOUT)) != (EPOLLIN | EPOLLOUT))
    {
        if ((events & EPOLLIN) && evep->evwrite != NULL)
        {
            need_write_delete = 0;
            events = EPOLLOUT;
            op = EPOLL_CTL_MOD;
        }
        else if ((events & EPOLLOUT) && evep->evread != NULL)
        {
            need_read_delete = 0;
            events = EPOLLIN;
            op = EPOLL_CTL_MOD;
        }
    }

    epev.events = events;
    epev.data.fd = fd;

    if (need_write_delete)
        evep->evwrite = NULL;
    if (need_read_delete)
        evep->evread = NULL;

    if (epoll_ctl(epollfd, op, fd, &epev) == -1)
        return -1;

    return 0;
}
int 
epollop::epoll_recalc(struct event_base* base, int max)
{
    if (max > nfds)
    {
        struct evepoll *new_fds;
        int new_nfds = nfds;
        while (new_nfds <= max)
            new_nfds <<= 1;
        new_fds = static_cast<struct evepoll*>
                    (realloc(fds, new_nfds * sizeof(struct evepoll)));
        if (new_fds == NULL)
        {
            perror("realloc");
            return -1;
        }
        free(fds);
        fds = new_fds;
        new_fds = NULL;
        memset(fds + nfds, 0, (new_nfds - nfds) * sizeof(struct evepoll));
        nfds = new_nfds;
    }
    
    return 0;
}

void 
epollop::epoll_dealloc()
{
    assert(base != NULL);
    evsignal_dealloc(base);

    if (fds)
        free(fds);
    if (events)
        free(events);
    if (epollfd >= 0)
        close(epollfd);
}

int 
epollop::epoll_dispatch(struct timeval *tv)
{
    struct epoll_event *events = events;
    struct evepoll *evep;
    int i, res, timeout = -1;

    if (tv != NULL)
        timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;

    if (timeout > MAX_EPOLL_TIMEOUT_MSEC)
        timeout = MAX_EPOLL_TIMEOUT_MSEC;
    res = epoll_wait(epollfd, events, nevents, timeout);

    if (res == -1)
    {
        if (errno != EINTR)
            perror("epoll_wait");

        evsignal_process(base);
        return 0;
    }
    else if (base->sig.evsignal_caught)
    {
        evsignal_process(base);
    }

    for (i = 0; i < res; i++)
    {
        int what = events[i].events;
        struct event *evread = NULL, *evwrite = NULL;
        int fd = events[i].data.fd;

        if (fd < 0 || fd >= nfds)
            continue;

        if (what & (EPOLLHUP | EPOLLERR))
        {
            evread = evep->evread;
            evwrite = evep->evwrite;
        }
        else 
        {
            if (what & EPOLLIN)
                evread = evep->evread;
            if (what & EPOLLOUT)
                evwrite = evep->evwrite;
        }

        if (!(evread || evwrite))
            continue;

        if (evread != NULL)
            event_active(evread, EV_READ, 1);
        if (evwrite != NULL)
            event_active(evwrite, EV_WRITE, 1);
    }

    if (res == nevents && nevents < MAX_NEVENTS)
    {
        int new_nevents = nevents * 2;
        struct epoll_event *new_events;

        new_events = static_cast<struct epoll_event*> 
                 (realloc(events, new_nevents * sizeof(struct epoll_event)));

        if (new_events)
        {
            events = new_events;
            nevents = new_nevents;
        }
    }

    return 0;
}
