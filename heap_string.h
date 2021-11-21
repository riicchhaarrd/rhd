#ifndef HEAP_STRING_H
#define HEAP_STRING_H

#include "std.h"
#include <stdio.h> //vsnprintf
#include <malloc.h>
#include <string.h>
#include <stdarg.h>

//msvc warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning( push )
#pragma warning( disable : 4200 )
struct heap_string_header
{
	int capacity;
	int size;
	char buf[];
};
#pragma warning( pop )

#define HEAP_STRING_HDR(s) (struct heap_string_header *)( ( s ) - sizeof(struct heap_string_header) )

//#define heap_string char*

typedef char* heap_string;

void heap_string_appendn(heap_string *s, const char *str, size_t n);
void heap_string_append(heap_string *s, const char *str);
#ifndef HEAP_STRING_IMPL

heap_string heap_string_alloc(int n);
heap_string heap_string_new(const char* s);
void heap_string_free(heap_string *s);
size_t heap_string_capacity(heap_string *s);
size_t heap_string_size(heap_string *s);
void heap_string_push(heap_string *s, int i);
void heap_string_appendf(heap_string *s, const char *fmt, ...);
heap_string heap_string_read_from_text_file( const char* filename );
#else
heap_string heap_string_alloc(int n)
{
	struct heap_string_header *d = (struct heap_string_header*)malloc(sizeof(struct heap_string_header) + n + 1);
	d->capacity = n;
	d->size = 0;
	return (heap_string)&d->buf[0];
}

heap_string heap_string_read_from_text_file( const char* filename )
{
	heap_string data = NULL;

	FILE* fp;
	std_fopen_s(&fp, filename, "r");
	if ( !fp )
		return data;
	fseek( fp, 0, SEEK_END );
	size_t fs = ftell( fp );
	rewind( fp );
	data = heap_string_alloc( fs + 1 );
	fread( data, fs, 1, fp );
    
	struct heap_string_header *hdr = HEAP_STRING_HDR(data);
	hdr->buf[fs] = '\0';
    	hdr->size = fs;

	fclose( fp );
	return data;
}

//not binary safe
heap_string heap_string_new(const char* s)
{
	int strsz = strlen(s);
	struct heap_string_header *d = (struct heap_string_header*)malloc(sizeof(struct heap_string_header) + strsz + 1);
	d->capacity = strsz;
	d->size = d->capacity;
	memcpy(d->buf, s, strsz + 1);
	return d->buf;
}

void heap_string_free(heap_string *s)
{
	if (!s)
		return;
	if (!*s)
		return;
	struct heap_string_header *hdr = HEAP_STRING_HDR(*s);
	free(hdr);
	*s = NULL;
}

size_t heap_string_capacity(heap_string *s)
{
	struct heap_string_header *hdr = HEAP_STRING_HDR(*s);
	return hdr->capacity;
}

size_t heap_string_size(heap_string *s)
{
	if(!s || !*s) return 0;
	struct heap_string_header *hdr = HEAP_STRING_HDR(*s);
	return hdr->size;
}

void heap_string_push(heap_string *s, int i)
{
	struct heap_string_header *hdr = 0;
	if(*s)
		hdr = HEAP_STRING_HDR(*s);
	int newsize = 4;
	if (*s)
		newsize = hdr->size;

	if (!*s || newsize + 1 >= hdr->capacity)
	{
		heap_string n = heap_string_alloc(newsize + 2); //slower, but less likely to run into allocation issues due to large N in successive calls

		if (*s)
		{
			heap_string_appendn(&n, *s, hdr->size);
		}
		heap_string_push(&n, i);
		heap_string_free(s);
		*s = n;
	}
	else {
		hdr->buf[hdr->size] = i & 0xff;
		++hdr->size;
		hdr->buf[hdr->size] = '\0';
	}
}

static inline void heap_string_transfer(heap_string* a, heap_string *b)
{
	*a = *b;
	*b = NULL;
}

//not binary safe, avoid using the formatted functions for binary data
void heap_string_appendf(heap_string *s, const char *fmt, ...)
{
#define NBUF (10)
	static char buffer[NBUF][1024] = { 0 };
	static int bufIndex = 0;

	char *buf = &buffer[bufIndex++ % NBUF][0];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buffer[0]), fmt, args);
	heap_string_append(s, buf);
	va_end(args);
}

void heap_string_appendn(heap_string *s, const char *str, size_t n)
{
#if 0 //binary safe
        int sl = strlen(str);
        if (n >= sl)
                n = sl;
#endif
        if (*s == NULL)
                *s = heap_string_alloc(n);

        for (size_t i = 0; i < n; ++i)
        {
                heap_string_push(s, *(char*)(str + i));
        }
}

//binary unsafe, it won't change the existing data, but won't add any data past \0
void heap_string_append(heap_string *s, const char *str)
{
        heap_string_appendn(s, str, strlen(str));
}
#endif
#endif
