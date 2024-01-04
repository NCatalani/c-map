#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <cmap/log.h>
#include <cmap/map.h>

void fill_test_map_struct(hashmap_t* hm)
{

    void* val = NULL;
    int search_result = -1;

    HM_LOG(LOG_LEVEL_INFO, "Testing HM insert");

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO");
    hm_insert(hm, HM_VALUE_MAP, NULL, "POSPAGO", NULL);

    HM_LOG(LOG_LEVEL_INFO, "Inserting: PREPAGO");
    hm_insert(hm, HM_VALUE_MAP, NULL, "PREPAGO", NULL);

    HM_LOG(LOG_LEVEL_INFO, "Inserting: PREPAGO->MENSAL->BBS->\"foo\"");
    hm_insert(hm, HM_VALUE_STR, "foo", "PREPAGO", "MENSAL", "BBS", NULL);

    HM_LOG(LOG_LEVEL_INFO, "Inserting: PREPAGO->MENSAL->BES->\"bar\"");
    hm_insert(hm, HM_VALUE_STR, "bar", "PREPAGO", "MENSAL", "BES", NULL);

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->TRIMESTRAL->VSA");
    hm_insert(hm, HM_VALUE_MAP, NULL, "POSPAGO", "TRIMESTRAL", "VSA", NULL);

    HM_LOG(LOG_LEVEL_INFO, "HM: %s", hm_serialize(hm));

    search_result = hm_search(hm, &val, "PREPAGO", NULL);
    assert(search_result == HM_SUCCESS);

    search_result = hm_search(hm, &val, "POSPAGO", NULL);
    assert(search_result == HM_SUCCESS);

    search_result = hm_search(hm, &val, "POSPAGO", "TRIMESTRAL", NULL);
    assert(search_result == HM_SUCCESS);

    search_result = hm_search(hm, &val, "POSPAGO", "TRIMESTRAL", "VSA", NULL);
    assert(search_result == HM_SUCCESS);

    search_result = hm_search(hm, &val, "POSPAGO", "TRIMESTRAL", "CRD", NULL);
    assert(search_result == HM_NOT_FOUND);

    search_result = hm_search(hm, &val, "PREPAGO", "MENSAL", "BES", NULL);
    assert(search_result == HM_SUCCESS);
    assert(!strcmp(val, "bar"));
}

int main()
{

    hashmap_t* hm = NULL;

    if ((hm = hm_create_default()) == NULL) {
        printf("Erro ao criar hashmap\n");
        return 1;
    }

    fill_test_map_struct(hm);

    hm_free((void**)&hm);
}
