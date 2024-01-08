#ifndef __MAP_H_
#define __MAP_H_

#define HM_LIST_INITIAL_CAPACITY 5
#define HM_LIST_RESIZE_FACTOR 2.0

#define HM_INITIAL_CAPACITY 5
#define HM_LOAD_FACTOR_THRESHOLD 0.75
#define HM_RESIZE_FACTOR 2.0

#define HM_SUCCESS -1
#define HM_ERROR -2
#define HM_NOT_FOUND -3

typedef enum {
    HM_VALUE_STR,
    HM_VALUE_MAP,
    HM_VALUE_LIST
} node_value_t;

typedef struct node {
    char* key;
    void* value;
    node_value_t value_type;
    struct node* next;
} node_t;

typedef struct {
    node_t** items;
    int size;
    int capacity;
} list_t;

typedef struct
{
    node_t** list;
    int size;
    int capacity;
} hashmap_t;

node_t* hm_node_new(void);
node_t* hm_node_create(char* key, node_value_t value_type, void* value, void* next);
list_t* hm_list_new(void);
list_t* hm_list_create(int capacity);
list_t* hm_list_create_default(void);
hashmap_t* hm_new(void);
hashmap_t* hm_create(int capacity);
hashmap_t* hm_create_default(void);
char* hm_serialize(hashmap_t* hm);
char* hm_serialize_node(node_t* node);
int hm_hash(hashmap_t* hm, char* str);
int hm_search(hashmap_t* hm, void** value, ...);
int hm_resize(hashmap_t* hm, float factor);
int hm_node_compare(const void* a, const void* b);
int hm_list_contains(list_t* list, char* str);
float hm_get_load_factor(hashmap_t* hm);
void hm_insert(hashmap_t* hm, node_value_t value_type, void* value, ...);
void hm_rehash_insert(hashmap_t* hm, char* key, node_value_t value_type, void* value);
void hm_list_append(list_t* list, node_t* node);
void hm_list_append_str(list_t* list, char* str);
void hm_free(void** hm_p);
void hm_node_free(void** node_p);
void hm_list_free(void** list_p);
unsigned char* sha256_hash(char* key);

#endif
