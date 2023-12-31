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


node_t* hm_node_new(void) {
    node_t *node = malloc(sizeof(node_t));

    if (!node) {
        return NULL;
    }

    node->key = NULL;
    node->value = NULL;
    node->value_type = HM_VALUE_MAP;
    node->next = NULL;

    return node;
}


node_t* hm_node_create(char *key, node_value_t value_type, void* value, void* next) {
    node_t *node = hm_node_new();

    if (!node) {
        return NULL;
    }

    node->key = key;
    node->value = value;
    node->value_type = value_type;
    node->next = next;

    return node;
}


void hm_node_free(void **node_p) {
    if (node_p == NULL || *node_p == NULL) {
        return;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "Freeing node [%p][%p]", *node_p, node_p);

    node_t *node = *node_p;

    HM_LOG(LOG_LEVEL_DEBUG, "Freeing node key [%s][%p]", node->key, &node->key);
    if (node->key) {
        free(node->key);
        node->key = NULL;
    }

    if (node->value) {
        switch(node->value_type) {
            case HM_VALUE_STR:
                free(node->value);
                break;
            case HM_VALUE_MAP:
                hm_free((void **)&node->value);
                break;
        }
        node->value = NULL;
    }

    free(node);
    *node_p = NULL;
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
    if (hashmap_p == NULL || *hashmap_p == NULL) {
        return;
    }

    hashmap_t *hashmap = *hashmap_p;

    for (int i = 0; i < hashmap->capacity; i++) {
        printf("Freeing bucket [%d][%p][%p]\n", i, *hashmap_p, hashmap_p);

        node_t *current_node = hashmap->list[i];
        while (current_node != NULL) {
            node_t *next_node = current_node->next;
            HM_LOG(LOG_LEVEL_DEBUG, "Freeing node [c:%p][n:%p]", current_node, next_node);
            hm_node_free((void **)&current_node);
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
    node_t *node = NULL;

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


    if ((node = hm_node_create(
        strdup(key),
        value_type,
        value,
        hashmap->list[bucket]
    )) == NULL) {
        return;
    }

    hashmap->list[bucket] = node;
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
void hm_insert(hashmap_t *hashmap,  node_value_t value_type, void* value, ...) {
    va_list args;

    int bucket = 0;
    double current_load_factor = 0;

    node_t *node = NULL;
    hashmap_t *aux_hm = NULL;
    hashmap_t *current_hm = NULL;

    char* key = NULL;
    char* next_key = NULL;

    char* node_key = NULL;
    char* node_val = NULL;

    if (hashmap == NULL || hashmap->list == NULL) {
        return;
    }

    current_hm = hashmap;

    va_start(args, value);
    key = va_arg(args, char *);

    while(key != NULL) {
        node_value_t node_type = HM_VALUE_MAP;

        next_key = va_arg(args, char *);

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
            if (
                next_key != NULL
                && ((node_val = hm_create_default()) == NULL)
            ){
                va_end(args);
                free(node_key);
                return;
            }
        } else {
            // Key found inside current hashmap
            // If it's not the last one, checks if its a map
            if (next_key != NULL) {
                if (node->value_type != HM_VALUE_MAP) {
                    HM_LOG(LOG_LEVEL_ERROR, "Key [%s] is not a hashmap", key);
                    va_end(args);
                    return;
                }
            } else {
                // If it's the last one, checks if it's a string
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

        if (
            node_key
            && node_val
        ){

            node = hm_node_create(
                node_key,
                node_type,
                node_val,
                current_hm->list[bucket]
            );

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
