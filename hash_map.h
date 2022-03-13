#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "std.h"

#include <string.h>
#include <assert.h>
#include "memory.h"
#include "hash_string.h"
/* we'll use a custom made linked list instead of the linked list.. */

#define HM_STATIC static
#define HM_INLINE inline

//msvc warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning( push )
#pragma warning( disable : 4200 )
struct hash_bucket_entry
{
	struct hash_bucket_entry *next;
	unsigned long hash;
	char *key;
	unsigned char data[];
};
#pragma warning( pop )

struct hash_bucket
{
	struct hash_bucket_entry *head;
	size_t size;
};

#define HASH_BUCKET_SIZE (16)
#define HASH_LOAD_FACTOR (1)

struct hash_map
{
	struct hash_bucket *buckets;
	size_t bucket_size;
	size_t data_size;
	size_t num_entries;
	
	deallocator_t on_key_removal_fn;
	int distinct;
	void *custom_allocator_userptr;
	custom_allocator_fn_t custom_allocator_fn;
};

#ifndef HASH_MAP_IMPL
//public API
extern struct hash_map *hash_map_create_data(size_t data_size, void *custom_allocator_userptr, custom_allocator_fn_t custom_allocator_fn);
extern void hash_map_destroy(struct hash_map **hmp);
extern void *hash_map_find(struct hash_map *ht, const char *key);
extern int hash_map_insert_data(struct hash_map *ht, const char *key, unsigned char *data, size_t data_size);
extern void hash_map_dump(struct hash_map *hm);
extern void hash_map_set_on_key_removal(struct hash_map *hm, deallocator_t fn);
extern int hash_map_remove_key(struct hash_map **hmp, const char *key);

#define hash_map_foreach_entry(hm, entry, body) \
	do { \
		for(size_t i = 0; i < hm->bucket_size; ++i) \
		{ \
			struct hash_bucket *bucket = &hm->buckets[i]; \
			if(bucket->head == NULL) continue; \
			struct hash_bucket_entry *entry = bucket->head; \
			while(entry) \
			{ \
				body \
				entry = entry->next; \
			} \
		} \
	} while(0)
#else

void hash_map_set_on_key_removal(struct hash_map *hm, deallocator_t fn)
{
	hm->on_key_removal_fn = fn;
}

HM_STATIC struct hash_bucket *hash_allocate_buckets(struct hash_map *hm, size_t num_buckets)
{
	struct hash_bucket *buckets = NULL;
	
	if(hm->custom_allocator_fn && hm->custom_allocator_userptr)
		buckets = hm->custom_allocator_fn(hm->custom_allocator_userptr, sizeof(struct hash_bucket) * num_buckets);
	else
		buckets = memory_allocate(sizeof(struct hash_bucket) * num_buckets);
	
	for(size_t i = 0; i < num_buckets; ++i)
	{
		buckets[i].head = NULL; //all empty lists..
		buckets[i].size = 0;
	}
	return buckets;
}

struct hash_map *hash_map_create_data(size_t data_size, void *custom_allocator_userptr, custom_allocator_fn_t custom_allocator_fn)
{
	struct hash_map *ht = NULL;
	if(custom_allocator_fn && custom_allocator_userptr)
		ht = custom_allocator_fn(custom_allocator_userptr, sizeof(struct hash_map));
	else
		ht = memory_allocate(sizeof(struct hash_map));
	ht->custom_allocator_fn = custom_allocator_fn;
	ht->custom_allocator_userptr = custom_allocator_userptr;
	ht->num_entries = 0;
	ht->buckets = hash_allocate_buckets(ht, HASH_BUCKET_SIZE);
	ht->bucket_size = HASH_BUCKET_SIZE;
	ht->data_size = data_size;
	ht->distinct = 1;
	ht->on_key_removal_fn = NULL;
	return ht;
}

HM_STATIC void hash_bucket_free(struct hash_map *hm, struct hash_bucket *bucket)
{
	if(bucket->head == NULL)
		return;
	
	struct hash_bucket_entry *cur = bucket->head;
	
	while(cur != NULL)
	{
		struct hash_bucket_entry *tmp = cur;
		cur = cur->next;
		if(hm->on_key_removal_fn)
			hm->on_key_removal_fn(tmp->data);
		memory_deallocate(tmp->key);
		memory_deallocate(tmp);
	}
	bucket->head = NULL;
}

void hash_map_destroy(struct hash_map **hmp)
{
	struct hash_map *hm = *hmp;
	
	for(size_t i = 0; i < hm->bucket_size; ++i)
	{
		struct hash_bucket *bucket = &hm->buckets[i];
		if(bucket->head == NULL) //empty bucket skip
			continue;
		hash_bucket_free(hm, bucket);
	}
	memory_deallocate(hm->buckets);
	
	memory_deallocate(hm);
	*hmp = NULL;
}

HM_STATIC struct hash_bucket_entry *hash_bucket_find(struct hash_bucket *bucket, const char *key, unsigned long hashed_key)
{
	if(bucket->head == NULL)
		return NULL; //empty bucket
	
	struct hash_bucket_entry *cur = bucket->head;
	
	for(;;)
	{
		if(cur->hash == hashed_key && !strcmp(cur->key, key))
			return cur;
		if(cur->next == NULL)
			break;
		cur = cur->next;
	}
	return NULL;
}

