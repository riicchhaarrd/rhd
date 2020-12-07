#ifndef PARSE_H
#define PARSE_H

#include <stdio.h> //FILE
#include <stdlib.h> //atof
#include <ctype.h> //isspace
//should probably make heap_string a struct and forward declare it in the func prototype below
#include "heap_string.h"

#ifndef PARSE_IMPL
void parse_whitespace(FILE *fp);
void parse_skip_line(FILE *fp);
int parse_float(FILE *fp, float* out);
int parse_float3(FILE *fp, float *v);
/* don't forget to free ident! */
int parse_ident(FILE *fp, heap_string *ident);
int parse_character(FILE *fp, int ch);
int parse_characters(FILE *fp, const char *str);
int parse_ident_to_buffer(FILE *fp, char *buf, size_t bufsz, int *overflow);
int fpeekc(FILE *fp);
#else
int fpeekc(FILE *fp)
{
    int c = fgetc(fp);
    ungetc(c, fp);
    return c;
}

void parse_whitespace(FILE *fp)
{
	while(1)
	{
		int pk = fpeekc(fp);
		if(pk == EOF)
			return;
		if(pk != ' ' && pk != '\t')
			return;
		fgetc(fp);
	}
}

void parse_skip_line(FILE *fp)
{
	int c;
	do
	{
		c = fgetc(fp);
	} while(c != EOF && c != '\n');
}

int parse_float(FILE *fp, float* out)
{
	int c;
	char string[128]; //let's just allow up to 128..
	size_t stringindex = 0;
	do
	{
		if(stringindex >= sizeof(string))
			return 1;
		c = fgetc(fp);
		string[stringindex++ % sizeof(string)] = c;
	} while(c != EOF && ( c == 'e' || isdigit(c) || c == '-' || c == '.' ));
	*out = (float)atof(string);
	ungetc(c, fp); //we've parsed 1 too many
	return c == EOF ? 1 : 0;
}

int parse_float3(FILE *fp, float *v)
{
	parse_whitespace(fp);
	if(parse_float(fp, &v[0]))
		return 1;
	parse_whitespace(fp);
	if(parse_float(fp, &v[1]))
		return 1;
	parse_whitespace(fp);
	if(parse_float(fp, &v[2]))
		return 1;
	parse_whitespace(fp);
	return 0;
}

int parse_ident_to_buffer(FILE *fp, char *buf, size_t bufsz, int *overflow)
{
	if(overflow)
	*overflow = 0;
	parse_whitespace(fp);
	int c;
	size_t index = 0;
	
	for(;;)
	{
		if(index + 1 >= bufsz)
		{
			if(overflow)
			*overflow = 1;
			break;
		}
		c = fgetc(fp);
		if(c == EOF || isspace(c))
		{
			ungetc(c, fp); //unget space
			break;
		}
		buf[index++] = c;
	}
	buf[index] = '\0';
	return c == EOF ? 1 : 0;
}

/* don't forget to free ident! */

int parse_ident(FILE *fp, heap_string *ident)
{
	parse_whitespace(fp);
	int c;
	for(;;)
	{
		c = fgetc(fp);
		if(c == EOF || isspace(c))
		{
			ungetc(c, fp); //unget space
			break;
		}
		heap_string_push(ident, c);
	}
	if(c == EOF)
		heap_string_free(ident);
	return c == EOF ? 1 : 0;
}
#if 0
int parse_ident(FILE *fp, heap_string *ident)
{
	parse_whitespace(fp);
	int c;
	do
	{
		c = fgetc(fp);
		heap_string_push(ident, c);
	} while(c != EOF && !isspace(c));
	ungetc(c, fp); //eh, ungetting space..
	if(c == EOF) //free string..
		heap_string_free(ident);
	return c == EOF ? 1 : 0;
}
#endif

int parse_character(FILE *fp, int ch)
{
	parse_whitespace(fp);
	if(fpeekc(fp) != ch)
	{
		printf("expected %c got %c at %ld\n", ch, fpeekc(fp), ftell(fp));
		return 1;
	}
	fgetc(fp);
	parse_whitespace(fp);
	return 0;
}

int parse_characters(FILE *fp, const char *str)
{
	size_t len = strlen(str);
	for(size_t i = 0; i < len; ++i)
	{
		if(parse_character(fp, str[i]))
			return 1;
	}
	return 0;
}
#endif
#endif