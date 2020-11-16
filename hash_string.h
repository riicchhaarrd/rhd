#ifndef HASH_STRING
#define HASH_STRING

//http://www.cse.yorku.ca/~oz/hash.html
static inline unsigned long
hash_buffer(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

#define hash_string(str) hash_buffer((unsigned char*)str)

#endif