void *hash_map_find(struct hash_map *ht, const char *key)
{
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket *bucket = &ht->buckets[hashed_key % ht->bucket_size];
	struct hash_bucket_entry *entry = hash_bucket_find(bucket, key, hashed_key);
	return entry ? entry->data : NULL;
}

int hash_map_remove_key(struct hash_map **hmp, const char *key)
{
	struct hash_map *ht = *hmp;
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket *bucket = &ht->buckets[hashed_key % ht->bucket_size];
	struct hash_bucket_entry *entry = hash_bucket_find(bucket, key, hashed_key);
    if(!entry)
        return 0;
    
    struct hash_bucket_entry **cur = &bucket->head;
    while((*cur) != entry)
        cur = &(*cur)->next;
    *cur = entry->next;

	if ( ht->on_key_removal_fn )
		ht->on_key_removal_fn( entry->data );
	memory_deallocate( entry->key );
	memory_deallocate( entry );

	--bucket->size;
    --ht->num_entries;
	return 1;
}

void hash_map_dump(struct hash_map *hm)
{
	#if 0
	for(size_t i = 0; i < hm->bucket_size; ++i)
	{
		struct hash_bucket *bucket = &hm->buckets[i];
		printf("bucket %lu: %lu entries\n", i, bucket->size);
	}
	#endif
}

HM_STATIC struct hash_bucket_entry *hash_bucket_entry_create(struct hash_map *hm, const char *key, unsigned char *data, size_t data_size)
{
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket_entry *entry = NULL;
	
	if(hm->custom_allocator_fn && hm->custom_allocator_userptr)
		entry = hm->custom_allocator_fn(hm->custom_allocator_userptr, sizeof(struct hash_bucket_entry) + data_size);
	else
		entry = memory_allocate(sizeof(struct hash_bucket_entry) + data_size);
	
	entry->hash = hashed_key;
	int kl = strlen(key);
	
	if(hm->custom_allocator_fn && hm->custom_allocator_userptr)
		entry->key = hm->custom_allocator_fn(hm->custom_allocator_userptr, kl + 1);
	else
		entry->key = memory_allocate(kl + 1); //DON'T FORGET TO FREE THIS KEY
	
	std_strncpy_s(entry->key, kl + 1, key, kl);
	entry->next = NULL;
	memcpy(entry->data, data, data_size);
	return entry;
}

HM_STATIC void hash_bucket_insert(struct hash_map *hm, struct hash_bucket *bucket, const char *key, unsigned char *data, size_t data_size)
{
	struct hash_bucket_entry *entry = hash_bucket_entry_create(hm, key, data, data_size);
	
	++bucket->size;
	
	if(bucket->head == NULL) //empty bucket
	{
		bucket->head = entry;
	} else
	{
		//prepend
		struct hash_bucket_entry *tmp = bucket->head;
		bucket->head = entry;
		bucket->head->next = tmp;
	}
}

HM_STATIC void hash_map_rehash(struct hash_map *hm)
{
	size_t new_bucket_size = hm->bucket_size * 2;
	struct hash_bucket *new_buckets = hash_allocate_buckets(hm, new_bucket_size);
	//fill the new buckets with old data
	for(size_t i = 0; i < hm->bucket_size; ++i)
	{
		struct hash_bucket *bucket = &hm->buckets[i];
		if(bucket->head == NULL)
			continue; //skip.. empty bucket
		struct hash_bucket_entry *cur = bucket->head;
		while(cur != NULL)
		{
			struct hash_bucket *new_bucket = &new_buckets[cur->hash % new_bucket_size];
			//hash map num entries stays the same, we're just rehashing, not adding new entries
			hash_bucket_insert(hm, new_bucket, cur->key, cur->data, hm->data_size);
			cur = cur->next;
		}
		//free old bucket
		hash_bucket_free(hm, bucket);
	}
	memory_deallocate(hm->buckets);
	hm->bucket_size = new_bucket_size;
	hm->buckets = new_buckets;
}

int hash_map_insert_data(struct hash_map *ht, const char *key, unsigned char *data, size_t data_size)
{
	assert(data_size == ht->data_size);
	
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket *bucket = &ht->buckets[hashed_key % ht->bucket_size];
	
	//unique keys
	if(ht->distinct && hash_bucket_find(bucket, key, hashed_key) != NULL)
		return 1;
	
	++ht->num_entries;
	
	hash_bucket_insert(ht, bucket, key, data, data_size);
	
	//probably should rehash before insertion
	if(ht->num_entries >= HASH_LOAD_FACTOR * ht->bucket_size)
	{
		//rehash
		//size_t old_size = ht->bucket_size;
		hash_map_rehash(ht);
		//printf("rehashing.. (%d entries) %d to %d\n", ht->num_entries, old_size, ht->bucket_size);
	}
	return 0;
}

#endif

#define hash_map_create(type) \
	hash_map_create_data(sizeof(type), NULL, NULL)
#define hash_map_create_with_custom_allocator(type, userptr, allocator_fn) \
	hash_map_create_data(sizeof(type), userptr, allocator_fn)

#define hash_map_insert(ht, key, value) \
	hash_map_insert_data(ht, key, (unsigned char*)&(value), sizeof(value))
#endif