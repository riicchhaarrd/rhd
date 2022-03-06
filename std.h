#ifndef RHD_STD_H
#define RHD_STD_H

#include <stdio.h>
#include <string.h>

static
int std_fopen_s(FILE **fp, const char *filename, const char *mode)
{
	#ifdef _WIN32
		fopen_s(fp, filename, mode);
	#else
		*fp = fopen(filename, mode);
	#endif
	//unused
	return 0;
}

static
int std_strncpy_s(char *dest, size_t destsz, const char *src, size_t count)
{
	#ifdef _WIN32
		strncpy_s(dest, destsz, src, count);
	#else
		strncpy(dest, src, count);
		dest[destsz - 1] = 0;
	#endif
	return 0;
}

static
int std_strerror_s(char *buf, size_t sz, int errnum)
{
	#ifdef _WIN32
		return strerror_s(buf, sz, errnum);
	#else
		snprintf(buf, sz, "%s", strerror(errnum));
		return 0;
	#endif
}
#endif
