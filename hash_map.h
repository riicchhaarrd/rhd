#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "hash_string.h"
/* we'll use a custom made linked list instead of the linked list.. */

struct hash_bucket_entry
{
	struct hash_bucket_entry *next;
	unsigned long hash;
	char *key;
	unsigned char data[];
};

struct hash_bucket
{
	struct hash_bucket_entry *head;
	size_t size;
};

#define HASH_BUCKET_SIZE (16)
#define HASH_LOAD_FACTOR (0.75)

struct hash_map
{
	struct hash_bucket *buckets;
	size_t bucket_size;
	size_t data_size;
	size_t num_entries;
	int distinct;
};

struct hash_bucket *hash_allocate_buckets(size_t num_buckets)
{
	struct hash_bucket *buckets = malloc(sizeof(struct hash_bucket) * num_buckets);
	for(size_t i = 0; i < num_buckets; ++i)
	{
		buckets[i].head = NULL; //all empty lists..
		buckets[i].size = 0;
	}
	return buckets;
}

struct hash_map *hash_map_create(size_t data_size)
{
	struct hash_map *ht = malloc(sizeof(struct hash_map));
	ht->num_entries = 0;
	ht->buckets = hash_allocate_buckets(HASH_BUCKET_SIZE);
	ht->bucket_size = HASH_BUCKET_SIZE;
	ht->data_size = data_size;
	ht->distinct = 1;
	return ht;
}

void hash_bucket_free(struct hash_bucket *bucket)
{
	if(bucket->head == NULL)
		return;
	
	struct hash_bucket_entry *cur = bucket->head;
	
	while(cur != NULL)
	{
		struct hash_bucket_entry *tmp = cur;
		cur = cur->next;
		free(tmp->key);
		free(tmp);
	}
	bucket->head = NULL;
}

void hash_map_free(struct hash_map **hmp)
{
	struct hash_map *hm = *hmp;
	
	for(size_t i = 0; i < hm->bucket_size; ++i)
	{
		struct hash_bucket *bucket = &hm->buckets[i];
		if(bucket->head == NULL) //empty bucket skip
			continue;
		hash_bucket_free(bucket);
	}
	free(hm->buckets);
	
	free(hm);
	*hmp = NULL;
}

struct hash_bucket_entry *hash_bucket_find(struct hash_bucket *bucket, const char *key, unsigned long hashed_key)
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

struct hash_bucket_entry *hash_map_find(struct hash_map *ht, const char *key)
{
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket *bucket = &ht->buckets[hashed_key % ht->bucket_size];
	return hash_bucket_find(bucket, key, hashed_key);
}

void hash_map_dump(struct hash_map *hm)
{
	for(size_t i = 0; i < hm->bucket_size; ++i)
	{
		struct hash_bucket *bucket = &hm->buckets[i];
		printf("bucket %d: %d entries\n", i, bucket->size);
	}
}

struct hash_bucket_entry *hash_bucket_entry_create(const char *key, unsigned char *data, size_t data_size)
{
	unsigned long hashed_key = hash_string(key);
	struct hash_bucket_entry *entry = malloc(sizeof(struct hash_bucket_entry) + data_size);
	entry->hash = hashed_key;
	entry->key = malloc(strlen(key) + 1); //DON'T FORGET TO FREE THIS KEY
	strcpy(entry->key, key);
	entry->next = NULL;
	memcpy(entry->data, data, data_size);
	return entry;
}

void hash_bucket_insert(struct hash_bucket *bucket, const char *key, unsigned char *data, size_t data_size)
{
	struct hash_bucket_entry *entry = hash_bucket_entry_create(key, data, data_size);
	
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

void hash_map_rehash(struct hash_map *hm)
{
	size_t new_bucket_size = hm->bucket_size * 2;
	struct hash_bucket *new_buckets = hash_allocate_buckets(new_bucket_size);
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
			hash_bucket_insert(new_bucket, cur->key, cur->data, hm->data_size);
			cur = cur->next;
		}
		//free old bucket
		hash_bucket_free(bucket);
	}
	free(hm->buckets);
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
	
	hash_bucket_insert(bucket, key, data, data_size);
	
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

#define hash_map_new(type) \
	hash_map_create(sizeof(type))

#define hash_map_insert(ht, key, value) \
	hash_map_insert_data(ht, key, (unsigned char*)&(value), sizeof(value))

#endif
