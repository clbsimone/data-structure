#include "unity.h"
#include "priority_queue.h"

#include <stdlib.h>

/* ----------------- Helper types and functions for tests ----------------- */

/*
 * We use int* as elements stored in the priority queue.
 * compare_int compares the integer values.
 */
static int compare_int(const void *a, const void *b){
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    if (ia > ib)
        return 1;
    if (ia < ib)
        return -1;
    return 0;
}

/*
 * Hash function for int* keys.
 * We hash based on the integer value.
 */
static unsigned long hash_int(const void *p){
    long v = *(const int *)p;
    /* simple hash: mix bits of v */
    return (unsigned long)(v * 2654435761u);
}

/*
 * Utility to allocate a new int on the heap and initialize it.
 * We use this to have distinct pointers for each element.
 */
static int *make_int(int value){
    int *ptr = malloc(sizeof(int));
    TEST_ASSERT_NOT_NULL(ptr); // if this fails, test should stop
    *ptr = value;
    return ptr;
}

/* We'll keep track of allocated ints to free them at the end. */
#define MAX_ALLOCATED 1024
static int *allocated[MAX_ALLOCATED];
static int allocated_count;

/* Register an allocated int* so we can free it later. */
static int *reg_int(int value){
    TEST_ASSERT_MESSAGE(allocated_count < MAX_ALLOCATED, "Too many allocated ints in test");
    int *p = make_int(value);
    allocated[allocated_count++] = p;
    return p;
}

/* Free all ints allocated via reg_int. */
static void free_allocated_ints(void){
    for (int i = 0; i < allocated_count; i++)
    {
        free(allocated[i]);
    }
    allocated_count = 0;
}


/* ----------------- setUp / tearDown for Unity ----------------- */

void setUp(void){
    /* Called before each test */
    allocated_count = 0;
}

void tearDown(void){
    /* Called after each test */
    free_allocated_ints();
}

/* ----------------- TEST ----------------- */

/* Test creation with valid and invalid arguments */
static void test_create_queue(void){
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));

    priority_queue_free(pq);

    /* invalid args */
    pq = priority_queue_create(NULL, hash_int);
    TEST_ASSERT_NULL(pq);

    pq = priority_queue_create(compare_int, NULL);
    TEST_ASSERT_NULL(pq);
}

/* Basic push + top + pop on a small queue */
static void test_push_top_pop_simple(void){
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    int *a = reg_int(10);
    int *b = reg_int(5);
    int *c = reg_int(20);

    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));

    /* push elements */
    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, a));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, b));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, c));

    TEST_ASSERT_EQUAL_INT(3, priority_queue_size(pq));

    /* top should be the max: 20 */
    int *top = (int *)priority_queue_top(pq);
    TEST_ASSERT_NOT_NULL(top);
    TEST_ASSERT_EQUAL_INT(20, *top);

    /* pop: remove 20 */
    priority_queue_pop(pq);
    TEST_ASSERT_EQUAL_INT(2, priority_queue_size(pq));

    top = (int *)priority_queue_top(pq);
    TEST_ASSERT_NOT_NULL(top);
    TEST_ASSERT_TRUE(*top == 10 || *top == 5); // depending on heap shape

    priority_queue_free(pq);
}

/* Test that pushing duplicates returns 0 and does not change size */
static void test_push_duplicates(void){
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    int *x = reg_int(42);
    int *y = x; // same pointer -> same element

    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, x));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_size(pq));

    /* pushing the same pointer again should not insert */
    TEST_ASSERT_EQUAL_INT(0, priority_queue_push(pq, y));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_size(pq));

    priority_queue_free(pq);
}

/* Test contains + remove on a small set of elements */
static void test_contains_and_remove(void){
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    int *a = reg_int(10);
    int *b = reg_int(50);
    int *c = reg_int(30);

    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, a));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, b));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, c));

    TEST_ASSERT_EQUAL_INT(3, priority_queue_size(pq));

    /* contains */
    TEST_ASSERT_EQUAL_INT(1, priority_queue_contains(pq, b));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_contains(pq, c));

    int not_in = 99;
    TEST_ASSERT_EQUAL_INT(0, priority_queue_contains(pq, &not_in));

    /* remove middle element (not root, not last) */
    TEST_ASSERT_EQUAL_INT(1, priority_queue_remove(pq, c));
    TEST_ASSERT_EQUAL_INT(2, priority_queue_size(pq));
    TEST_ASSERT_EQUAL_INT(0, priority_queue_contains(pq, c));

    /* remove root (max) */
    TEST_ASSERT_EQUAL_INT(1, priority_queue_remove(pq, b));
    TEST_ASSERT_EQUAL_INT(1, priority_queue_size(pq));
    TEST_ASSERT_EQUAL_INT(0, priority_queue_contains(pq, b));

    /* remove last remaining */
    TEST_ASSERT_EQUAL_INT(1, priority_queue_remove(pq, a));
    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));

    /* removing again should return 0 (not found) */
    TEST_ASSERT_EQUAL_INT(0, priority_queue_remove(pq, a));

    priority_queue_free(pq);
}

/* Test behavior of top/pop on an empty queue */
static void test_empty_queue_behaviour(void){
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));
    TEST_ASSERT_NULL(priority_queue_top(pq));

    /* pop on empty should not crash */
    priority_queue_pop(pq);
    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));

    priority_queue_free(pq);
}

/* Stress-like test with many elements to check basic stability */
static void test_many_elements(void){
    const int N = 1000;
    PriorityQueue *pq = priority_queue_create(compare_int, hash_int);
    TEST_ASSERT_NOT_NULL(pq);

    for (int i = 0; i < N; i++){
        int *p = reg_int(i);
        TEST_ASSERT_EQUAL_INT(1, priority_queue_push(pq, p));
    }

    TEST_ASSERT_EQUAL_INT(N, priority_queue_size(pq));

    /* Max element should be N-1 (since compare makes it a max-heap) */
    int *top = (int *)priority_queue_top(pq);
    TEST_ASSERT_NOT_NULL(top);
    TEST_ASSERT_EQUAL_INT(N - 1, *top);

    /* Pop all elements, they should come out in non-increasing order */
    int last_value = N + 1;
    while (priority_queue_size(pq) > 0){
        int *t = (int *)priority_queue_top(pq);
        TEST_ASSERT_NOT_NULL(t);
        TEST_ASSERT_TRUE(*t <= last_value);
        last_value = *t;
        priority_queue_pop(pq);
    }

    TEST_ASSERT_EQUAL_INT(0, priority_queue_size(pq));

    priority_queue_free(pq);
}

/* ----------------- main() ----------------- */

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_create_queue);
    RUN_TEST(test_push_top_pop_simple);
    RUN_TEST(test_push_duplicates);
    RUN_TEST(test_contains_and_remove);
    RUN_TEST(test_empty_queue_behaviour);
    RUN_TEST(test_many_elements);

    return UNITY_END();
}
