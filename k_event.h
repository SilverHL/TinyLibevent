/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/

#ifndef _K_EVENT_H_
#define _K_EVENT_H_

#include <sys/types.h>
#include <sys/time.h>

#include "k_evutil.h"
#include "k_queue.h"

//事件状态
#define EVLIST_TIMEOUT  0x01
#define EVLIST_INSERTED 0x02
#define EVLIST_SIGNAL   0x04
#define EVLIST_ACTIVE   0x08
#define EVLIST_INTERNAL 0x10
#define EVLIST_INIT     0x80

#define EVLIST_ALL  (0xf000 | 0x9f)

//事件类型
#define EV_TIMEOUT  0x01 
#define EV_READ     0x02
#define EV_WRITE    0x04
#define EV_SIGNAL   0x08
#define EV_PERSIST  0x10

TAILQ_HEAD (event_list, event);

struct event_base;

struct event
{
public:
    TAILQ_ENTRY (event) ev_next;
    TAILQ_ENTRY (event) ev_active_next;
    TAILQ_ENTRY (event) ev_signal_next;
    unsigned int min_heap_idx;
    event_base *ev_base;

    int event_fd;  //文件描述符或信号
    short ev_events;    //事件类型
    short ev_ncalls;    //事件就绪时ev_callback被执行的次数
    short *ev_pncalls; 

    struct timeval ev_timeout;

    int ev_pri;     

    void (*ev_callback)(int, short, void* arg); //分别为对应的描述符 监听的事件 参数

    int ev_res;         //返回的事件状态
    int ev_flags;       //当前event的状态
};

#define EVENT_SIGNAL(ev)    (int)(ev)->event_fd
#define EVENT_FD(ev)        (int)(ev)->event_fd

event_base* event_base_new(void);

event_base* event_init(void);

int event_base_loop(event_base *, int);

int event_base_dispatch(event_base *);

void event_base_free(event_base *);

int event_base_set(event_base *, event *);

int event_base_priority_init(event_base *, int);

int event_priority_set(event *, int);

#define evtimer_set(ev, cb, arg)    event_set(ev, -1, 0, cb, arg);
#define signal_set(ev, x, cb, arg)  event_set(ev, x, EV_SIGNAL | EV_PERSIST, cb, arg);
#define timeout_set(ev, cb, arg)    event_set(ev, -1, 0, cb, arg);

void event_set(event *, int, short, void (*)(int, short, void *), void *);

/*event loop flags */
#define EVLOOP_ONCE     0x1
#define EVLOOP_NONBLOCK 0x2

int event_loop(int);

#define evtimer_add(ev, tv)     event_add(ev, tv);  
#define signal_add(ev, tv)      event_add(ev, tv);
#define timeout_add(ev, tv)     event_add(ev, tv);
int event_add(struct event* ev, const struct timeval *timeout);

#define evtimer_del(ev)         event_del(ev);
#define signal_del(ev)          event_del(ev);
#define timeout_del(ev)         event_del(ev);
int event_del(struct event *);

void event_active(struct event*, int, short);
int event_dispatch(void);

#define evtimer_pending(ev, tv) event_pending(ev, EV_TIMEOUT, tv)
#define signal_pending(ev, tv)  event_pending(ev, EV_SIGNAL, tv)
#define timeout_pending(ev, tv) event_pending(ev, EV_TIMEOUT, tv)
int event_pending(struct event *ev, short event, struct timeval *tv);

struct evbuffer
{
    unsigned char *buffer;
    unsigned char *orig_buffer;
    size_t misalign;    //已经使用的长度
    size_t totallen;    //
    size_t off;         //未被使用的长度

    void (*cb)(struct evbuffer *, size_t, size_t, void *);
    void *cbargs;

};

#define EVBUFFER_READ       0x01
#define EVBUFFER_WRITE      0x02
#define EVBUFFER_EOF        0x10
#define EVBUFFER_ERROR      0x20
#define EVBUFFER_TIMEOUT    0x40

struct bufferevent;
typedef void (*evbuffercb)(struct bufferevent *, void *);
typedef void (*everrorcb)(struct bufferevent *, short what, void *);

struct event_watermark
{
    size_t low;
    size_t high;
};

struct bufferevent
{
    struct event_base *ev_base;

    struct event ev_read;
    struct event ev_write;

    struct evbuffer *input;
    struct evbuffer *output;

    struct event_watermark wm_read;
    struct event_watermark wm_write;

    evbuffercb readcb;
    evbuffercb writecb;
    everrorcb  errorcb;
    void *cbarg;

    int timeout_read;
    int timeout_write;

    short enabled;  //当前已经启用的事件
};

#define EVBUFFER_LENGTH(x)  (x)->off
#define EVBUFFER_DATA(x)    (x)->buffer
#define EVBUFFER_INPUT(x)   (x)->input
#define EVBUFFER_OUTPUT(x)  (x)->output

struct bufferevent *bufferevent_new(int fd, 
                                    evbuffercb readcb, 
                                    evbuffercb writecb, 
                                    everrorcb errorcb, 
                                    void *cbarg);

int bufferevent_base_set(struct event_base *base, struct bufferevent *bufev);

void bufferevent_priority_set(struct bufferevent *bufev, int pri);

void bufferevent_free(struct bufferevent *bufev);

void bufferevent_setcb(struct bufferevent *bufev, 
                       evbuffercb readcb, 
                       evbuffercb writecb, 
                       everrorcb errorcb, 
                       void *cbarg);

void bufferevent_setfd(struct bufferevent *bufev, int fd);

int bufferevent_write(struct bufferevent *bufev,
                      const void *data, size_t size);
int bufferevent_write_buffer(struct bufferevent *bufev, 
                             struct evbuffer *buf);

size_t bufferevent_read(struct bufferevent *bufev, 
                        void *data, size_t size);

int bufferevent_enable(struct bufferevent *bufev, short event);

int bufferevent_disable(struct bufferevent *bufev, short event);

void bufferevent_settimeout(struct bufferevent *bufev, 
                            int timeout_read, 
                            int timeout_write);

void bufferevent_setwatermark(struct bufferevent *bufev, 
                              short events, 
                              size_t lowmark, 
                              size_t highmark);
//evbuffer 中
struct evbuffer* evbuffer_new(void);

void evbuffer_free(struct evbuffer *buffer);

int evbuffer_add_buffer(struct evbuffer *outbuf, struct evbuffer *inbuf);

int evbuffer_add(struct evbuffer *buf, const void* data, size_t datlen);

void evbuffer_align(struct evbuffer *buf);

int evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen);

void evbuffer_drain(struct evbuffer *buf, size_t len);

char *evbuffer_readline(struct evbuffer *buffer);

int evbuffer_read(struct evbuffer *buf, int fd, int size);

int evbuffer_write(struct evbuffer *buffer, int fd);
#endif
