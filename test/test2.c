#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/map.h"
#include "../src/log.h"

int main() {

    hashmap_t *hm = NULL;

    hashmap_t *aux_hm = NULL;

    void *val = NULL;

    if ((hm = hm_create_default()) == NULL) {
        printf("Erro ao criar hashmap\n");
        return 1;
    }

    HM_LOG(LOG_LEVEL_DEBUG, "INSERTING key1->key2->key3->'value1'");
    hm_insert(hm, HM_VALUE_STR, "value1", "key1", "key2", "key3", NULL);

    HM_LOG(LOG_LEVEL_DEBUG, "INSERTING key1->key2->key4->'value2'");
    hm_insert(hm, HM_VALUE_STR, "value2", "key1", "key2", "key4", NULL);

    HM_LOG(LOG_LEVEL_DEBUG, "INSERTING key1->key2->key4->'value3'");
    hm_insert(hm, HM_VALUE_STR, "value3", "key1", "key2", "key4", NULL);

    HM_LOG(LOG_LEVEL_DEBUG, "INSERTING key1->key2->key4->key5[map]");
    hm_insert(hm, HM_VALUE_MAP, NULL, "key1", "key2", "key5", NULL);

    if (hm_search(hm, &val, "key1", "key2", "asdasd", NULL) == HM_SUCCESS) {
        HM_LOG(LOG_LEVEL_DEBUG, "Achou 1");
    }else {
        HM_LOG(LOG_LEVEL_DEBUG, "Nao achou 1");
    }

    if (hm_search(hm, &val, "key1", "key2", "key6", NULL) == HM_SUCCESS) {
        HM_LOG(LOG_LEVEL_DEBUG, "Achou 2, val: [%s]", (char *)val);
    }else {
        HM_LOG(LOG_LEVEL_DEBUG, "Nao achou 2");
    }

    if (hm_search(hm, &val, "key1", "key2", "key4", NULL) == HM_SUCCESS) {
        HM_LOG(LOG_LEVEL_DEBUG, "Achou 3, val: [%s]", (char *)val);
        //assert(strcmp(val, "value3") == 0);
    }else {
        HM_LOG(LOG_LEVEL_DEBUG, "Nao achou 3");
    }

    if (hm_search(hm, &val, "key1", "key2", "key3", NULL) == HM_SUCCESS) {
        HM_LOG(LOG_LEVEL_DEBUG, "Achou 4, val: [%s]", (char *)val);
        assert(strcmp(val, "value1") == 0);
    }else {
        HM_LOG(LOG_LEVEL_DEBUG, "Nao achou 4");
    }

    HM_LOG(LOG_LEVEL_DEBUG, "hm_search: k:[%s][%s][%s] v:[%s]", "key1", "key2", "key3", val);
    hm_free((void **)&hm);
}
