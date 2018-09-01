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

#include "k_evsignal.h"
#include "k_event.h"
#include "k_evutil.h"
#include "k_event-internal.h"

struct event_base *evsignal_base = NULL;
static void evsignal_handler(int sig);
static int _evsignal_set_handler(struct event_base *base, 
                                 int evsignal, void (*handler)(int));
static int _evsignal_restort_handler(struct event_base *base, int evsignal);

int _evsignal_restort_handler(struct event_base *base, int evsignal)
{
    int ret = 0;
    struct evsignal_info *sig = &base->sig;
    struct sigaction *sh;

    sh = sig->sh_old[evsignal];
}


