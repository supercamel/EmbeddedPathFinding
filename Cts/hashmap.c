#include <string.h>
#include <math.h>
#include "hashmap.h"
#include "array.h"
#include "cts_string.h"
#include "block_pool.h"

static const size_t BucketSizes[] = {
    11,
    23,
    47,
    97,
    199,
    409,
    823,
    1657,
    3323,
    6653,
    13309,
    26627,
    53279,
    106573,
    SIZE_MAX
};

typedef struct HashMapBucketNode {
    cts_pointer key;
    cts_pointer value;
    struct HashMapBucketNode* next;
} HashMapBucketNode;

typedef struct HashMapPrivate {
    HashMapKeyHashFunc hash_func;
    HashMapKeyEqualFunc equal_func;
    CtsFreeFunc key_destroy_func;
    cts_pointer key_user_pointer;
    CtsFreeFunc value_destroy_func;
    cts_pointer value_user_pointer;
    HashMapBucketNode** buckets; // array of buckets of size BucketSizes[bucket_size_index]
    int bucket_size_index;
    size_t n_entries;
    CtsBlockPool* bucket_pool;
} HashMapPrivate;


CTS_DEFINE_TYPE(CtsBase, cts_base, CtsHashMap, cts_hash_map)

static size_t hash_to_bucket(uint32_t hash, size_t bucket_size)
{
    return hash % bucket_size;
}

bool cts_hash_map_construct(CtsHashMap* map)
{
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)map);
    map->priv = cts_allocator_alloc(alloc, sizeof(HashMapPrivate));
    if (map->priv == NULL) {
        return false;
    }
    map->priv->bucket_pool = cts_block_pool_new(alloc, sizeof(HashMapBucketNode), 12);
    if(map->priv->bucket_pool == NULL) {
        cts_allocator_free(alloc, map->priv);
        return false;
    }
    map->priv->hash_func = cts_hash_map_hash_string;
    map->priv->equal_func = cts_hash_map_equal_string;
    map->priv->key_destroy_func = (CtsFreeFunc)cts_allocator_free;
    map->priv->value_destroy_func = (CtsFreeFunc)cts_allocator_free;
    map->priv->buckets = cts_allocator_alloc(alloc, sizeof(HashMapBucketNode*) * BucketSizes[0]);
    if (map->priv->buckets == NULL) {
        cts_allocator_free(alloc, map->priv);
        return false;
    }
    memset(map->priv->buckets, 0, sizeof(HashMapBucketNode*) * BucketSizes[0]);

    map->priv->bucket_size_index = 0;
    map->priv->n_entries = 0;
    return true;
}

CtsHashMap* cts_hash_map_new_full(CtsAllocator* alloc, 
    HashMapKeyHashFunc hash_func, 
    HashMapKeyEqualFunc equal_func,
    CtsFreeFunc key_destroy_func, cts_pointer key_user_pointer,
    CtsFreeFunc value_destroy_func, cts_pointer value_user_pointer)
{
    CtsHashMap* map = cts_hash_map_new(alloc);
    if (map == NULL) {
        return NULL;
    }
    map->priv->hash_func = hash_func;
    map->priv->equal_func = equal_func;
    map->priv->key_destroy_func = key_destroy_func;
    map->priv->key_user_pointer = key_user_pointer;
    map->priv->value_destroy_func = value_destroy_func;
    map->priv->value_user_pointer = value_user_pointer;
    return map;
}

void cts_hash_map_destruct(CtsHashMap* map)
{
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)map);
    cts_hash_map_clear(map);
    cts_block_pool_delete(map->priv->bucket_pool);
    cts_allocator_free(alloc, map->priv->buckets);
    cts_allocator_free(alloc, map->priv);
}

uint32_t cts_hash_map_hash_string(const cts_pointer key)
{
    const char* str = (const char*)key;
    uint32_t hash = 0;
    while (*str != '\0') {
        hash = hash * 31 + *str;
        str++;
    }
    return hash;
}

