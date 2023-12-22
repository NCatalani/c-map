#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <openssl/sha.h>

#include "map.h"

#define MAX_TEST_INSERT_KEYS 100
#define MAX_SEARCH_INDEX 1000000
#define POOLING_SIZE 100000
#define SEARCH_KEYS 1


/**
 * @brief Realiza o hash SHA256 de uma string
 *
 * @param key Chave a ser hasheada
 * @return unsigned char* Array de bytes com o hash alocado no heap. O retorno deve ser liberado.
 */
unsigned char *sha256_hash(char *key) {
    unsigned char hash_buff[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *hash =  NULL;

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, key, strlen(key));
    SHA256_Final(hash_buff, &sha256);

    hash = malloc(SHA256_DIGEST_LENGTH);
    memcpy(hash, hash_buff, SHA256_DIGEST_LENGTH);

    return hash;
}


hashmap_t* hm_new() {
    hashmap_t *hashmap = malloc(sizeof(hashmap_t));

    if (!hashmap) {
        return NULL;
    }

    hashmap->list = NULL;
    hashmap->capacity = 0;
    hashmap->size = 0;

    return hashmap;
}

/**
 * @brief Cria uma novo hashmap aloca no heap
 *
 * @return hashmap_t* Ponteiro para o hashmap
 */
hashmap_t* hm_create(int capacity) {
    hashmap_t *hashmap = hm_new();

    if (!hashmap) {
        return NULL;
    }

    hashmap->list = calloc(capacity, sizeof(node_t*));
    hashmap->capacity = capacity;

    return hashmap;
}


/**
 * @brief Criar um novo hashmap com capacidade inicial padrão
 * 
 * @return hashmap_t* Hashmap
 */
hashmap_t* hm_create_default(void) {
    return hm_create(HM_INITIAL_CAPACITY);
}


/**
 * @brief Retorna o fator de carga do hashmap
 * 
 * @param hashmap Hashmap
 * @return int Fator de carga
 */
float hm_get_load_factor(hashmap_t *hashmap) {
    if (hashmap == NULL || hashmap->size > hashmap->capacity) {
        return HM_ERROR;
    }

    //printf("[%s][%s](%d) - hm_get_load_factor: %d/%d\n", __FILE__, __FUNCTION__, __LINE__, hashmap->size, hashmap->capacity);
    //printf("[%s][%s](%d) - hm_get_load_factor: %f\n", __FILE__, __FUNCTION__, __LINE__, (float)hashmap->size/hashmap->capacity);

    if (hashmap->capacity == 0) {
        return 0.0;
    }

    return (float)hashmap->size/hashmap->capacity;
}

/**
 * @brief Libera a memoria alocada para o hashmap
 *
 * @param hashmap_p Referencia para o ponteiro do hashmap
 */
void hm_free(void **hashmap_p) {
    hashmap_t *hashmap = NULL;
    node_t *current_node, *next_node = NULL;

    if (hashmap_p == NULL) {
        return;
    }

    hashmap = *hashmap_p;

    if (hashmap == NULL && hashmap->list == NULL) {
        return;
    }

    for (int i = 0; i < hashmap->capacity; i++) {
        current_node = hashmap->list[i];

        while (current_node != NULL) {
            next_node = current_node->next;

            switch(current_node->value_type) {
                case HM_VALUE_STR:
                    if (current_node->value != NULL) {
                        free(current_node->value);
                        current_node->value = NULL;
                    }
                    break;
                case HM_VALUE_MAP:
                    if (current_node->value != NULL) {
                        hm_free((void **) &current_node->value);
                        current_node->value = NULL;
                    }
                    break;
                default:
                    break;  
            }

            if (current_node->key != NULL) {
                free(current_node->key);
                current_node->key = NULL;
            }

            free(current_node);

            current_node = next_node;
        }
    }

    free(hashmap->list);
    hashmap->list = NULL;

    free(hashmap);
    *hashmap_p = NULL;
}


/**
 * @brief Realiza o hash da string e retorna um
 * id de bucket v�lido
 *
 * @param key Chave a ser hasheada
 * @return unsigned int Valor do hash
 */
int hm_hash(hashmap_t *hashmap, char *key) {

    unsigned char *hash = NULL;
    unsigned int hash_code = 0;

    hash = sha256_hash(key);

    if (hash == NULL) {
        return HM_ERROR;
    }

    // Transforma os 4 primeiros bytes do hash em um inteiro unsigned
    hash_code = ((unsigned int)hash[0] << 24) |
        ((unsigned int)hash[1] << 16) |
        ((unsigned int)hash[2] << 8)  |
        (unsigned int)hash[3];

    free(hash);
    hash = NULL;

    return hash_code % hashmap->capacity;
}


void hm_rehash_insert(hashmap_t* hashmap, char* key, node_value_t value_type, void* value) {

    unsigned int bucket = 0;
    node_t *new_node = NULL;

    bucket = hm_hash(hashmap, key);
    if (bucket == HM_ERROR) {
        return;
    }

    new_node = malloc(sizeof(node_t));

    if (new_node == NULL) {
        return;
    }

    if (key == NULL || key[0] == '\0') {
        return;
    }

    new_node->key = key;
    new_node->value = value;
    new_node->value_type = value_type;
    new_node->next = hashmap->list[bucket];

    hashmap->list[bucket] = new_node;
}



/**
 * @brief Realiza o resize do hashmap por um fator de resize
 * 
 * @param hashmap Hashmap
 * @param resize_factor Factor de resize
 * @return int Novo tamanho, em caso de sucesso. HM_ERROR, em caso de erro.
 */
