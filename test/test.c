#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/map.h"
#include "../src/log.h"

#define MAX_TEST_INSERT_KEYS 1
#define MAX_SEARCH_INDEX 10
#define MAX_SUBMAPS 10
#define MAX_TEST_SUBMAP_INSERT_KEYS 10
#define SEARCH_KEYS 0

int main() {
    int i, j;

    hashmap_t *hm = NULL;
    hashmap_t *sub_hm = NULL;

    char key_buffer[128] = {'\0'};
    char submap_buffer[128] = {'\0'};
    char value_buffer[128] = {'\0'};

    clock_t outer_end, outer_start, start, end, start_p, end_p;
    double time_spent = 0.0;
    div_t divresult;

    void *val, *val2 = NULL;


    divresult = div(-10, 3);
    HM_LOG(LOG_LEVEL_DEBUG, "divresult: -10/3 -- q:%d | r:%d -- mod: %d", divresult.quot, divresult.rem, -10%3);

    //printf("[%s][%s](%d) - Criando hashmap root\n", __FILE__, __FUNCTION__, __LINE__);
    HM_LOG(LOG_LEVEL_INFO, "Criando hashmap root");
    HM_LOG(LOG_LEVEL_DEBUG, "Criando hashmap root");

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
    //printf("Time taken to insert: %.2f ms [keys: %d | rate: %.2f ins/s]\n", time_spent, MAX_TEST_INSERT_KEYS, MAX_TEST_INSERT_KEYS/time_spent*1000.0);

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
    //printf("Time taken to search: %.2f ms [keys: %d | rate: %.2f lkps/s]\n", time_spent, MAX_TEST_INSERT_KEYS, MAX_TEST_INSERT_KEYS/time_spent*1000.0);

    outer_end = clock();

    time_spent = (double)(outer_end - outer_start) / CLOCKS_PER_SEC * 1000.0;

    //printf("Time taken in total: %.2f ms\n", time_spent);

    //printf("[%s][%s](%d) - Criando submaps\n", __FILE__, __FUNCTION__, __LINE__);
    start = clock();
    // Insert some maps
    for (i = 0; i < MAX_SUBMAPS; i++) {
        sprintf(key_buffer, "submap_%d", i);

        sub_hm = hm_create_default();
        hm_insert(hm, key_buffer, HM_VALUE_MAP, sub_hm);

        for(j = 0; j < MAX_TEST_SUBMAP_INSERT_KEYS; j++) {
            sprintf(key_buffer, "key_%d", j);
            sprintf(value_buffer, "value_%d", j);

            hm_insert(sub_hm, key_buffer, HM_VALUE_STR, value_buffer);
        }

    }
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    //printf(
    //    "Time taken to insert: %.2f ms [sbmps: %d | sbkys: %d | rate: %.2f ins/s]\n",
    //    time_spent,
    //    MAX_SUBMAPS,
    //    MAX_TEST_SUBMAP_INSERT_KEYS,
    //    MAX_TEST_INSERT_KEYS/time_spent*1000.0
    //);

    start = clock();
    // Get random keys from submaps
    for (i=0; i < MAX_SUBMAPS; i++) {
        sprintf(submap_buffer, "submap_%d", i);
        if (hm_search(hm, submap_buffer, &val) != HM_SUCCESS) {
            printf("[%s][%s](%d) - hm_search: k:[%s] v:[%s]\n", __FILE__, __FUNCTION__, __LINE__, submap_buffer, "null");
            continue;
        }

        sub_hm = (hashmap_t*)val;

        sprintf(key_buffer, "key_%d", rand() % (MAX_TEST_SUBMAP_INSERT_KEYS*10));
        if (hm_search(sub_hm, key_buffer, &val2) != HM_SUCCESS) {
            //printf("[%s][%s](%d) - hm_search: subhm:[%s] k:[%s] v:[%s]\n", __FILE__, __FUNCTION__, __LINE__, submap_buffer, key_buffer, "null");
            continue;
        }

        //printf(
        //    "[%s][%s](%d) - hm_search: sm_k:[%s] k:[%s] v:[%s]\n",
        //    __FILE__,
        //    __FUNCTION__,
        //    __LINE__,
        //    submap_buffer,
        //    key_buffer,
        //    (char*)val2
        //);
    }

    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    //printf(
    //    "Time taken to search: %.2f ms [submps: %d | subkys: %d | rate: %.2f lkps/s]\n",
    //    time_spent,
    //    MAX_SUBMAPS,
    //    MAX_TEST_INSERT_KEYS,
    //    MAX_TEST_INSERT_KEYS/time_spent*1000.0
    //);

    hm_free((void **)&hm);
}
