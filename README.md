&copy; Munteanu Eugen 315CA 2023
# Load Balancer

This project implements a Load Balancer in C that simulates information management across multiple servers. It utilizes Consistent Hashing to ensure minimal data transfers when servers are added or removed from the system.

The system is built using fundamental data structures such as singly linked lists and hashtables to simulate individual server memory. The Load Balancer manages a "hashring" (circular sorted array) and supports up to 100,000 simultaneous servers.

## Implementation Details

The core data structures (linked lists and hashtables) are based on the labs from the _Data Structures & Algorithms_ course, including functions used in `hashtable.c`.

In short, the main components would be:

* **Hashtable**: Used for server memory with a default of 100 buckets (`HMAX`).
* **Consistent Hashing**: Each server is represented by 3 replicas on the hashring to ensure uniform distribution.
* **Binary Search**: Employed to efficiently find the correct position for a key or a server replica on the hashring.

&nbsp;

## Project Structure

### Server Management (```server.c```)
Each server is represented by a `server_memory` structure containing a hashtable.
* `init_server_memory()`: Dynamically allocates a new server and its hashtable.
* `server_store()`: Adds a key-value pair to the server's memory.
* `server_retrieve()`: Returns the value associated with a specific key.
* `server_remove()`: Deletes a key-value pair from the server.
* `free_server_memory()`: Releases all resources associated with a server.

### Load Balancer Logic (```load_balancer.c```)
The Load Balancer manages the distribution of data across servers using a simulated hashring.
* **Initialization**: `init_load_balancer()` allocates the main structure and the hashring array, which grows dynamically as servers are added.
* **Adding Servers**: `loader_add_server()` adds a server and its 3 replicas. It uses:
    * `add_to_hashring()`: Finds the insertion point via binary search.
    * `insert_at_position_in_hashring()`: Handles the actual array shift and insertion.
    * `balance_load_balancer()`: Redistributes keys from the successor server to the newly added server.
* **Removing Servers**: `loader_remove_server()` removes all 3 replicas. It uses:
    * `delete_from_hashring()`: Locates the replica index.
    * `erase_at_position_in_hashring()`: Removes the element and shifts the array.
    * Redistributes the removed server's data to the next available server on the ring.
* **Data Operations**: 
    * `loader_store()`: Maps a key to a server ID using the hashring and stores the data.
    * `loader_retrieve()`: Maps a key to the responsible server and retrieves the data.

### Utilities and Data Structures
* `linked_list.h`: Singly linked list implementation for hashtable collision handling.
* `hashtable.h.`: Generic hashtable implementation.
* `utils.h`: Contains the `DIE` macro for robust error handling during memory allocation or system calls.

&nbsp;

## Observations and Future Improvements

* **Modularization**: Further code modularization could reduce logic repetition in the hashring management functions.
* **Memory Efficiency**: The server and load balancer structures could be optimized to reduce the memory footprint during high-scale reallocations.
* **Learning Outcomes**: This project provided a deep understanding of how hashtables function in a distributed context and reinforced concepts regarding pointers and complex data structure interactions.