bool cts_hash_map_equal_string(const cts_pointer key1, const cts_pointer key2)
{
    return cts_strcmp((const char*)key1, (const char*)key2) == 0;
}

uint32_t cts_hash_map_hash_int(const cts_pointer key)
{
    const uint32_t* number = (const uint32_t*)key;
    float A = 0.6180339887; // ((sqrt(5) - 1) / 2), the fractional part of the golden ratio
    uint32_t hash = floor(UINT32_MAX * fmod((*number * A), 1));
    return hash;
}

bool cts_hash_map_equal_int(const cts_pointer key1, const cts_pointer key2)
{
    return *(const uint32_t*)key1 == *(const uint32_t*)key2;
}

static bool cts_hash_map_resize(CtsHashMap* self)
{
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    HashMapPrivate* priv = self->priv;
    size_t new_size_index = priv->bucket_size_index + 1;
    if (new_size_index >= (sizeof(BucketSizes) / sizeof(size_t)) - 1) {
        // We've hit the maximum bucket size, can't resize
        return false;
    }
    size_t new_size = BucketSizes[new_size_index];
    HashMapBucketNode** new_buckets = cts_allocator_alloc(alloc, sizeof(HashMapBucketNode*) * new_size);
    if (new_buckets == NULL) {
        return false;
    }
    memset(new_buckets, 0, sizeof(HashMapBucketNode*) * new_size);
    // Rehash entries
    for (size_t i = 0; i < BucketSizes[priv->bucket_size_index]; i++) {
        HashMapBucketNode* node = priv->buckets[i];
        while (node != NULL) {
            HashMapBucketNode* next = node->next;
            size_t new_bucket_index = hash_to_bucket(priv->hash_func(node->key), new_size);
            node->next = new_buckets[new_bucket_index];
            new_buckets[new_bucket_index] = node;
            node = next;
        }
    }
    cts_allocator_free(alloc, priv->buckets);
    priv->buckets = new_buckets;
    priv->bucket_size_index = new_size_index;
    return true;
}


bool cts_hash_map_set(CtsHashMap* self, cts_pointer key, cts_pointer value)
{
    HashMapPrivate* priv = self->priv;
    uint32_t hash = priv->hash_func(key);
    size_t bucket_index = hash_to_bucket(hash, BucketSizes[priv->bucket_size_index]);
    HashMapBucketNode* node = self->priv->buckets[bucket_index];
    while (node != NULL) {
        if (priv->equal_func(node->key, key)) {
            self->priv->key_destroy_func(
                self->priv->key_user_pointer,
                node->key);
            self->priv->value_destroy_func(
                self->priv->value_user_pointer,
                node->value);
            node->key = key;
            node->value = value;

            return true;
        }
        node = node->next;
    }

    // key not found, add new node
    //node = cts_allocator_alloc(cts_base_get_allocator((CtsBase*)self), sizeof(HashMapBucketNode));
    node = cts_block_pool_alloc(self->priv->bucket_pool);
    if (node == NULL) {
        return false;
    }
    node->key = key;
    node->value = value;
    node->next = self->priv->buckets[bucket_index];
    self->priv->buckets[bucket_index] = node;
    self->priv->n_entries++;

    // check if we need to resize
    float pc = (float)self->priv->n_entries / BucketSizes[priv->bucket_size_index];
    if (pc > 0.75) {
        cts_hash_map_resize(self); 
        // while it's possible that the resize failed, we don't really care. 
        // it would only fail if memory allocation failed, and there isn't much we can do about that.
        // it just means that we'll have a few more collisions than we would have otherwise
    }
    return true;
}

