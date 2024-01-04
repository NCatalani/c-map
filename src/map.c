#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cmap/log.h>
#include <cmap/map.h>

#include <openssl/sha.h>

/**
 * @brief Hash's a string using SHA256
 *
 * @param key String to be hashed
 * @return unsigned char* Heap allocated byte array.
 */
unsigned char* sha256_hash(char* str)
{
    unsigned char hash_buff[SHA256_DIGEST_LENGTH] = { 0 };
    unsigned char* hash = NULL;

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str, strlen(str));
    SHA256_Final(hash_buff, &sha256);

    hash = calloc(SHA256_DIGEST_LENGTH, sizeof(unsigned char));
    memcpy(hash, hash_buff, SHA256_DIGEST_LENGTH);

    return hash;
}

/**
 * @brief Initializes a new node
 *
 * @return node_t* Pointer to the new node
 */
node_t* hm_node_new(void)
{
    node_t* node = malloc(sizeof(node_t));

    if (!node) {
        return NULL;
    }

    node->key = NULL;
    node->value = NULL;
    node->value_type = HM_VALUE_MAP;
    node->next = NULL;

    return node;
}

/**
 * @brief Instantiates a heap allocated node using the given parameters as a
 * reference.
 *
 * @param key Node key
 * @param value_type Node value type
 * @param value Node value
 * @param next Next node
 * @return node_t* Instantiated node
 */
node_t* hm_node_create(char* key, node_value_t value_type, void* value, void* next)
{
    node_t* node = hm_node_new();

    if (!node) {
        return NULL;
    }

    node->key = key;
    node->value = value;
    node->value_type = value_type;
    node->next = next;

    return node;
}

/**
 * @brief Deallocates the memory used by a node
 *
 * @param node_p Reference to the node pointer
 */
