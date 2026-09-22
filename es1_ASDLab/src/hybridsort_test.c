#include "unity.h"
#include <string.h>
#include <stdlib.h>

#include "hybridsort.h"

/* ---------------- Comparators ---------------- */

static int int_cmp(const void *a, const void *b){
    int aa = *(const int *)a;
    int bb = *(const int *)b;
    if (aa < bb)
        return -1;
    if (aa > bb)
        return 1;
    return 0;
}

static int float_cmp(const void *a, const void *b){
    float aa = *(const float *)a;
    float bb = *(const float *)b;
    if (aa < bb)
        return -1;
    if (aa > bb)
        return 1;
    return 0;
}

static int char_cmp(const void *a, const void *b){
    char aa = *(const char *)a;
    char bb = *(const char *)b;
    if (aa < bb)
        return -1;
    if (aa > bb)
        return 1;
    return 0;
}

static int cstring_cmp(const void *a, const void *b){
    const char *const *sa = (const char *const *)a;
    const char *const *sb = (const char *const *)b;
    return strcmp(*sa, *sb);
}

#define MAX_ELEMS 16

static int arr_int[MAX_ELEMS];
static int exp_int[MAX_ELEMS];
static size_t n_int;

static float arr_float[MAX_ELEMS];
static float exp_float[MAX_ELEMS];
static size_t n_float;

static char arr_char[MAX_ELEMS];
static char exp_char[MAX_ELEMS];
static size_t n_char;

static const char *arr_str[MAX_ELEMS];
static const char *exp_str[MAX_ELEMS];
static size_t n_str;

/* ---------------- Unity setUp / tearDown ---------------- */

void setUp(void){
    /* Reset all buffers before each test to ensure independence */
    memset(arr_int, 0, sizeof(arr_int));
    memset(exp_int, 0, sizeof(exp_int));
    n_int = 0;

    memset(arr_float, 0, sizeof(arr_float));
    memset(exp_float, 0, sizeof(exp_float));
    n_float = 0;

    memset(arr_char, 0, sizeof(arr_char));
    memset(exp_char, 0, sizeof(exp_char));
    n_char = 0;

    memset(arr_str, 0, sizeof(arr_str));
    memset(exp_str, 0, sizeof(exp_str));
    n_str = 0;
}

void tearDown(void) {}

/* ---------------- Tests for helper functions ---------------- */

static void test_memswap_integers(void){
    int a = 10;
    int b = 20;

    memswap(&a, &b, sizeof(int));

    TEST_ASSERT_EQUAL_INT(20, a);
    TEST_ASSERT_EQUAL_INT(10, b);
}

static void test_memswap_same_pointer(void){
    int a = 42;
    memswap(&a, &a, sizeof(int));
    TEST_ASSERT_EQUAL_INT(42, a);
}

