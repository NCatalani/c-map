#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

#include <openssl/sha.h>

#include "map.h"
#include "log.h"


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
 * id de bucket válido
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

    hash_code = ((unsigned int)hash[0] << 24) |
        ((unsigned int)hash[1] << 16) |
        ((unsigned int)hash[2] << 8)  |
        (unsigned int)hash[3];

    free(hash);
    hash = NULL;

    HM_LOG(LOG_LEVEL_DEBUG, "Raw unsigned 4byte hash code: %u", hash_code);

    return hash_code % hashmap->capacity;
}


void hm_rehash_insert(hashmap_t* hashmap, char* key, node_value_t value_type, void* value) {

    int bucket = 0;
    node_t *new_node = NULL;

    if (key == NULL || key[0] == '\0') {
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Rehashing key [%s] to new hashmap [%p]", key, hashmap);

    bucket = hm_hash(hashmap, key);
    if (bucket == HM_ERROR) {
        HM_LOG(LOG_LEVEL_ERROR, "Error hashing key!", key);
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Bucket [%d]", bucket);

    new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
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

    HM_LOG(LOG_LEVEL_DEBUG, "Resizing hashmap [%p][ll: %p] by factor %.2f", hashmap, hashmap->list, resize_factor);

    if (hashmap == NULL || resize_factor <= 1.0) {
        return HM_ERROR;
    }

    new_size = (size_t)(hashmap->capacity * resize_factor);

    aux_hashmap = hm_create(new_size);

    HM_LOG(
        LOG_LEVEL_DEBUG,
        "New aux hashmap [%p][ll: %p] created with capacity %d",
        aux_hashmap,
        aux_hashmap->list,
        aux_hashmap->capacity
    );

    if (aux_hashmap == NULL) {
        return HM_ERROR;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Rehashing elements", hashmap, aux_hashmap);
    for (int i = 0; i < hashmap->capacity; i++) {
        current_node = hashmap->list[i];

        while (current_node != NULL) {
            hm_rehash_insert(aux_hashmap, current_node->key, current_node->value_type, current_node->value);

            next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
    }


    HM_LOG(LOG_LEVEL_DEBUG, "Freeing old content and swapping it's content with aux", hashmap);
    free(hashmap->list);

    hashmap->list = aux_hashmap->list;
    hashmap->capacity = aux_hashmap->capacity;

    free(aux_hashmap);
    HM_LOG(LOG_LEVEL_DEBUG, "Aux hashmap freed", hashmap);
    HM_LOG(LOG_LEVEL_DEBUG, "Hashmap [%p][ll: %p] resized to capacity %d", hashmap, hashmap->list, hashmap->capacity);

    return HM_SUCCESS;
}


/**
 * @brief Busca um elemento no hashmap
 *
 * @param hashmap Ponteiro para o hashmap
 * @param key Chave do elemento
 * @return int Caso encontre o elemento, retorna o valor. Caso contr�rio, retorna HM_NOT_FOUND. Em caso de erro, retorna HM_ERROR.
 */
int hm_search(hashmap_t *hashmap, void **value, ...) {
    va_list args;
    char *key = NULL;

    unsigned int bucket = 0;

    hashmap_t *current_hm = hashmap;
    node_t *node = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return HM_ERROR;
    }

    va_start(args, value);

    while (1) {
        // Retrieves the current key
        key = va_arg(args, char *);

        if (key == NULL) {
            break;
        }

        // Hashes the key
        if((bucket = hm_hash(current_hm, key)) == HM_ERROR) {
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
        } while(node != NULL);

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
 * @brief Insere um elemento no hashmap
 *
 * @param hashmap Ponteiro para o hashmap
 * @param key Chave do elemento
 * @param value Valor do elemento
 */
void hm_insert(hashmap_t *hashmap, char *key, node_value_t value_type, void* value) {
    int bucket = 0;
    int hash_code = 0;
    double current_load_factor = 0;

    void* new_node_val = NULL;
    char* new_node_key = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return;
    }

    if (key == NULL || key[0] == '\0') {
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Inserting key [%s] to hashmap [%p]", key, hashmap);

    current_load_factor = hm_get_load_factor(hashmap);
    if (current_load_factor == HM_ERROR) {
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Current load factor: %.2f", current_load_factor);

    if (current_load_factor >= HM_LOAD_FACTOR_THRESHOLD) {
        HM_LOG(LOG_LEVEL_DEBUG, "Load factor threshold reached. Resizing hashmap");
        if (hm_resize(hashmap, HM_RESIZE_FACTOR) == HM_ERROR) {
            return;
        }
    }

    hash_code = hm_hash(hashmap, key);
    if (hash_code == HM_ERROR) {
        return;
    }

    bucket = hash_code;

    HM_LOG(LOG_LEVEL_DEBUG, "Bucket [%d]", bucket);

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
    hashmap->size++;
}