cts_pointer cts_hash_map_get(CtsHashMap* self, cts_pointer key)
{
    HashMapPrivate* priv = self->priv;
    uint32_t hash = priv->hash_func(key);
    size_t bucket_index = hash_to_bucket(hash, BucketSizes[priv->bucket_size_index]);
    HashMapBucketNode* node = self->priv->buckets[bucket_index];
    while (node != NULL) {
        if (priv->equal_func(node->key, key)) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

bool cts_hash_map_remove(CtsHashMap* self, cts_pointer key)
{
    HashMapPrivate* priv = self->priv;
    uint32_t hash = priv->hash_func(key);
    size_t bucket_index = hash_to_bucket(hash, BucketSizes[priv->bucket_size_index]);

    HashMapBucketNode* node = priv->buckets[bucket_index];
    HashMapBucketNode* prev_node = NULL;

    while (node != NULL) {
        if (priv->equal_func(node->key, key)) {
            // Found the node. Now let's remove it.

            if (prev_node == NULL) {
                // Node is at the head of the bucket
                priv->buckets[bucket_index] = node->next;
            } else {
                // Node is in the middle or at the end of the bucket
                prev_node->next = node->next;
            }

            // Free the node
            if(self->priv->key_destroy_func != NULL)
                self->priv->key_destroy_func(
                    self->priv->key_user_pointer,
                    node->key);
            if(self->priv->value_destroy_func != NULL)
                self->priv->value_destroy_func(
                    self->priv->value_user_pointer,
                    node->value);

            //cts_allocator_free(cts_base_get_allocator((CtsBase*)self), node);
            cts_block_pool_free(self->priv->bucket_pool, node);
            priv->n_entries--;

            return true;
        }

        prev_node = node;
        node = node->next;
    }

    // If we got here, we did not find the key
    return false;
}

bool cts_hash_map_contains(CtsHashMap* self, cts_pointer key)
{
    HashMapPrivate* priv = self->priv;
    uint32_t hash = priv->hash_func(key);
    size_t bucket_index = hash_to_bucket(hash, BucketSizes[priv->bucket_size_index]);

    HashMapBucketNode* node = priv->buckets[bucket_index];
    while (node != NULL) {
        if (priv->equal_func(node->key, key)) {
            return true; // Key found
        }
        node = node->next;
    }
    return false; // Key not found
}

size_t cts_hash_map_size(CtsHashMap* self)
{
    return self->priv->n_entries;
}

void cts_hash_map_clear(CtsHashMap* self)
{
    HashMapPrivate* priv = self->priv;

    for (size_t i = 0; i < BucketSizes[priv->bucket_size_index]; i++) {
        HashMapBucketNode* node = priv->buckets[i];
        while (node != NULL) {
            if(self->priv->key_destroy_func != NULL)
                self->priv->key_destroy_func(
                    self->priv->key_user_pointer,
                    node->key);
            if(self->priv->value_destroy_func != NULL)
                self->priv->value_destroy_func(
                    self->priv->value_user_pointer,
                    node->value);

            HashMapBucketNode* next_node = node->next;
            // COMMENT ME OUT IF USING block_pool or bucket_pool
            //cts_allocator_free(cts_base_get_allocator((CtsBase*)self), node);
            node = next_node;
        }
        priv->buckets[i] = NULL;
    }
    cts_block_pool_clear(self->priv->bucket_pool);
    priv->n_entries = 0;
}

CtsSList* cts_hash_map_get_keys(CtsHashMap* self)
{
    CtsSList* keys = cts_slist_new(cts_base_get_allocator((CtsBase*)self));
    if(keys == NULL) {
        return NULL;
    }

    HashMapPrivate* priv = self->priv;
    for (size_t i = 0; i < BucketSizes[priv->bucket_size_index]; i++) {
        HashMapBucketNode* node = priv->buckets[i];
        while (node != NULL) {
            if (!cts_slist_append(keys, node->key)) {
                cts_slist_destroy(keys);
                return NULL;
            }
            node = node->next;
        }
    }

    return keys;
}


