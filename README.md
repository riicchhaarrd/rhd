# rhd

This is a repository for common/useful C header file implementations I've made and use myself.
Feel free to use them in your own project.

# heap_string.h example
```c
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
```

# linked_list.h example
```c
#define LINKED_LIST_IMPL
#include "../linked_list.h"

int linked_list_test_int(void)
{
	struct linked_list *list = NULL;
	
	list = linked_list_create(int);
	
	for(int i = 0; i < 32; ++i)
		linked_list_prepend(list, (int){i+123});
	
	linked_list_reversed_foreach(list, int*, it,
	{
		printf("it %d\n", *it);
	});
	
	linked_list_destroy(&list);
}

void on_each_string(char **p)
{
	printf("each string: %s\n", *p);
	free(*p);
}

int linked_list_test_heap_allocated_string(void)
{
	struct linked_list *list = NULL;
	
	list = linked_list_create(char*);
	linked_list_set_node_value_finalizer(list, (deallocator_t)on_each_string);
	
	for(int i = 0; i < 4; ++i)
		linked_list_prepend(list, (char*){strdup("hello")});
	
	linked_list_foreach(list, char**, it,
	{
		printf("it %s\n", *it);
	});
	
	linked_list_destroy(&list);
}

int main(void)
{
	linked_list_test_heap_allocated_string();
	linked_list_test_int();
}
```

# hash_map.h example

```c
#define HASH_MAP_IMPL
#include "../hash_map.h"

static void on_remove_value(char **p)
{
	printf("freeing string '%s'\n", *p);
	free(*p);
	*p = NULL;
}

void example_heap_allocated_string()
{
	struct hash_map *hm = hash_map_create(char*);
	hash_map_set_on_key_removal(hm, (deallocator_t)on_remove_value);
	
	hash_map_insert(hm, "test", (char*){strdup("test")});

	char **s = hash_map_find(hm, "test");
	if(s)
		printf("found string '%s'\n", *s);

	hash_map_destroy(&hm);
}

void example_int()
{
	
	struct hash_map *hm = hash_map_create(int);
	for(int i = 0; i < 4; ++i)
	{
		if(hash_map_insert(hm, "test", (int){ 123 + i }))
			printf("key 'test' already exists..\n");
		else
			printf("added 'key' with value %d\n", i);
	}

	int *i = hash_map_find(hm, "test");
	if(i)
		printf("found value %d\n", *i);

	hash_map_destroy(&hm);
}

int main(void)
{
	example_heap_allocated_string();
	example_int();
}
```
