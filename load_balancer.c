/* Copyright 2023 Munteanu Eugen 315CA */
#include "load_balancer.h"
#include "hashtable.h"

unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer *init_load_balancer() {
	// allocate new load balancer
	load_balancer *new_load = calloc(1, sizeof(load_balancer));
	DIE(!new_load, "calloc() for *new_load failed\n");

	// allocate data types used
	new_load->servers = calloc(MAX_SERVERS, sizeof(server_memory *));
	DIE(!(new_load->servers), "calloc() for new_load->servers failed\n");

	new_load->hashring = calloc(REPLICAS, sizeof(int));
	DIE(!(new_load->hashring), "calloc() for new_load->hashring failed\n");

	new_load->no_servers = 0;
	new_load->no_hashring_points = 0;
	new_load->max_no_hashring_points = REPLICAS;

	return new_load;
}

void balance_load_balancer(load_balancer *main, int index, unsigned int label) {
	// used for calculating the position of the neighbor server
	unsigned int aux_index = 0;
	// used for storing the position of the neighbor server in the hashring
	unsigned int next_server_index = 0;

	// if we have at most one server in the system, we do nothing
	if (main->no_hashring_points <= 1) {
		return;
	} else {
		// find the position of the right neighbor server for the current index
		aux_index = (index + 1) % main->no_hashring_points;
		next_server_index = main->hashring[aux_index] % MAX_SERVERS;
	}

	// if the current server (identified by label) is the same as the
	// right neighbor server, we do nothing
	if (label % MAX_SERVERS == next_server_index)
		return;

	// else, redistribute the elements to the next server on the right
	for (int i = 0; i < HMAX; i++) {
		if (main->servers[next_server_index]->memory->buckets[i] != NULL) {
			// iterate the current bucket, element by element
			server_memory *curr_server = main->servers[next_server_index];
			node_t* curr_elem = curr_server->memory->buckets[i]->head;

			while (curr_elem != NULL) {
				// find the key-value pair for the current element
				info *info = curr_elem->data;
				char *key = (char *)info->key;
				char *value = (char *)info->value;

				// redistribute the content to the next server
				server_store(main->servers[label % MAX_SERVERS], key, value);

				curr_elem = curr_elem->next;
			}
		}
	}
}

void insert_at_position_in_hashring(load_balancer *main, int index,
									unsigned int label) {
	// insert the corresponding server, depending on the position

	// 0-index case (at the beginning of the hashring)
	if (index == 0) {
		main->no_hashring_points++;

		// iterate through hashring and shift servers to the right
		for (int k = main->no_hashring_points - 1; k > index; k--)
			main->hashring[k] = main->hashring[k - 1];

		// insert the server itself at the respective position
		main->hashring[index] = label;
	} else {
		// last-index case (at the end of the hashring)
		if (index == main->no_hashring_points) {
			main->no_hashring_points++;
			main->hashring[index] = label;

		// between-two-consecutive-servers-index case
		} else {
			main->no_hashring_points++;
			// iterate through hashring and shift servers to the right
			for (int k = main->no_hashring_points - 1; k > index - 1; k--)
				main->hashring[k] = main->hashring[k - 1];

			// insert the server itself at the respective position
			main->hashring[index] = label;
		}
	}
}

void add_to_hashring(load_balancer *main, unsigned int label,
						unsigned int hash_label) {
	int start = 0;
	int end = main->no_hashring_points - 1;
	int index = -1;

	// iterate through hashring (binary search);
	// search for index in hashring where the server will be added
	while (start <= end) {
		int mid = start + (end - start) / 2;  // avoid overflow
		int curr_server_value = main->hashring[mid];

		if (hash_label > hash_function_servers(&curr_server_value)) {
			start = mid + 1;
		} else {
			index = mid;
			end = mid - 1;
		}
	}

	// if we have no servers, we will add the server on the first position
	if (index == -1 && !(main->no_hashring_points))
		index = 0;

	// if we did not find the index but we have servers, add the server at
	// the end of the hashring (last index + 1)
	if (index == -1 && main->no_hashring_points)
		index = main->no_hashring_points;

	index = index % MAX_SERVERS;  // maximum 99999 servers

	// next, insert the server itself at the found position, then
	// redistribute the data in the system uniformly (balance_load_balancer())
	insert_at_position_in_hashring(main, index, label);
	balance_load_balancer(main, index, label);
}

