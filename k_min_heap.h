/**************************************************
 *
 * C++11 TinyLibevent
 *
 * Author: @github/SilverHL
 * ***********************************************/


#ifndef _K_MIN_HEAP_H_
#define _K_MIN_HEAP_H_

#include <cstdlib>

#include "k_event.h"
#include "k_evutil.h"

enum _INIT_MIN_HEAP_SIZE { _init_min_heap_size = 8 };

class min_heap
{
private:
    struct event** timeout_event;
    unsigned int capacity;
    unsigned int used_size; 

public:
    min_heap();
    ~min_heap();
    
    inline void min_heap_elem_init(event*);
    int         min_heap_elem_greater(event*);
    
    bool        min_heap_empty();
    unsigned    min_heap_size();
    int         min_heap_reverve(unsigned);
    
    event *     min_heap_top();
    int         min_heap_push(event*);
    event *     min_heap_pop();
    int         min_heap_erase(event*);

    void        min_heap_shift_up(unsigned int);
    void        min_heap_shift_down(unsigned int);
};

min_heap::min_heap()
{
    capacity = _init_min_heap_size;
    timeout_event = (event**)malloc(capacity * sizeof((void*)0));
    used_size = 0;
}

void 
min_heap::min_heap_elem_init(event *e)
{
    e->min_heap_idx = -1;

}

int 
min_heap::min_heap_reverve(unsigned new_capacity)
{
    if (capacity < new_capacity)
    {
        capacity = capacity ? capacity * 2 : 8;
        if (capacity < new_capacity)
            capacity = new_capacity;
        if (!(timeout_event = 
              (struct event**)realloc
              (timeout_event, capacity * sizeof((*timeout_event)))))
        {

        }
    }
    return 0;
}

#endif
