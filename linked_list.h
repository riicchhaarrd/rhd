#ifndef LIST_H
#define LIST_H
#include <assert.h>
#include <string.h>
#include "memory.h"
/*

TODO:
doubly linked list, just add a prev field
end field so we know the end of the linked list
for binary search, a mid point, requires more calculations though when inserting and removing..

*/

struct linked_list_node
{
	struct linked_list_node *next;
	struct linked_list_node *prev;
	size_t data_size;
	unsigned char data[];
};

struct linked_list
{
	struct linked_list_node *head;
	struct linked_list_node *tail;
	size_t data_size;
	deallocator_t on_node_delete_fn;
};

#define linked_list_foreach(list, type, var_name, body) \
	do { \
	struct linked_list_node *cur = (list) ? (list)->head : NULL; \
	while(cur != NULL) \
	{ \
		type var_name = (type)cur->data; \
		cur = cur->next; \
		body \
	} \
	} while(0)
#define linked_list_reversed_foreach(list, type, var_name, body) \
	do { \
	struct linked_list_node *cur = (list) ? (list)->tail : NULL; \
	while(cur != NULL) \
	{ \
		type var_name = (type)cur->data; \
		cur = cur->prev; \
		body \
	} \
	} while(0)

#define linked_list_create(type) \
	linked_list_create_with_data_size(sizeof(type))
	
#define linked_list_append(list, value) \
	linked_list_append_((list), (unsigned char*)&(value), sizeof(value))
	
#define linked_list_prepend(list, value) \
	linked_list_prepend_((list), (unsigned char*)&(value), sizeof(value))
	
#define linked_list_init(list, type) \
	linked_list_init_with_data_size((list), sizeof(type))
	
#ifndef LINKED_LIST_IMPL
extern struct linked_list* linked_list_create_with_data_size(size_t data_size);
extern void linked_list_destroy(struct linked_list **list);
extern void linked_list_free_with_deleter(struct linked_list *list, deallocator_t fn);
extern void* linked_list_append_(struct linked_list *list, unsigned char *data, size_t data_size);
extern void* linked_list_prepend_(struct linked_list *list, unsigned char *data, size_t data_size);
extern void linked_list_init_with_data_size(struct linked_list *, size_t);
extern void linked_list_set_node_value_finalizer(struct linked_list*, deallocator_t);
#else

void linked_list_set_node_value_finalizer(struct linked_list *list, deallocator_t fn)
{
	list->on_node_delete_fn = fn;
}

void linked_list_init_with_data_size(struct linked_list *list, size_t data_size)
{
	list->head = NULL;
	list->tail = NULL;
	list->data_size = data_size;
	list->on_node_delete_fn = NULL;
}

struct linked_list *linked_list_create_with_data_size(size_t data_size)
{
	struct linked_list *list = memory_allocate(sizeof(struct linked_list));
	linked_list_init_with_data_size(list, data_size);
	return list;
}

struct linked_list_node *linked_list_create_node_(unsigned char *data, size_t data_size)
{
	struct linked_list_node *list = memory_allocate(sizeof(struct linked_list_node) + data_size);
	list->next = NULL;
	list->prev = NULL;
	list->data_size = data_size;
	memcpy(list->data, data, data_size);
	return list;
}

void* linked_list_prepend_(struct linked_list *list, unsigned char *data, size_t data_size)
{	
	assert(list->data_size == data_size);
	
	if(list->head == NULL)
	{
		list->tail = list->head = linked_list_create_node_(data, data_size);
		return list->head->data;
	}
	
	struct linked_list_node *new_node = linked_list_create_node_(data, data_size);
	struct linked_list_node *current_node = list->head;
	
	//make the new_node point to the current_node
	new_node->next = current_node;
	current_node->prev = new_node;
	
	//replace the list with our new new_node, which will become the head
	list->head = new_node;
	return new_node->data;
}

void* linked_list_append_(struct linked_list *list, unsigned char *data, size_t data_size)
{
	assert(list->data_size == data_size);
	
	if(list->head == NULL)
	{
		list->tail = list->head = linked_list_create_node_(data, data_size);
		return list->head->data;
	}
	struct linked_list_node *cur = list->head;
	for(;;)
	{
		if(cur->next == NULL)
			break;
		cur = cur->next;
	}
	struct linked_list_node *new_node = linked_list_create_node_(data, data_size);
	new_node->prev = cur;
	cur->next = new_node;
	list->tail = new_node;
	return cur->next->data;
}

void linked_list_free_with_deleter(struct linked_list *list, deallocator_t fn)
{
	struct linked_list_node *cur = list->head;
	while(cur != NULL)
	{
		struct linked_list_node *tmp = cur;
		cur = cur->next;
		if(fn)
			fn(tmp->data);
		memory_deallocate(tmp);
	}
	list->head = NULL;
}

void linked_list_destroy(struct linked_list **plist)
{
	assert(plist);
	if(!*plist)
		return;
	struct linked_list *list = (*plist);
	
	if(list->on_node_delete_fn)
		linked_list_free_with_deleter(list, list->on_node_delete_fn);
	else
		linked_list_free_with_deleter(list, NULL);
	memory_deallocate(list);
	*plist = NULL;
}
#endif
#endif