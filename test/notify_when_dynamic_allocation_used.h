#ifndef _NOTIFY_WHEN_DYNAMIC_ALLOCATION_USED_H_
#define _NOTIFY_WHEN_DYNAMIC_ALLOCATION_USED_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void *operator new(size_t size)
{
    printf("called new(%zu)!\n", size);
    return malloc(size);
}

#endif // _NOTIFY_WHEN_DYNAMIC_ALLOCATION_USED_H_