void loader_add_server(load_balancer* main, int server_id) {
	// if the maximum number of servers in the system has been reached,
	// reallocate the memory for the servers array accordingly
	if ((main->no_hashring_points != REPLICAS * MAX_SERVERS) &&
		(main->no_hashring_points + REPLICAS > main->max_no_hashring_points)) {
		// double capacity
		main->max_no_hashring_points *= 2;

		// check server limit
		if (main->max_no_hashring_points > REPLICAS * MAX_SERVERS) {
			main->max_no_hashring_points = REPLICAS * MAX_SERVERS;
		}
		main->hashring = realloc(main->hashring,
								 main->max_no_hashring_points * sizeof(int));
	}

	// init new server
	server_memory *new_server = init_server_memory();

	// add server in servers array and update no. of servers
	main->servers[server_id % MAX_SERVERS] = new_server;
	main->no_servers++;

	// generate no. of replicas for a server, as well as the hash of each one
	unsigned int labels[REPLICAS] = {0};
	unsigned int hash_labels[REPLICAS] = {0};

	for (int i = 0; i < REPLICAS; i++) {
		labels[i] = MAX_SERVERS * i + server_id;
		hash_labels[i] = hash_function_servers(&labels[i]);
	}

	// next, for adding the server in the hashring, we will use several functions:
	//   add_to_hashring(), which will call at the end successively:
	//   insert_at_position_in_hashring(), and balance_load_balancer()
	for (int i = 0; i < REPLICAS; i++)
		add_to_hashring(main, labels[i], hash_labels[i]);
}

void erase_at_position_in_hashring(load_balancer *main, int index) {
	// delete the corresponding server, depending on the position
	// 0-index case (at the beginning of the hashring)
	if (index == 0) {
		// iterate through hashring and shift servers to the left
		for (int k = index; k < main->no_hashring_points - 1; k++)
			main->hashring[k] = main->hashring[k + 1];

		main->no_hashring_points--;

	// last-index case (at the end of the hashring)
	} else {
		if (index == main->no_hashring_points) {
			main->no_hashring_points--;

		// between-two-consecutive-servers-index case
		} else {
			// iterate through hashring and shift servers to the left
			for (int k = index; k < main->no_hashring_points - 1; k++)
				main->hashring[k] = main->hashring[k + 1];

			main->no_hashring_points--;
		}
	}
}

void delete_from_hashring(load_balancer *main, unsigned int hash_label) {
	int start = 0;
	int end = main->no_hashring_points - 1;
	int index = -1;

	// iterate through hashring (binary search)
	while (start <= end) {
		int mid = start + (end - start) / 2;  // avoid overflow
		int curr_server_value = main->hashring[mid];

		if (hash_label > hash_function_servers(&curr_server_value)) {
			start = mid + 1;
		} else {
			if (hash_label < hash_function_servers(&curr_server_value)) {
				index = mid;
				end = mid - 1;
			} else {
				index = mid;
				break;
			}
		}
	}

	// if we have no servers, we will delete the first server
	if (index == -1 && !(main->no_hashring_points))
		index = 0;

	// if we did not find the index but we have servers, delete the server at
	// the end of the hashring (last index + 1)
	if (index == -1 && main->no_hashring_points)
		index = main->no_hashring_points;

	index = index % MAX_SERVERS;  // maximum 99999 servers

	// next, delete the server from the system at the found position,
	// using a helper function
	erase_at_position_in_hashring(main, index);
}

