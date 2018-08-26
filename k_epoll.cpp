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

#include "k_queue.h"
#include "k_event.h"
#include "k_event-internal.h"
#include "k_evsignal.h"

struct evepoll
{
    struct event *evread;
    struct event *everite;
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

public:
    epollop();
    ~epollop();
    void epoll_init(struct event_base *);
    int epoll_add(void *, struct event *);
    int epoll_del(void *, struct event *);
    int epoll_dispatch(struct event_base *, void *, struct timeval *);
    void epoll_dealloc(struct event_base *, void *);
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

}

epollop::~epollop()
{

}

void 
epollop::epoll_init(struct event_base* base)
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
epollop::epoll_add(void *arg, struct event *ev)
{
    epollop *epop = static_cast<epollop *>(arg);
    struct epoll_event epev = {0, {0}};
    struct evepoll *evep;
    int fd, op, events;

    if (ev->ev_events & EV_SIGNAL)
    {
        return evsignal_add(ev);
    }

    fd = ev->event_fd;
       
}