void hm_node_free(void** node_p)
{
    if (node_p == NULL || *node_p == NULL) {
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Freeing node [%p][%p]", *node_p, node_p);

    node_t* node = *node_p;

    HM_LOG(LOG_LEVEL_DEBUG, "Freeing node key [%s][%p]", node->key, &node->key);
    if (node->key) {
        free(node->key);
        node->key = NULL;
    }

    if (node->value) {
        switch (node->value_type) {
        case HM_VALUE_STR:
            free(node->value);
            break;
        case HM_VALUE_MAP:
            hm_free((void**)&node->value);
            break;
        }
        node->value = NULL;
    }

    free(node);
    *node_p = NULL;
}

/**
 * @brief Initializes a new hashmap
 *
 * @return hashmap_t* Pointer to the new hashmap
 */
hashmap_t* hm_new(void)
{
    hashmap_t* hashmap = malloc(sizeof(hashmap_t));

    if (!hashmap) {
        return NULL;
    }

    hashmap->list = NULL;
    hashmap->capacity = 0;
    hashmap->size = 0;

    return hashmap;
}

/**
 * @brief Instantiates a heap allocated hashmap
 *
 * @param capacity The hashmap's initial capacity
 * @return hashmap_t* Pointer to the new hashmap
 */
hashmap_t* hm_create(int capacity)
{
    hashmap_t* hashmap = hm_new();

    if (!hashmap) {
        return NULL;
    }

    hashmap->list = calloc(capacity, sizeof(node_t*));
    hashmap->capacity = capacity;

    return hashmap;
}

/**
 * @brief Instantiates a heap allocated hashmap with the default capacity
 *
 * @return hashmap_t*
 */
hashmap_t* hm_create_default(void)
{
    return hm_create(HM_INITIAL_CAPACITY);
}

/**
 * @brief Retrieves the load factor of the hashmap
 *
 * @param hashmap Pointer to the hashmap
 * @return float Load factor
 */
float hm_get_load_factor(hashmap_t* hashmap)
{
    if (hashmap == NULL || hashmap->size > hashmap->capacity) {
        return HM_ERROR;
    }

    if (hashmap->capacity == 0) {
        return 0.0;
    }

    return (float)hashmap->size / hashmap->capacity;
}

/**
 * @brief Deallocates the memory used by the hashmap
 *
 * @param hashmap_p Reference to the hashmap pointer
 */
void hm_free(void** hashmap_p)
{
    if (hashmap_p == NULL || *hashmap_p == NULL) {
        return;
    }

    hashmap_t* hashmap = *hashmap_p;

    for (int i = 0; i < hashmap->capacity; i++) {
        node_t* current_node = hashmap->list[i];
        while (current_node != NULL) {
            node_t* next_node = current_node->next;
            HM_LOG(LOG_LEVEL_DEBUG, "Freeing node [c:%p][n:%p]", current_node, next_node);
            hm_node_free((void**)&current_node);
            current_node = next_node;
        }
        hashmap->list[i] = NULL;
    }

    free(hashmap->list);
    hashmap->list = NULL;

    free(hashmap);
    *hashmap_p = NULL;
}

/**
 * @brief Hashes a key and returns the bucket index
 *
 * @param hashmap Pointer to the hashmap
 * @param key Key to be hashed
 * @return int Bucket index
 */
int hm_hash(hashmap_t* hashmap, char* key)
{

    unsigned char* hash = NULL;
    unsigned int hash_code = 0;

    if ((hash = sha256_hash(key)) == NULL) {
        return HM_ERROR;
    }

    hash_code = ((unsigned int)hash[0] << 24) | ((unsigned int)hash[1] << 16) | ((unsigned int)hash[2] << 8) | (unsigned int)hash[3];

    free(hash);
    hash = NULL;

    return hash_code % hashmap->capacity;
}

/**
 * @brief Rehashes a key to a new hashmap
 *
 * @param hashmap Pointer to the hashmap
 * @param key Key to be rehashed
 * @param value_type Value type
 * @param value Value
 */
void hm_rehash_insert(hashmap_t* hashmap, char* key, node_value_t value_type, void* value)
{

    int bucket = 0;
    node_t* node = NULL;

    if (key == NULL || key[0] == '\0') {
        return;
    }

    bucket = hm_hash(hashmap, key);
    if (bucket == HM_ERROR) {
        HM_LOG(LOG_LEVEL_ERROR, "Error hashing key!", key);
        return;
    }

    if ((node = hm_node_create(strdup(key), value_type, value, hashmap->list[bucket])) == NULL) {
        return;
    }

    // Sets the new node as the head of the bucket
    hashmap->list[bucket] = node;
}

/**
 * @brief Resizes the hashmap according to the given factor
 *
 * @param hashmap Pointer to the hashmap
 * @param resize_factor Resize factor
 * @return int Status code (HM_SUCCESS or HM_ERROR)
 */
int hm_resize(hashmap_t* hashmap, float resize_factor)
{
    size_t new_size = 0;

    hashmap_t* aux_hashmap = NULL;
    node_t* current_node = NULL;
    node_t* next_node = NULL;

    if (hashmap == NULL || resize_factor <= 1.0) {
        return HM_ERROR;
    }

    // Creates a new auxiliar hashmap to store the rehashed nodes
    new_size = (size_t)(hashmap->capacity * resize_factor);
    aux_hashmap = hm_create(new_size);
    if (aux_hashmap == NULL) {
        return HM_ERROR;
    }

    for (int i = 0; i < hashmap->capacity; i++) {
        current_node = hashmap->list[i];

        while (current_node != NULL) {
            hm_rehash_insert(aux_hashmap, current_node->key, current_node->value_type, current_node->value);

            next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
    }

    // Frees the old hashmap
    free(hashmap->list);
    hashmap->list = aux_hashmap->list;
    hashmap->capacity = aux_hashmap->capacity;

    free(aux_hashmap);

    return HM_SUCCESS;
}

/**
 * @brief Searches for a value inside the hashmap.
 *
 * The function receives a variable number of keys as arguments. It will search
 * for the hierarchy of keys inside the hashmap, returning the following
 * possible return codes:
 *
 *  - HM_SUCCESS: The value was found and the pointer to it was stored in the
 * value argument.
 *
 *  - HM_NOT_FOUND: The value was not found.
 *
 *  - HM_ERROR: An error ocurred.
 *
 * @param hashmap Pointer to the hashmap
 * @param value Reference to the pointer where the value will be stored
 * @param ... Variable number of keys terminated by a NULL value
 * @return int Status code (HM_SUCCESS, HM_NOT_FOUND or HM_ERROR)
 */
int hm_search(hashmap_t* hashmap, void** value, ...)
{
    va_list args;
    char* key = NULL;

    int bucket = 0;

    hashmap_t* current_hm = hashmap;
    node_t* node = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return HM_ERROR;
    }

    va_start(args, value);

    while (1) {
        // Retrieves the current key
        key = va_arg(args, char*);

        if (key == NULL) {
            break;
        }

        // Hashes the key
        if ((bucket = hm_hash(current_hm, key)) == HM_ERROR) {
            va_end(args);
            return HM_ERROR;
        }

        if ((node = current_hm->list[bucket]) == NULL) {
            va_end(args);
            return HM_NOT_FOUND;
        }

        do {
            if (!strcmp(key, node->key)) {
                break;
            }
            node = node->next;
        } while (node != NULL);

        if (node && node->value_type == HM_VALUE_MAP) {
            current_hm = node->value;
        }
    }

    va_end(args);

    if (node == NULL) {
        return HM_NOT_FOUND;
    }

    *value = node->value;
    return HM_SUCCESS;
}

/**
 * @brief Inserts a value inside the hashmap.
 *
 * The function receives a variable number of keys as arguments.
 * If the key is not found inside the hashmap and it's not the last one, a new
 * hashmap will be created and inserted. If the key is not found inside the
 * hashmap and it's the last one, the value will be inserted.
 *
 * @param hashmap Pointer to the hashmap
 * @param value_type Value type
 * @param value Pointer to the value
 * @param ... Variable number of keys terminated by a NULL value
 */
void hm_insert(hashmap_t* hashmap, node_value_t value_type, void* value, ...)
{
    va_list args;

    int bucket = 0;
    double current_load_factor = 0;

    node_t* node = NULL;
    hashmap_t* current_hm = NULL;

    char* key = NULL;
    char* next_key = NULL;

    char* node_key = NULL;
    void* node_val = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return;
    }

    current_hm = hashmap;

    va_start(args, value);
    key = va_arg(args, char*);

    while (key != NULL) {
        node_value_t node_type = HM_VALUE_MAP;

        next_key = va_arg(args, char*);

        if ((current_load_factor = hm_get_load_factor(current_hm)) == HM_ERROR) {
            return;
        }

        HM_LOG(LOG_LEVEL_DEBUG, "Current load factor: %.2f", current_load_factor);

        if (current_load_factor >= HM_LOAD_FACTOR_THRESHOLD) {
            HM_LOG(LOG_LEVEL_DEBUG, "Load factor threshold reached. Resizing hashmap");
            if (hm_resize(current_hm, HM_RESIZE_FACTOR) == HM_ERROR) {
                return;
            }
        }

        if ((bucket = hm_hash(current_hm, key)) == HM_ERROR) {
            va_end(args);
            return;
        }

        node = current_hm->list[bucket];

        while (node != NULL) {
            if (!strcmp(key, node->key)) {
                break;
            }
            node = node->next;
        }

        if (node == NULL) {
            // Key not found inside current hashmap
            node_key = strdup(key);

            // If it's not the last one, create a new hashmap
            if (next_key != NULL && ((node_val = hm_create_default()) == NULL)) {
                va_end(args);
                free(node_key);
                return;
            }
        } else {
            // Key found inside current hashmap
            // If it's not the last one, checks if its a map
            if (next_key != NULL) {
                if (node->value_type != HM_VALUE_MAP) {
                    HM_LOG(LOG_LEVEL_WARNING, "Key [%s] is not a hashmap", key);
                    va_end(args);
                    return;
                }
            } else {
                // If it's the last one, checks if it's a string
                if (value_type == HM_VALUE_STR && node->value_type == HM_VALUE_STR) {
                    free(node->value);
                    node->value = strdup((char*)value);
                    continue;
                }
            }
        }

        if (next_key == NULL) {
            switch (value_type) {
            case HM_VALUE_STR:
                node_val = value ? strdup(value) : strdup("");
                break;
            case HM_VALUE_MAP:
                node_val = value ? value : hm_create_default();
                break;
            }

            node_type = value_type;
        }

        if (node_key && node_val) {

            node = hm_node_create(node_key, node_type, node_val, current_hm->list[bucket]);

            current_hm->list[bucket] = node;
            current_hm->size++;
        }

        if (node->value_type == HM_VALUE_MAP) {
            current_hm = node->value;
        }

        key = next_key;
        node_key = NULL;
        node_val = NULL;
    }

    va_end(args);
}

