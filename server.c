/* Copyright 2023 Munteanu Eugen 315CA */
#include "hashtable.h"
#include "server.h"

server_memory *init_server_memory()
{
	// allocate new server
	server_memory *new_server = calloc(1, sizeof(server_memory));
	DIE(!new_server, "calloc() for *new_server failed\n");

	// create its hashtable, according to the structure
	new_server->memory = ht_create(HMAX, hash_function_string,
								compare_function_strings, key_val_free_function);
	return new_server;
}

void server_store(server_memory *server, char *key, char *value) {
	if (!server || !(server->memory) || !key || !value)
		return;

	// put key-value pair in server (hashtable)
	// +1 for null terminator
	ht_put(server->memory, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	if (!server || !(server->memory) || !key)
		return NULL;

	// find the value associated with the key in the server and return it
	char *value = ht_get(server->memory, key);

    return value;
}

void server_remove(server_memory *server, char *key) {
	if (!server || !(server->memory) || !key)
		return;

	// remove key-value pair from the given server
	ht_remove_entry(server->memory, key);
}

void free_server_memory(server_memory *server) {
	if (!server || !(server->memory))
		return;

	// free server memory
	ht_free(server->memory);
	server->memory = NULL;
	free(server);
	server = NULL;
}
