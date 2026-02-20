/* Copyright 2023 Munteanu Eugen 315CA */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "utils.h"
#include "linked_list.h"

/* HMAX is the maximum number of buckets in the hashtable. */
#define HMAX 100

/*
Source: https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-04
*/

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	list_t **buckets; /* Array of singly-linked lists. */
	/* Total number of nodes currently existing in all buckets. */
	unsigned int size;
	unsigned int hmax; /* Number of buckets. */
	/* (Pointer to) Function to calculate the hash value associated with keys. */
	unsigned int (*hash_function)(void*);
	/* (Pointer to) Function to compare two keys. */
	int (*compare_function)(void*, void*);
	/* (Pointer to) Function to free the memory occupied by key and value. */
	void (*key_val_free_function)(void*);
};

/* Some functions were taken from the lab support */
int compare_function_ints(void *a, void *b);
int compare_function_strings(void *a, void *b);

unsigned int hash_function_int(void *a);
unsigned int hash_function_string(void *a);

void key_val_free_function(void *data);
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
					   int (*compare_function)(void*, void*),
					   void (*key_val_free_function)(void*));

int ht_has_key(hashtable_t *ht, void *key);
void *ht_get(hashtable_t *ht, void *key);
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
			void *value, unsigned int value_size);

void ht_remove_entry(hashtable_t *ht, void *key);
void ht_free(hashtable_t *ht);
unsigned int ht_get_size(hashtable_t *ht);
unsigned int ht_get_hmax(hashtable_t *ht);

#endif  // HASHTABLE_H_
