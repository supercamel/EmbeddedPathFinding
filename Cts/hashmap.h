/*
 * CTS_HASH_MAP_H
 *
 * CtsHashMap is a hash map implementation, providing a mechanism to store and retrieve
 * key-value pairs in an efficient manner. It provides a variety of operations such as
 * setting, getting and removing key-value pairs, checking if a key is contained within
 * the map, getting the size of the map and clearing the map.
 *
 * Built-in hash and compare functions for string and integer key types are provided. However, 
 * for other key types, user-defined hash and compare functions must be supplied.
 *
 * Functions to destroy keys and values are required because the CtsHashMap takes responsibility 
 * for releasing these elements whenever they are removed or the map is cleared.
 *
 * To clean up a CtsHashMap, simply call cts_hash_map_unref().
 *
 * Example usage:
 * 
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new hash map with string keys
 * CtsHashMap* map = cts_hash_map_new_full(alloc, 
 *   cts_hash_map_hash_string, 
 *   cts_hash_map_equal_string, 
 *   free, 
 *   (HashMapValueDestroyFunc)cts_allocator_free);
 *
 * // Set some key-value pairs
 * char* key = strdup("Hello");
 * int* value = cts_allocator_alloc(alloc, sizeof(int));
 * *value = 42;
 * cts_hash_map_set(map, key, value);
 * 
 * // Get value by key
 * int* retrieved_value = (int*)cts_hash_map_get(map, "Hello");
 * printf("Value: %d\n", *retrieved_value); // Value: 42
 *
 * // Clean up
 * cts_hash_map_unref(map);
 */

#ifndef CTS_HASH_MAP_H
#define CTS_HASH_MAP_H

#include <stddef.h>
#include <stdint.h>
#include "object.h"
#include "slist.h"

typedef uint32_t (*HashMapKeyHashFunc)(const cts_pointer key);
typedef bool (*HashMapKeyEqualFunc)(const cts_pointer key1, const cts_pointer key2);

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsHashMap, cts_hash_map)
struct HashMapPrivate* priv;
CTS_END_DECLARE_TYPE(CtsHashMap, cts_hash_map)

uint32_t cts_hash_map_hash_string(const cts_pointer key);
bool cts_hash_map_equal_string(const cts_pointer key1, const cts_pointer key2);

uint32_t cts_hash_map_hash_int(const cts_pointer key);
bool cts_hash_map_equal_int(const cts_pointer key1, const cts_pointer key2);

CtsHashMap* cts_hash_map_new_full(CtsAllocator* alloc, 
    HashMapKeyHashFunc hash_func, 
    HashMapKeyEqualFunc equal_func, 
    CtsFreeFunc key_destroy_func, cts_pointer key_alloc,
    CtsFreeFunc value_destroy_func, cts_pointer value_alloc);

bool cts_hash_map_set(CtsHashMap* self, cts_pointer key, cts_pointer value);
cts_pointer cts_hash_map_get(CtsHashMap* self, cts_pointer key);
bool cts_hash_map_remove(CtsHashMap* self, cts_pointer key);
bool cts_hash_map_contains(CtsHashMap* self, cts_pointer key);
size_t cts_hash_map_size(CtsHashMap* self);
void cts_hash_map_clear(CtsHashMap* self);
CtsSList* cts_hash_map_get_keys(CtsHashMap* self);

#endif // CTS_HASH_MAP_H

