/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/


#ifndef _K_EVUTIL_H_
#define _K_EVUTIL_H_

int evutil_socketpair(int family, int type, int protocol, int fd[2]);
int evutil_make_socket_nonblocking(int sock);
int evutil_make_socket_reuseable(int sock);

#define evutil_timeradd(tvp, uvp, vvp)  \
    do {                                                    \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;      \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;   \
        if ((vvp)->tv_usec >= 1000000) {                    \
            (vvp)->tv_src++;                                \
            (vvp)->tv_usec -= 1000000;                      \
        }                                                   \
    } while (0)

#define evutil_timersub(tvp, uvp, vvp)                      \
    do {                                                    \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;      \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;   \
        if ((vvp)->tv_usec < 0) {                           \
            (vvp)->tv_sec--;                                \
            (vvp)->tv_usec += 1000000;                      \
        }                                                   \
    } while (0)

#define evutil_timercmp(tvp, uvp, cmp)                      \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?                     \
     ((tvp)->tv_usec cmp (uvp)->tv_usec) :                  \
     ((tvp)->tv_sec cmp (uvp)->tv_sec)) 


#define evutil_timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0

#define EVUTIL_CLOSESOCKET(s)   close(s)

#endif
