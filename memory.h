#ifndef MEMORY_H
#define MEMORY_H

#include <malloc.h>
#include <stddef.h>

typedef void*(*allocator_t)(size_t);
typedef void(*deallocator_t)(void*);

typedef void*(*custom_allocator_fn_t)(void *userptr, size_t nbytes);

#define memory_allocate malloc
#define memory_deallocate free
#endif