/**
 * @brief Serializes a hashmap into a JSON string
 *
 * @param hashmap Pointer to the hashmap
 * @return char* Heap allocated JSON string
 */
char* hm_serialize(hashmap_t* hashmap)
{
    if (hashmap == NULL || hashmap->list == NULL) {
        return NULL;
    }

    size_t buffer_size = 2; // For opening and closing braces
    size_t offset = 0;
    char* serialized = malloc(buffer_size);
    if (serialized == NULL) {
        return NULL;
    }

    serialized[offset++] = '{';

    bool first_item = true;
    for (int i = 0; i < hashmap->capacity; i++) {
        for (node_t* current = hashmap->list[i]; current != NULL; current = current->next) {
            if (!first_item) {
                serialized[offset++] = ',';
                buffer_size++;
            }

            char* node_serialized = hm_serialize_node(current);
            if (node_serialized) {
                size_t node_length = strlen(node_serialized);
                if (offset + node_length >= buffer_size) {
                    buffer_size += node_length;
                    serialized = realloc(serialized, buffer_size);
                    if (serialized == NULL) {
                        free(node_serialized);
                        return NULL;
                    }
                }
                strcpy(serialized + offset, node_serialized);
                offset += node_length;
                first_item = false;
                free(node_serialized);
            }
        }
    }

    serialized[offset++] = '}';
    serialized[offset] = '\0'; // Null-terminate the string

    return serialized;
}

