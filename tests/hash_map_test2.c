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