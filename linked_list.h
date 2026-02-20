/* Copyright 2023 Munteanu Eugen 315CA */
#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "utils.h"

/*
Source: https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-02
*/

/* implementation of a node from a singly linked list */
typedef struct node_t {
	void *data;
	struct node_t *next;
} node_t;

/* implementation of a singly linked list */
typedef struct list_t {
	node_t *head;
	unsigned int data_size;
	unsigned size;
} list_t;

/* Some functions were taken from the lab support */
list_t *create_list(unsigned int data_size);
list_t *ll_create(unsigned int data_size);

void ll_add_nth_node(list_t* list, unsigned int n, const void* new_data);
node_t *ll_remove_nth_node(list_t* list, unsigned int n);
unsigned int ll_get_size(list_t* list);

void ll_free(list_t** pp_list);

void ll_print_int(list_t* list);
void ll_print_string(list_t* list);

#endif  // LINKED_LIST_H_
