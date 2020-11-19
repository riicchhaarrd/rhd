#define HEAP_STRING_IMPL
#include "../heap_string.h"

static void print_bytes(const char *buf, size_t len)
{
	for(size_t i = 0; i < len; ++i)
	{
		printf("0x%02X (%c)", buf[i] & 0xff, buf[i]);
	}
	putchar('\n');
}

int main(void)
{
	heap_string s = NULL;
	
	for(size_t i = 0; i < 26; ++i)
		heap_string_push(&s, 'a' + i);
	
	printf("%s\n", s); //[a-z]
	
	printf("a through z is %ld characters\n", heap_string_size(&s));
	printf("capacity = %ld\n", heap_string_capacity(&s));
	
	heap_string_free(&s); //s should be NULL by now..
	
	s = heap_string_new("hello world");
	printf("s is now %s\n", s);
	
	heap_string_free(&s);
	
	s = heap_string_alloc(64); //allocate 64 bytes
	
	for(size_t i = 0; i < 9; ++i)
		heap_string_push(&s, '0' + i);
	
	heap_string_appendf(&s, "formatted data %d\n", 12345); //not-binary safe!
	printf("allocated string = %s\n", s);
	
	static const char buf[] = {1,2,3,4,5,0,5,3,2,6};
	
	heap_string_appendn(&s, buf, sizeof(buf)); //binary-safe
	
	print_bytes(s, heap_string_size(&s));
	
	heap_string_free(&s);
	return 0;
}