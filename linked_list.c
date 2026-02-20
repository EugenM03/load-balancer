/* Copyright 2023 Munteanu Eugen 315CA */
#include "linked_list.h"

/*
Source: https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-02
*/

list_t* ll_create(unsigned int data_size)
{
	list_t* list;
	list = calloc(1, sizeof(list_t));
	DIE(!list, "calloc() for *list failed\n");

	list->data_size = data_size;

	return list;
}


/*
 * Function that returns the n-th node of the list (indexing from 0).
 */
node_t* get_nth_node(list_t* list, unsigned int n)
{
	if (!list || !(list->head))
		return NULL;

	node_t* n_node;
	n_node = list->head;

	for (unsigned  int i = 0; i < n; ++i)
		n_node = n_node->next;

	return n_node;
}

/*
 * Based on the data sent through the new_data pointer, a new node is created
 * and added at position n of the list represented by the list pointer. Positions
 * in the list are indexed starting from 0 (i.e. the first node in the list is at
 * position n=0). If n >= number of nodes, the new node is added at the end of
 * the list.
 */
void ll_add_nth_node(list_t* list, unsigned int n, const void* new_data)
{
	if (!list || !new_data)
		return;

	if (n > list->size)
		n = list->size;

	// add node on n-1 position (indexing from 0)
	node_t* n_node;
	n_node = calloc(1, sizeof(node_t));
	DIE(!n_node, "calloc() for *n_node failed\n");
	n_node->data = calloc(1, list->data_size);
	DIE(n_node->data == NULL, "calloc() for n_node->data failed\n");

	memcpy(n_node->data, new_data, list->data_size);

	n_node->next = NULL;
	if (n == 0) {
		n_node->next = list->head;
		list->head = n_node;
	} else {
		node_t* prev;
		prev = get_nth_node(list, n - 1);
		if (!prev)
			return;
		n_node->next = prev->next;
		prev->next = n_node;
	}

	if (n == list->size)
		n_node->next = NULL;
	list->size++;
}

/*
 * Removes the node at position n from the list whose pointer is passed as
 * a parameter. Positions in the list are indexed from 0 (i.e. the first node
 * in the list is at position n=0). If n >= nr_nodes - 1, the node at the end
 * of the list is removed.
 *
 * The function returns a pointer to this newly removed node from the list.
 * It is the caller's responsibility to free the memory of this node.
 */
node_t* ll_remove_nth_node(list_t* list, unsigned int n)
{
	if (!list || !(list->head))
		return NULL;

	if (n >= list->size)
		n = list->size - 1;

	node_t *prev, *curr;
	if (n == 0) {
		curr = list->head;
		list->head = curr->next;
	} else {
		prev = get_nth_node(list , n - 1);
		curr = prev->next;
		prev->next = curr->next;
	}

	list->size--;
	return curr;
}

/*
 * Function that frees the memory used by all nodes in the list, and at
 * the end, frees the memory used by the list structure and updates
 * the value of the pointer to NULL, the one that the argument points to
 * (the argument is a pointer to a pointer).
 */
void ll_free(list_t** pp_list)
{
	if (!pp_list || !(*pp_list))
		return;

	node_t* curr;
	curr = (*pp_list)->head;
	node_t* next;

	while (curr != NULL) {
		next = curr->next;

		free(curr->data);
		curr->data = NULL;
		free(curr);
		curr = next;
	}
	free(*pp_list);
	*pp_list = NULL;
}
