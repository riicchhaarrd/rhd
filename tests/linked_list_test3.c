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