static void test_selectionsort_integers_basic(void){
    int temp[]     = {5, 2, 9, 1, 5};
    int expected[] = {1, 2, 5, 5, 9};
    
    n_int = 5;

    memcpy(arr_int, temp, n_int * sizeof(int));
    memcpy(exp_int, expected, n_int * sizeof(int));

    selectionsort(arr_int, n_int, sizeof(int), int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_selectionsort_already_sorted(void){
    int temp[]     = {1, 2, 3, 4, 5};
    int expected[] = {1, 2, 3, 4, 5};

    n_int = 5;
    memcpy(arr_int, temp, n_int * sizeof(int));
    memcpy(exp_int, expected, n_int * sizeof(int));

    selectionsort(arr_int, n_int, sizeof(int), int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_median_of_three_basic(void){
    int temp[] = {10, 30, 20};
    
    n_int = 3;
    memcpy(arr_int, temp, n_int * sizeof(int));

    size_t idx = median_of_three(arr_int, sizeof(int), 0, 2, int_cmp);

    TEST_ASSERT_EQUAL_UINT(2, idx);
}

static void test_median_of_three_larger_array(void){
    int temp[] = {40, 10, 30, 20, 50};
    /* values = 40, 30, 50 -> median = 40 at index 0 */

    n_int = 5;
    memcpy(arr_int, temp, n_int * sizeof(int));

    size_t idx = median_of_three(arr_int, sizeof(int), 0, 4, int_cmp);

    TEST_ASSERT_EQUAL_UINT(0, idx);
}

static void test_partition_basic(void){
    int temp[] = {9, 3, 4, 8, 2, 7};
    
    n_int = 6;
    memcpy(arr_int, temp, n_int * sizeof(int));

    unsigned char *base = (unsigned char *)arr_int;
    int pivot_value = 4;
    int pivot_buf;
    memcpy(&pivot_buf, &pivot_value, sizeof(int));

    size_t j = partition(base, sizeof(int), 0, n_int - 1, int_cmp, &pivot_buf);

    for (size_t i = 0; i <= j; i++)
    {
        TEST_ASSERT_TRUE(arr_int[i] <= pivot_value);
    }
    for (size_t i = j + 1; i < n_int; i++)
    {
        TEST_ASSERT_TRUE(arr_int[i] >= pivot_value);
    }
}

/* ---------------- Tests for hybridsort on various types ---------------- */

static void test_hybridsort_integers_basic(void){
    int temp[]     = {5, 2, 9, 1, 5};
    int expected[] = {1, 2, 5, 5, 9};

    n_int = 5;
    memcpy(arr_int, temp, n_int * sizeof(int));
    memcpy(exp_int, expected, n_int * sizeof(int));

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_hybridsort_floats_basic(void)
{
    float temp[]     = {3.5f, -1.0f, 2.2f, 2.2f, 0.0f};
    float expected[] = {-1.0f, 0.0f, 2.2f, 2.2f, 3.5f};

    n_float = 5;
    memcpy(arr_float, temp, n_float * sizeof(float));
    memcpy(exp_float, expected, n_float * sizeof(float));

    hybridsort(arr_float, n_float, sizeof(float), 3, float_cmp);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(exp_float, arr_float, n_float);
}

static void test_hybridsort_chars_basic(void){
    char temp[]     = {'d', 'a', 'c', 'b'};
    char expected[] = {'a', 'b', 'c', 'd'};

    n_char = 4;
    memcpy(arr_char, temp, n_char * sizeof(char));
    memcpy(exp_char, expected, n_char * sizeof(char));

    hybridsort(arr_char, n_char, sizeof(char), 2, char_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_char, arr_char, n_char);
}

static void test_hybridsort_strings_basic(void)
{
    const char *temp[]     = {"banana", "apple", "pear", "apple"};
    const char *expected[] = {"apple", "apple", "banana", "pear"};

    n_str = 4;
    memcpy(arr_str, temp, n_str * sizeof(char *));
    memcpy(exp_str, expected, n_str * sizeof(char *));

    hybridsort(arr_str, n_str, sizeof(char *), 3, cstring_cmp);

    TEST_ASSERT_EQUAL_STRING_ARRAY(exp_str, arr_str, n_str);
}

/* ---------------- Empty and small arrays ---------------- */

static void test_hybridsort_empty_array(void){
    /* Must not crash. Array buffer is valid but nitems=0. */
    n_int = 0;
    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);
    TEST_ASSERT_TRUE(1);
}

static void test_hybridsort_single_element(void){
    int temp[] = {42};

    n_int = 1;
    memcpy(arr_int, temp, sizeof(int));

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT(42, arr_int[0]);
}

static void test_hybridsort_two_elements(void){
    int temp[]     = {2, 1};
    int expected[] = {1, 2};

    n_int = 2;
    memcpy(arr_int, temp, sizeof(int) * 2);
    memcpy(exp_int, expected, sizeof(int) * 2);

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

/* ---------------- Different k values ---------------- */

static void test_hybridsort_k_small(void){
    int temp[]     = {5, 1, 4, 2, 8, 0, -1};
    int expected[] = {-1, 0, 1, 2, 4, 5, 8};

    n_int = 7;
    memcpy(arr_int, temp, sizeof(int) * n_int);
    memcpy(exp_int, expected, sizeof(int) * n_int);

    hybridsort(arr_int, n_int, sizeof(int), 1, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_hybridsort_k_medium(void){
    int temp[]     = {10, 7, 8, 9, 1, 5};
    int expected[] = {1, 5, 7, 8, 9, 10};

    n_int = 6;
    memcpy(arr_int, temp, sizeof(int) * n_int);
    memcpy(exp_int, expected, sizeof(int) * n_int);

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_hybridsort_k_larger_than_array(void){
    int temp[]     = {3, 1, 2};
    int expected[] = {1, 2, 3};

    n_int = 3;
    memcpy(arr_int, temp, sizeof(int) * 3);
    memcpy(exp_int, expected, sizeof(int) * 3);

    hybridsort(arr_int, n_int, sizeof(int), 10, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

/* ---------------- Special edge cases ---------------- */

static void test_hybridsort_already_sorted(void){
    int temp[]     = {1, 2, 3, 4, 5};
    int expected[] = {1, 2, 3, 4, 5};

    n_int = 5;
    memcpy(arr_int, temp, sizeof(int) * 5);
    memcpy(exp_int, expected, sizeof(int) * 5);

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_hybridsort_reverse_sorted(void){
    int temp[]     = {5, 4, 3, 2, 1};
    int expected[] = {1, 2, 3, 4, 5};

    n_int = 5;
    memcpy(arr_int, temp, sizeof(int) * 5);
    memcpy(exp_int, expected, sizeof(int) * 5);

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

static void test_hybridsort_with_duplicates(void){
    int temp[]     = {2, 3, 2, 1, 3, 2, 1};
    int expected[] = {1, 1, 2, 2, 2, 3, 3};

    n_int = 7;
    memcpy(arr_int, temp, sizeof(int) * 7);
    memcpy(exp_int, expected, sizeof(int) * 7);

    hybridsort(arr_int, n_int, sizeof(int), 3, int_cmp);

    TEST_ASSERT_EQUAL_INT_ARRAY(exp_int, arr_int, n_int);
}

/* ---------------- main() ---------------- */

int main(void){
    UNITY_BEGIN();

    /* Helper function tests */
    RUN_TEST(test_memswap_integers);
    RUN_TEST(test_memswap_same_pointer);
    RUN_TEST(test_selectionsort_integers_basic);
    RUN_TEST(test_selectionsort_already_sorted);
    RUN_TEST(test_median_of_three_basic);
    RUN_TEST(test_median_of_three_larger_array);
    RUN_TEST(test_partition_basic);

    /* Hybridsort on various types */
    RUN_TEST(test_hybridsort_integers_basic);
    RUN_TEST(test_hybridsort_floats_basic);
    RUN_TEST(test_hybridsort_chars_basic);
    RUN_TEST(test_hybridsort_strings_basic);

    /* Empty and small arrays */
    RUN_TEST(test_hybridsort_empty_array);
    RUN_TEST(test_hybridsort_single_element);
    RUN_TEST(test_hybridsort_two_elements);

    /* k variations */
    RUN_TEST(test_hybridsort_k_small);
    RUN_TEST(test_hybridsort_k_medium);
    RUN_TEST(test_hybridsort_k_larger_than_array);

    /* Edge cases */
    RUN_TEST(test_hybridsort_already_sorted);
    RUN_TEST(test_hybridsort_reverse_sorted);
    RUN_TEST(test_hybridsort_with_duplicates);

    return UNITY_END();
}