/**
 * @brief  Serializes a node into a JSON string
 *
 * @param node Pointer to the node
 * @return char* Heap allocated JSON string
 */
char* hm_serialize_node(node_t* node)
{
    if (node == NULL) {
        return NULL;
    }

    size_t key_len = strlen(node->key);
    size_t buffer_size = key_len + 4; // Key, quotes, colon

    char* value_serialized = NULL;
    if (node->value_type == HM_VALUE_STR) {
        value_serialized = (char*)node->value;
        buffer_size += strlen(value_serialized) + 2; // Value and quotes
    } else if (node->value_type == HM_VALUE_MAP) {
        value_serialized = hm_serialize((hashmap_t*)node->value);
        buffer_size += value_serialized ? strlen(value_serialized) : 4; // Value or "null"
    }

    char* serialized_node = malloc(buffer_size);
    if (serialized_node == NULL) {
        free(value_serialized);
        return NULL;
    }

    sprintf(serialized_node, "\"%s\":", node->key);
    if (node->value_type == HM_VALUE_STR) {
        strcat(serialized_node, "\"");
        strcat(serialized_node, value_serialized);
        strcat(serialized_node, "\"");
    } else if (node->value_type == HM_VALUE_MAP) {
        if (value_serialized) {
            strcat(serialized_node, value_serialized);
            free(value_serialized);
        } else {
            strcat(serialized_node, "null");
        }
    }

    return serialized_node;
}