int hm_resize(hashmap_t* hashmap, float resize_factor) {
    size_t new_size = 0;

    hashmap_t* aux_hashmap = NULL;
    node_t* current_node = NULL;
    node_t* next_node = NULL;

    //printf("[%s][%s](%d) - Entrada da funcao de resize\n", __FILE__, __FUNCTION__, __LINE__);

    if (hashmap == NULL || resize_factor <= 1.0) {
        return HM_ERROR;
    }
    
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


    free(hashmap->list);

    hashmap->list = aux_hashmap->list;
    hashmap->capacity = aux_hashmap->capacity;

    free(aux_hashmap);

    return HM_SUCCESS;
}


/**
 * @brief Busca um elemento no hashmap
 *
 * @param hashmap Ponteiro para o hashmap
 * @param key Chave do elemento
 * @return int Caso encontre o elemento, retorna o valor. Caso contr�rio, retorna HM_NOT_FOUND. Em caso de erro, retorna HM_ERROR.
 */
int hm_search(hashmap_t *hashmap, char *key, void **value) {
    unsigned int bucket = 0;
    int hash_code = 0;

    hash_code = hm_hash(hashmap, key);

    if (hash_code == HM_ERROR || value == NULL) {
        return HM_ERROR;
    }

    bucket = hash_code;

    node_t *list = hashmap->list[bucket];

    while (list != NULL) {
        if (strcmp(key, list->key) == 0) {
            *value = list->value;
            return HM_SUCCESS;
        }
        list = list->next;
    }

    return HM_NOT_FOUND;
}


/**
 * @brief Insere um elemento no hashmap
 *
 * @param hashmap Ponteiro para o hashmap
 * @param key Chave do elemento
 * @param value Valor do elemento
 */
void hm_insert(hashmap_t *hashmap, char *key, node_value_t value_type, void* value) {
    unsigned int bucket = 0;
    int hash_code = 0;
    double current_load_factor = 0;

    void* new_node_val = NULL;
    char* new_node_key = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return;
    }


    current_load_factor = hm_get_load_factor(hashmap);
    if (current_load_factor == HM_ERROR) {
        return;
    }

    if (current_load_factor >= HM_LOAD_FACTOR_THRESHOLD) {
        //printf("[%s][%s](%d) - Resize necessario! [load:%.2f|threshold:%.2f\n", __FILE__, __FUNCTION__, __LINE__, current_load_factor, HM_LOAD_FACTOR_THRESHOLD);
        if (hm_resize(hashmap, HM_RESIZE_FACTOR) == HM_ERROR) {
            return;
        }
    }

    if (key == NULL || key[0] == '\0') {
        return;
    }

    hash_code = hm_hash(hashmap, key);

    if (hash_code == HM_ERROR) {
        return;
    }

    bucket = hash_code;

    new_node_key = strdup(key);
    new_node_val = value;

    switch (value_type) {
        case HM_VALUE_MAP:
            if (value == NULL) {
                if ((new_node_val = hm_create_default()) == NULL) {
                    return;
                }
            }
            break;
        case HM_VALUE_STR:
            if (value == NULL) {
                return;
            }
            new_node_val = strdup(value);
            break;
    }


    node_t *new_node = malloc(sizeof(node_t));
    new_node->key = new_node_key;
    new_node->value = new_node_val;
    new_node->value_type = value_type;
    new_node->next = NULL;

    if (hashmap->list[bucket] != NULL) {
        new_node->next = hashmap->list[bucket];
    }

    hashmap->list[bucket] = new_node;

    //printf("[%s][%s](%d) - Inseriu a chave %s no bucket %d\n", __FILE__, __FUNCTION__, __LINE__, key, bucket);
    hashmap->size++;
    //printf("[%s][%s](%d) - hm_insert: [s/c %d/%d]\n", __FILE__, __FUNCTION__, __LINE__, hashmap->size, hashmap->capacity);
}


int main() {
    int i;

    hashmap_t *hm = NULL;
    hashmap_t *hm2 = NULL;

    char key_buffer[14] = {'\0'};
    char value_buffer[16] = {'\0'};

    clock_t outer_end, outer_start, start, end, start_p, end_p;
    double time_spent = 0.0;

    void* val = NULL;


    //printf("[%s][%s](%d) - Criando hashmap root\n", __FILE__, __FUNCTION__, __LINE__);

    outer_start = clock();

    hm = hm_create_default();

    start = clock();

    // Criando chaves STR para inserir no hashmap
    for(i = 0; i < MAX_TEST_INSERT_KEYS; i++) {
        sprintf(key_buffer, "key_%d", i);
        sprintf(value_buffer, "value_%d", i);

        hm_insert(hm, key_buffer, HM_VALUE_STR, value_buffer);
    }
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken to insert: %.2f ms [rate: %.2f ins/s]\n", time_spent, MAX_TEST_INSERT_KEYS/time_spent*1000.0);

    start = clock();
    while (i > 0 && SEARCH_KEYS) {

        sprintf(key_buffer, "key_%d", rand() % MAX_SEARCH_INDEX);
        if (hm_search(hm, key_buffer, &val) != HM_SUCCESS) {
            //printf("[%s][%s](%d) - hm_search: k:[%s] v:[%s]\n", __FILE__, __FUNCTION__, __LINE__, key_buffer, "null");
        }

        //printf("[%s][%s](%d) - hm_search: k:[%s] v:[%s]\n", __FILE__, __FUNCTION__, __LINE__, key_buffer, (char*)val);
        i--;
    }
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken to search: %.2f ms [rate: %.2f lkps/s]\n", time_spent, MAX_TEST_INSERT_KEYS/time_spent*1000.0);

    outer_end = clock();

    time_spent = (double)(outer_end - outer_start) / CLOCKS_PER_SEC * 1000.0;

    printf("Time taken in total: %.2f ms\n", time_spent);

    hm_free((void **)&hm);
}