void loader_remove_server(load_balancer* main, int server_id) {
	// generate 3 replicas for a server, as well as the hash of each one
	unsigned int labels[REPLICAS] = {0};
	unsigned int hash_labels[REPLICAS] = {0};

	for (int i = 0; i < REPLICAS; i++) {
		labels[i] = MAX_SERVERS * i + server_id;
		hash_labels[i] = hash_function_servers(&labels[i]);
	}

	// delete current server replicas from the hashring
	for (int i = 0; i < REPLICAS; i++)
		delete_from_hashring(main, hash_labels[i]);

	// redistribute the elements for the next server
	for (int i = 0; i < HMAX; i++) {
		if (main->servers[server_id]->memory->buckets[i] != NULL) {
		   server_memory *curr_server = main->servers[server_id];
			node_t* curr_elem = curr_server->memory->buckets[i]->head;

			while (curr_elem != NULL) {
				// find the key-value pair for the current element
				info *info = curr_elem->data;
				char *key = (char *)info->key;
				char *value = (char *)info->value;

				// redistribute all the data stored on the deleted server
				int aux_server_id = server_id;
				loader_store(main, key, value, &aux_server_id);

				curr_elem = curr_elem->next;
			}
		}
	}

	// free deleted server memory
	free_server_memory(main->servers[server_id]);
	main->servers[server_id] = NULL;
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
	// find hash value for the received key
	unsigned int hash_value = hash_function_key(key);
	int index = -1;

	if (main->no_hashring_points == 0) {
		// if there are no servers, add pair to first server
		index = 0;
	} else {
		// if the hash value is greater than the last server's hash value,
		// add the pair to the first server (circular vector)
		int last_server = main->hashring[main->no_hashring_points - 1];
		if (hash_function_servers(&last_server) < hash_value) {
			index = 0;
		} else {
			// else, find first server that hash_server >= hash_value
			int start = 0;
			int end = main->no_hashring_points - 1;

			// iterate through hashring (binary search)
			while (start <= end) {
				int mid = start + (end - start) / 2;  // avoid overflow
				int server_value = main->hashring[mid];
				if (hash_value > hash_function_servers(&server_value)) {
					start = mid + 1;
				} else {
					end = mid - 1;
					index = mid;
				}
			}
		}
	}

	// keep index in bounds (maximum 99999 servers)
	int server_index = main->hashring[index] % MAX_SERVERS;

	// finally, add pair to the found server and return the server ID
	server_store(main->servers[server_index], key, value);
	*server_id = server_index;
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	// find hash value for the received key
	unsigned int hash_value = hash_function_key(key);
	int index = -1;

	if (main->no_hashring_points == 0) {
		// if there are no servers, first server is found
		index = 0;
	} else {
		// if hash_value >= hash_value_last_server,
		// search for the pair in the first server (circular vector)
		int last_server = main->hashring[main->no_hashring_points - 1];
		if (hash_function_servers(&last_server) < hash_value) {
			index = 0;
		} else {
			// else find first server that hash_server >= hash_value
			int start = 0;
			int end = main->no_hashring_points - 1;

			// iterate through hashring (binary search)
			while (start <= end) {
				int mid = start + (end - start) / 2;  // avoid overflow
				int server_value = main->hashring[mid];
				if (hash_value > hash_function_servers(&server_value)) {
					start = mid + 1;
				} else {
					end = mid - 1;
					index = mid;
				}
			}
		}
	}

	// keep index in bounds (maximum 99999 servers)
	int server_index = main->hashring[index] % MAX_SERVERS;

	// return the key-pair value of the found server
	*server_id = server_index;
	return server_retrieve(main->servers[server_index], key);
}

void free_load_balancer(load_balancer *main) {
	// free dynamically allocated memory
	if (!main)
		return;

	for (int i = 0; i < MAX_SERVERS; i++)
		if (main->servers[i]) {
			free_server_memory(main->servers[i]);
			main->servers[i] = NULL;
		}

	if (main->servers) {
		free(main->servers);
		main->servers = NULL;
	}
	if (main->hashring) {
		free(main->hashring);
		main->hashring = NULL;
	}

	if (main) {
		free(main);
		main = NULL;
	}
}
