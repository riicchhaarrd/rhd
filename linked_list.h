#ifndef LIST_H
#define LIST_H
#include <assert.h>
#include <string.h>
#include "memory.h"
/*

TODO:
for binary search, a mid point, requires more calculations though when inserting and removing..

*/

struct linked_list_node
{
	struct linked_list_node *next;
	struct linked_list_node *prev;
	size_t data_size;
	unsigned char data[];
};

typedef void (*linked_list_node_finalizer_callback_t)(void*);

struct linked_list
{
	struct linked_list_node *head;
	struct linked_list_node *tail;
	size_t data_size;
	linked_list_node_finalizer_callback_t on_node_delete_fn;
	void *custom_allocator_userptr;
	custom_allocator_fn_t custom_allocator_fn;
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
#define linked_list_foreach_node(list, var_name, body) \
	do { \
	struct linked_list_node *var_name = (list) ? (list)->head : NULL; \
	while(var_name != NULL) \
	{ \
		body \
		var_name = var_name->next; \
	} \
	} while(0)
#define linked_list_node_value(x) ((void*)(x)->data)
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
	
#define linked_list_create_with_custom_allocator(type, userptr, allocator_fn) \
	linked_list_create_with_data_size_and_custom_allocator(sizeof(type), userptr, allocator_fn)
	
#define linked_list_append(list, value) \
	linked_list_append_((list), (unsigned char*)&(value), sizeof(value))
	
#define linked_list_prepend(list, value) \
	linked_list_prepend_((list), (unsigned char*)&(value), sizeof(value))
	
#define linked_list_init(list, type) \
	linked_list_init_with_data_size((list), sizeof(type))
	
#ifndef LINKED_LIST_IMPL
extern struct linked_list* linked_list_create_with_data_size(size_t data_size);
extern struct linked_list* linked_list_create_with_data_size_and_custom_allocator(size_t data_size, void *userptr, custom_allocator_fn_t allocator_fn);
extern void linked_list_destroy(struct linked_list **list);
extern void linked_list_free_with_deleter(struct linked_list *list, linked_list_node_finalizer_callback_t fn);
extern void* linked_list_append_(struct linked_list *list, unsigned char *data, size_t data_size);
extern void* linked_list_prepend_(struct linked_list *list, unsigned char *data, size_t data_size);
extern void linked_list_init_with_data_size(struct linked_list *, size_t);

extern void linked_list_set_node_value_finalizer(struct linked_list*, linked_list_node_finalizer_callback_t);
extern int linked_list_erase_node(struct linked_list *list, struct linked_list_node *node);
#else

void linked_list_set_node_value_finalizer(struct linked_list *list, linked_list_node_finalizer_callback_t fn)
{
	list->on_node_delete_fn = fn;
}

void linked_list_init_with_data_size(struct linked_list *list, size_t data_size)
{
	list->head = NULL;
	list->tail = NULL;
	list->data_size = data_size;
	list->on_node_delete_fn = NULL;
	list->custom_allocator_userptr = NULL;
	list->custom_allocator_fn = NULL;
}

struct linked_list *linked_list_create_with_data_size(size_t data_size)
{
	struct linked_list *list = memory_allocate(sizeof(struct linked_list));
	linked_list_init_with_data_size(list, data_size);
	return list;
}

struct linked_list *linked_list_create_with_data_size_and_custom_allocator(size_t data_size, void *userptr, custom_allocator_fn_t allocator_fn)
{
	struct linked_list *list = allocator_fn(userptr, sizeof(struct linked_list));
	linked_list_init_with_data_size(list, data_size);
	list->custom_allocator_userptr = userptr;
	list->custom_allocator_fn = allocator_fn;
	return list;
}

struct linked_list_node *linked_list_create_node_(struct linked_list *list, unsigned char *data, size_t data_size)
{
	struct linked_list_node *n = NULL;
	if(list->custom_allocator_fn && list->custom_allocator_userptr)
		n = list->custom_allocator_fn(list->custom_allocator_userptr, sizeof(struct linked_list_node) + data_size);
	else
		n = memory_allocate(sizeof(struct linked_list_node) + data_size);
	n->next = NULL;
	n->prev = NULL;
	n->data_size = data_size;
	memcpy(n->data, data, data_size);
	return n;
}

int linked_list_erase_node(struct linked_list *list, struct linked_list_node *node)
{
	if(!list)
		return 1;
	if(!node)
		return 1;
	if(list->head == node)
	{
		list->head = node->next;
		if(list->head)
			list->head->prev = NULL; //no previous node anymore.. we're head
	}
	if(list->tail == node)
	{
		list->tail = node->prev;
		if(list->tail)
			list->tail->next = NULL; //no next node anymore.. we're (also) tail
	}
	if(node->next)
		node->next->prev = node->prev;
	if(node->prev)
		node->prev->next = node->next;

	if(list->on_node_delete_fn)
		list->on_node_delete_fn(node->data);
	memory_deallocate(node);
	return 0;
}

void* linked_list_prepend_(struct linked_list *list, unsigned char *data, size_t data_size)
{	
	assert(list->data_size == data_size);
	
	if(list->head == NULL)
	{
		list->tail = list->head = linked_list_create_node_(list, data, data_size);
		return list->head->data;
	}
	
	struct linked_list_node *new_node = linked_list_create_node_(list, data, data_size);
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
		list->tail = list->head = linked_list_create_node_(list, data, data_size);
		return list->head->data;
	}
	struct linked_list_node *cur = list->head;
	for(;;)
	{
		if(cur->next == NULL)
			break;
		cur = cur->next;
	}
	struct linked_list_node *new_node = linked_list_create_node_(list, data, data_size);
	new_node->prev = cur;
	cur->next = new_node;
	list->tail = new_node;
	return cur->next->data;
}

void linked_list_free_with_deleter(struct linked_list *list, linked_list_node_finalizer_callback_t fn)
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