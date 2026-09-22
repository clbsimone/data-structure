#include "unity.h"
#include "hash_table.h"
#include <string.h>
#include <stdlib.h>

/* ------------- GLOBAL TEST DATA ------------- */

static HashTable *table = NULL;

/* Integers used across tests */
static int int_keys[110];
static int int_values[110];

/* Strings used across tests */
static const char *str_keys[] = {"apple", "banana", "cherry"};
static const char *str_values[] = {"red", "yellow", "dark red"};

/* ------------- HELPERS ------------- */

/* Integer compare function */
static int int_compare(const void *a, const void *b){
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia == ib) ? 0 : (ia < ib ? -1 : 1);
}

/* Integer hash function */
static unsigned long int_hash(const void *key){
    return (unsigned long)(*(const int *)key);
}

/* String compare function */
static int string_compare(const void *a, const void *b){
    return strcmp((const char *)a, (const char *)b);
}

/* String hash function (djb2) */
static unsigned long string_hash(const void *key){
    const unsigned char *str = (const unsigned char *)key;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++) != 0){
        hash = ((hash << 5) + hash) + (unsigned char)c;
    }
    return hash;
}

/* ------------- UNITY FIXTURES ------------- */

void setUp(void){
    /* Used only in tests that need a table */
    table = NULL;
}

void tearDown(void){
    if (table != NULL){
        hash_table_free(table);
        table = NULL;
    }
}

/* ------------- TESTS ------------- */

void test_create_with_null_functions_should_return_null(void){
    HashTable *t1 = hash_table_create(NULL, int_hash);
    HashTable *t2 = hash_table_create(int_compare, NULL);

    TEST_ASSERT_NULL(t1);
    TEST_ASSERT_NULL(t2);
}

void test_create_valid_should_return_empty_table(void){
    table = hash_table_create(int_compare, int_hash);
    TEST_ASSERT_NOT_NULL(table);
    TEST_ASSERT_EQUAL_INT(0, hash_table_size(table));
}

void test_put_and_get_single_element(void){
    table = hash_table_create(int_compare, int_hash);

    int_keys[0] = 5;
    int_values[0] = 100;

    hash_table_put(table, &int_keys[0], &int_values[0]);

    TEST_ASSERT_EQUAL_INT(1, hash_table_size(table));

    int *val = (int *)hash_table_get(table, &int_keys[0]);
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT(100, *val);
}

void test_update_existing_key_should_not_increase_size(void){
    table = hash_table_create(int_compare, int_hash);

    int_keys[0] = 10;
    int_values[0] = 1;
    int_values[1] = 2;

    hash_table_put(table, &int_keys[0], &int_values[0]);
    hash_table_put(table, &int_keys[0], &int_values[1]);

    TEST_ASSERT_EQUAL_INT(1, hash_table_size(table));

    int *val = (int *)hash_table_get(table, &int_keys[0]);
    TEST_ASSERT_EQUAL_INT(2, *val);
}

void test_contains_key(void){
    table = hash_table_create(int_compare, int_hash);

    int_keys[0] = 1;
    int_keys[1] = 2;

    hash_table_put(table, &int_keys[0], &int_keys[0]);

    TEST_ASSERT_TRUE(hash_table_contains_key(table, &int_keys[0]));
    TEST_ASSERT_FALSE(hash_table_contains_key(table, &int_keys[1]));
}

void test_remove_key(void){
    table = hash_table_create(int_compare, int_hash);

    int_keys[0] = 1;
    int_keys[1] = 2;
    int_keys[2] = 3;

    int_values[0] = 10;
    int_values[1] = 20;
    int_values[2] = 30;

    hash_table_put(table, &int_keys[0], &int_values[0]);
    hash_table_put(table, &int_keys[1], &int_values[1]);
    hash_table_put(table, &int_keys[2], &int_values[2]);

    TEST_ASSERT_EQUAL_INT(3, hash_table_size(table));

    hash_table_remove(table, &int_keys[1]);

    TEST_ASSERT_EQUAL_INT(2, hash_table_size(table));
    TEST_ASSERT_NULL(hash_table_get(table, &int_keys[1]));
}

void test_keyset(void){
    table = hash_table_create(int_compare, int_hash);

    for (int i = 0; i < 5; i++)
    {
        int_keys[i] = i * 10;
        int_values[i] = i;
        hash_table_put(table, &int_keys[i], &int_values[i]);
    }

    void **keys = hash_table_keyset(table);
    TEST_ASSERT_NOT_NULL(keys);

    int found = 0;
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            int *k = (int *)keys[j];
            if (*k == int_keys[i])
            {
                found++;
                break;
            }
        }
    }

    TEST_ASSERT_EQUAL_INT(5, found);
    free(keys);
}

void test_resize_and_find_all_elements(void){
    table = hash_table_create(int_compare, int_hash);

    for (int i = 0; i < 100; i++)
    {
        int_keys[i] = i;
        int_values[i] = i * 10;
        hash_table_put(table, &int_keys[i], &int_values[i]);
    }

    TEST_ASSERT_EQUAL_INT(100, hash_table_size(table));

    for (int i = 0; i < 100; i++)
    {
        int *val = (int *)hash_table_get(table, &int_keys[i]);
        TEST_ASSERT_NOT_NULL(val);
        TEST_ASSERT_EQUAL_INT(int_values[i], *val);
    }
}

void test_string_keys(void){
    table = hash_table_create(string_compare, string_hash);

    hash_table_put(table, str_keys[0], str_values[0]);
    hash_table_put(table, str_keys[1], str_values[1]);
    hash_table_put(table, str_keys[2], str_values[2]);

    TEST_ASSERT_EQUAL_INT(3, hash_table_size(table));

    const char *v = (const char *)hash_table_get(table, str_keys[1]);
    TEST_ASSERT_EQUAL_STRING("yellow", v);
}

/* ------------- main() ------------- */

static void test_null_value_membership(void) {
    HashTable *table = hash_table_create(int_compare, int_hash);
    int key = 7;
    TEST_ASSERT_NOT_NULL(table);
    hash_table_put(table, &key, NULL);
    TEST_ASSERT_TRUE(hash_table_contains_key(table, &key));
    TEST_ASSERT_NULL(hash_table_get(table, &key));
    TEST_ASSERT_EQUAL_INT(1, hash_table_size(table));
    hash_table_remove(table, &key);
    TEST_ASSERT_FALSE(hash_table_contains_key(table, &key));
    hash_table_free(table);
}

int main(void){
    UNITY_BEGIN();
    RUN_TEST(test_null_value_membership);

    RUN_TEST(test_create_with_null_functions_should_return_null);
    RUN_TEST(test_create_valid_should_return_empty_table);
    RUN_TEST(test_put_and_get_single_element);
    RUN_TEST(test_update_existing_key_should_not_increase_size);
    RUN_TEST(test_contains_key);
    RUN_TEST(test_remove_key);
    RUN_TEST(test_keyset);
    RUN_TEST(test_resize_and_find_all_elements);
    RUN_TEST(test_string_keys);

    return UNITY_END();
}
