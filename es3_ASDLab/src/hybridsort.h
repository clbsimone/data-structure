#ifndef HYBRIDSORT_H_SIMONE_BASSO
#define HYBRIDSORT_H_SIMONE_BASSO

#include <stddef.h>

/**
 * @brief Sort an array using a hybrid quicksort/selection sort algorithm.
 *
 * This function sorts an array of @p nitems elements, each of size @p size,
 * in ascending order according to the comparison function @p compar.
 *
 * The algorithm is:
 * - A quicksort partitioning strategy with median-of-five pivot selection (median-of-three for small ranges).
 * - When a subarray has fewer than @p k elements, selection sort is used
 *   instead of further partitioning.
 *
 * An empty array is a no-op, including when base is NULL.
 * The function performs basic validation for nonempty arrays:
 * - If @p base is NULL, an error message is printed to stderr and the
 *   function returns without sorting.
 * - If @p size is 0, an error message is printed to stderr and the
 *   function returns without sorting.
 * - If @p compar is NULL, an error message is printed to stderr and the
 *   function returns without sorting.
 *
 * @param base   Pointer to the first element of the array to be sorted.
 * @param nitems Number of elements in the array.
 * @param size   Size in bytes of each element.
 * @param k      Threshold for switching from quicksort to selection sort.
 *               For subarrays with fewer than @p k elements, selection
 *               sort is used.
 * @param compar Pointer to a comparison function which takes two
 *               @c const void* arguments (pointers to elements) and returns:
 *               - A negative value if the first argument is considered
 *                 less than the second.
 *               - Zero if they are considered equal.
 *               - A positive value if the first argument is considered
 *                 greater than the second.
 */
void hybridsort(void *base,
                size_t nitems,
                size_t size,
                size_t k,
                int (*compar)(const void *, const void *));

#ifdef TESTING
/* Internal helpers exposed only in test builds. */

void memswap(void *a, void *b, size_t size);

void selectionsort(void *base,
                   size_t nitems,
                   size_t size,
                   int (*compar)(const void *, const void *));

size_t median_of_three(void *base,
                       size_t size,
                       size_t lo,
                       size_t hi,
                       int (*compar)(const void *, const void *));


size_t partition(unsigned char *base,
                 size_t size,
                 size_t lo,
                 size_t hi,
                 int (*compar)(const void *, const void *),
                 void *pivot_buf);
#endif /* TESTING */

#endif /* HYBRIDSORT_H_SIMONE_BASSO */
