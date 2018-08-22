/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/

#include <sys/types.h>
#include <sys/time.h>


#define SIZE_MAX ((size_t)-1)
#define EVBUFFER_MAX_READ   4096
#define SWAP(x, y) do { \
    (x)->buffer = (y)->buffer; \
    (x)->orig_buffer = (y)->orig_buffer; \
    (x)->misalign = (y)->misalign; \
    (x)->totallen = (y)->totallen; \
    (x)->off = (y)->off; \ 
} while (0)

struct evbuffer* evbuffer_new(void)
{

}

