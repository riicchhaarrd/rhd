#ifndef MEMORY_H
#define MEMORY_H

#include <malloc.h>
#include <stddef.h>

typedef void*(*allocator_t)(size_t);
typedef void(*deallocator_t)(void*);

#define memory_allocate malloc
#define memory_deallocate free
#endif