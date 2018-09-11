/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "k_queue.h"
#include "k_event.h"
#include "k_event-internal.h"
#include "k_evutil.h"

int evutil_socketpair(int family, int type, int protocol, int fd[2])
{
    return socketpair(family, type, protocol, fd);
}

int evutil_make_socket_unblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0)
    {
        std::cout << "fcntl\n";
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cout << "fcntl F_SETFL error" << __FILE__ << "\n";
        return -1;
    }

    return 0;
}

int evutil_make_socket_reuseable(int sock)
{
    int on = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
}

