#ifndef __MAP_H_
#define __MAP_H_

#define HM_INITIAL_CAPACITY 5
#define HM_LOAD_FACTOR_THRESHOLD 0.75
#define HM_RESIZE_FACTOR 2.0

#define HM_SUCCESS -1
#define HM_ERROR -2
#define HM_NOT_FOUND -3

typedef enum {
    HM_VALUE_STR,
    HM_VALUE_MAP
} node_value_t;

typedef struct node {
    char *key;
    void *value;
    node_value_t value_type;
    struct node *next;
} node_t;

typedef struct {
    node_t **list;
    int size;
    int capacity;
} hashmap_t;

// Hash Table functions
node_t* hm_node_new(void);
node_t* hm_node_create(char* key, node_value_t value_type, void* value, void* next);
hashmap_t* hm_create(int capacity);
hashmap_t* hm_create_default(void);
int hm_hash(hashmap_t *hm, char* str);
int hm_search(hashmap_t* hm, void **value, ...);
int hm_resize(hashmap_t* hm, float factor);
float hm_get_load_factor(hashmap_t* hm);
void hm_free(void** hm_p);
void hm_node_free(void** node_p);
void hm_insert(hashmap_t* hm,  node_value_t value_type, void* value, ...);
void hm_rehash_insert(hashmap_t* hm, char* key, node_value_t value_type, void* value);
// Aux
unsigned char* sha256_hash(char* key);

#endif
