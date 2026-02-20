/* Copyright 2023 Munteanu Eugen 315CA */
#include "utils.h"
#include "hashtable.h"

#define ERROR_CODE -1

/*
Source: https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-04
*/

/*
 * Comparison functions for keys:
 */
int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Hashing functions
 */
unsigned int hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

/*
 * Function used to free the memory allocated for the key and
 * value of a pair in the hashtable.
 */
void key_val_free_function(void *data) {
	if (!data)
		return;

	info* pair = (info *) data;
	if (!pair)
		return;

	free(pair->key);
	pair->key = NULL;
	free(pair->value);
	pair->value = NULL;

	free(data);
	data = NULL;
}

/*
 * Function used to initialize a hashtable after its allocation.
 * The linked lists must also be allocated and initialized.
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	if (!hash_function || !compare_function)
		return NULL;

	// create hashtable and assign initial values for
	// the data types in the hashtable structure
	hashtable_t* ht = calloc(1, sizeof(*ht));
	DIE(ht == NULL, "calloc() for *ht failed\n");

	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;

	// allocate empty buckets, then for each pointer in
	// the array create an empty linked list
	ht->buckets = calloc(hmax, sizeof(*(ht->buckets)));
	DIE(ht->buckets == NULL, "calloc() for ht->buckets failed\n");

	for (unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(info));
		DIE(ht->buckets[i] == NULL, "calloc() for ht->buckets[i] failed\n");
	}

	return ht;
}

/*
 * Function that returns:
 * 1, if for the key key a value was previously associated in the hashtable
 * using the put function;
 * 0, otherwise.
 */
int ht_has_key(hashtable_t *ht, void *key)
{
	if (!ht || !key) {
		return ERROR_CODE;
	}

	unsigned int index = ht->hash_function(key) % (ht->hmax);
	list_t* bucket = ht->buckets[index];
	node_t* curr_node = bucket->head;

	while (curr_node != NULL) {
		info* curr_data = (info*) curr_node->data;
		if (!curr_data)
			return ERROR_CODE;

		if (ht->compare_function(curr_data->key, key) == 0)
			return 1;

		curr_node = curr_node->next;
	}

	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1)
		return NULL;

	// find bucket index where the key should be found
	unsigned int index = ht->hash_function(key) % (ht->hmax);
	list_t *bucket = ht->buckets[index];
	node_t *curr_node = bucket->head;

	// iterate through the bucket until the searched key is found or
	// until the end of the linked list
	while (curr_node != NULL) {
		info* curr_data = (info*) curr_node->data;
		if (!curr_data)
			return NULL;

		if (ht->compare_function(curr_data->key, key) == 0)
			return curr_data->value;

		curr_node = curr_node->next;
	}

	return NULL;
}

/*
 * Attention! Although the key is passed as a void pointer (since its type is
 * not enforced), when creating a new entry in the hashtable (in case the key
 * is not already found in ht), a copy of the value pointed to by key must be
 * created and the address of this copy must be saved in the info structure
 * associated with the entry in ht. To know how many bytes to allocate and copy,
 * use the key_size parameter.
 *
 * Motivation:
 * We need to copy the value pointed to by key because after a call to
 * put(ht, key_actual, value_actual), the value pointed to by key_actual
 * can be altered (e.g: *key_actual++). If we used the pointer address
 * key_actual directly, the key of an entry in the hashtable would effectively
 * be modified from outside the hashtable. We do not want this to happen,
 * because there is a risk of ending up in a situation where we no longer know
 * which key a certain value is registered under.
 */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	if (!ht || !key || !value)
		return;

	// calculate bucket index where the new value must be added/updated
	unsigned int index = ht->hash_function(key) % (ht->hmax);
	list_t *bucket = ht->buckets[index];
	node_t *curr_node = bucket->head;

	// check if there is already a key-value pair with the same key;
	// if there is, update the value associated with the pair
	if (ht_has_key(ht, key) == 1) {
		while (curr_node != NULL) {
			info* curr_data = (info*) curr_node->data;

			if (ht->compare_function(curr_data->key, key) == 0) {
				// free old value and allocate space for the new value
				free(curr_data->value);
				curr_data->value = NULL;

				curr_data->value = calloc(1, value_size);
				DIE(curr_data->value == NULL,
					"calloc() for curr_data->value failed\n");

				memcpy(curr_data->value, value, value_size);
			}

			curr_node = curr_node->next;  // next node in the bucket
		}
	} else {
		// if no such pair exists, create one and put it into
		// the current linked list
		info* curr_data = calloc(1, sizeof(info));
		DIE(curr_data == NULL, "calloc() for *curr_data failed\n");

		curr_data->key = calloc(1, key_size);
		DIE(curr_data->key == NULL,
			"calloc() for curr_data->key failed\n");
		curr_data->value = calloc(1, value_size);
		DIE(curr_data->value == NULL,
			"calloc() for curr_data->value failed\n");

		// copy new key and value into the new node
		memcpy(curr_data->key, key, key_size);
		memcpy(curr_data->value, value, value_size);

		// add new node on the first position of the list
		ll_add_nth_node(ht->buckets[index], 0, curr_data);
		ht->size++;

		free(curr_data);
		curr_data = NULL;
	}
}


/*
 * Procedure that removes from the hash table the entry associated with the key.
 * Warning! Care must be taken to free all the memory used for an entry in the
 * hash table (that is, the memory for the copy of the key -- see the note at
 * the put procedure --, for the info structure and for the Node structure from
 * the linked list).
 */
void ht_remove_entry(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1)
		return;

	// calculate bucket index using the hask of the key
	unsigned int index = ht->hash_function(key) % (ht->hmax);
	list_t *bucket = ht->buckets[index];
	node_t *curr_node = bucket->head;

	// to keep track of the position of the node to be deleted
	unsigned int index_node = 0;

	while (curr_node != NULL) {
		info* curr_data = (info*) curr_node->data;
		if (!curr_data)
			return;

		// if the node to be deleted is found,
		// free the allocated memory for the node
		if (ht->compare_function(curr_data->key, key) == 0) {
			free(curr_data->key);
			curr_data->key = NULL;
			free(curr_data->value);
			curr_data->value = NULL;
			free(curr_data);
			curr_data = NULL;

			// delete the node itself from the list and free its memory
			node_t* del_node;
			del_node = ll_remove_nth_node(ht->buckets[index], index_node);
			free(del_node);
			del_node = NULL;

			ht->size--;
			return;
		}
		curr_node = curr_node->next;
		index_node++;
	}
}


/*
 * Function that frees the memory used by all the entries in the hashtable, and
 * then also frees the memory used to store the hashtable structure itself.
 */
void ht_free(hashtable_t *ht)
{
	if (!ht)
		return;

	// to free the memory carefully, iterate through each bucket
	// and for each bucket, free the memory for the nodes in the list,
	// then free the list itself, and at the end, free the array of buckets
	for (unsigned int i = 0; i < ht->hmax; i++) {
		node_t* curr_node = ht->buckets[i]->head;

		while (curr_node != NULL) {
			info* curr_data = (info*)curr_node->data;
			if (!curr_data)
				return;

			free(curr_data->key);
			curr_data->key = NULL;
			free(curr_data->value);
			curr_data->value = NULL;

			curr_node = curr_node->next;
		}
		ll_free(&ht->buckets[i]);
		ht->buckets[i] = NULL;
	}

	free(ht->buckets);
	ht->buckets = NULL;
	free(ht);
	ht = NULL;
}
