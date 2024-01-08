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

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->ANUAL");
    list_t* aux = hm_list_create_default();

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->ANUAL->[CRD]");
    hm_list_append_str(aux, "CRD");

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->ANUAL->[VSA]");
    hm_list_append_str(aux, "VSA");

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->ANUAL->[BBS]");
    hm_list_append_str(aux, "BBS");

    HM_LOG(LOG_LEVEL_INFO, "Inserting: POSPAGO->ANUAL->[BES]");
    hm_list_append_str(aux, "BES");
    hm_insert(hm, HM_VALUE_LIST, aux, "POSPAGO", "ANUAL", NULL);

    // HM_LOG(LOG_LEVEL_INFO, "HM: %s", hm_serialize(hm));

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

    search_result = hm_search(hm, &val, "POSPAGO", "ANUAL", NULL);
    assert(search_result == HM_SUCCESS);

    assert(hm_list_contains(val, "CRD") == HM_SUCCESS);
    assert(hm_list_contains(val, "VSA") == HM_SUCCESS);
    assert(hm_list_contains(val, "KSA") == HM_NOT_FOUND);
    assert(((list_t*)val)->size == 4);
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
