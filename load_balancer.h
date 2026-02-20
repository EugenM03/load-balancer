/* Copyright 2023 Munteanu Eugen 315CA */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"

#define MAX_SERVERS 100000
#define REPLICAS 3

struct load_balancer;
typedef struct load_balancer load_balancer;
struct load_balancer {
	int no_servers;  /* current number of servers */
	server_memory **servers;

	/*
	 * We use an imaginary circle hashring;
	 * we will have a sorted circular vector.
	 * Each server will have 3 points on this circle
	 * (3 replicas for each server in the system).
	 */
	int no_hashring_points;
	int max_no_hashring_points;
	int *hashring;
};

/**
 * init_load_balancer() - initializes the memory for a new load balancer and
 *                        its fields and returns a pointer to it.
 *
 * Return: pointer to the load balancer struct.
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring).
 *
 * @arg1: Load balancer to free.
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should possess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/*
 * Function that uniformly distributes elements and servers on the hashring of
 * the system, in clockwise order.
 *
 * @arg1: Load Balancer for uniform distribution of servers.
 * @arg2: position of the new server to be added.
 * @arg3: a replica of the newly added server.
 */
void balance_load_balancer(load_balancer *main, int index, unsigned int label);

/**
 * Function that inserts a server into a hash ring at a given position.
 *
 * @arg1: Load Balancer for uniform distribution of servers.
 * @arg2: The position at which the new server will be added.
 * @arg3: A replica of the server to be added.
 */
void insert_at_position_in_hashring(load_balancer *main, int index,
									unsigned int label);

/**
 * Inserts a server into a simulated hash ring ("imaginary circle").
 *
 * We will use two other helper functions to uniformly distribute the servers
 * in the load balancer after adding the new one at the found 'index' position.
 *
 * @arg1: Load Balancer for uniform distribution of servers.
 * @arg2: A replica of the server to be added to the hash ring.
 * @arg3: The hash of the server replica to be added to the hash ring.
 */

void add_to_hashring(load_balancer *main, unsigned int label,
						unsigned int hash_label);

/**
 * loader_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some of the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * Function that removes a server from a given position
 * in the simulated hash ring.
 *
 * @arg1: Load Balancer for uniform distribution of servers.
 * @arg2: The position of the server to be removed from the system.
 */
void erase_at_position_in_hashring(load_balancer *main, int index);

/*
 * Function that removes a server from a hashring.
 *
 * @arg1: Load Balancer for uniform distribution of servers.
 * @arg2: Hash of the server replica that will be removed from the system.
 */
void delete_from_hashring(load_balancer *main, unsigned int hash_label);

/**
 * loader_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

#endif  // LOAD_BALANCER_H_
