#include "hybridsort.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef TESTING
    #define STATIC
#else
    #define STATIC static
#endif

#define FIVE 5

STATIC void memswap(void *a, void *b, size_t size);
STATIC void selectionsort(void *base,
                          size_t nitems,
                          size_t size,
                          int (*compar)(const void *, const void *));
STATIC size_t median_of_three(void *base,
                              size_t size,
                              size_t lo,
                              size_t hi,
                              int (*compar)(const void *, const void *));
STATIC size_t median_of_five(void *base, size_t size, size_t lo, size_t hi,
                             int (*compar)(const void *, const void *));
STATIC size_t partition(unsigned char *base, size_t size,
                            size_t lo, size_t hi,
                            int (*compar)(const void *, const void *),
                            void *pivot_buf);
STATIC void quicksort(unsigned char *base,
                      size_t size,
                      size_t lo, size_t hi,
                      size_t k,
                      int (*compar)(const void *, const void *),
                      void *pivot_buf);


void hybridsort(void *base, size_t nitems, size_t size, size_t k, int (*compar)(const void *, const void *)){
    if (nitems == 0) return;
    if (size != 0 && nitems > SIZE_MAX / size) return;
    if (!base){
        fprintf(stderr, "base cannot be NULL\n");
        return;
    }

    if (size == 0){
        fprintf(stderr, "size cannot be 0\n");
        return;
    }

    if (compar == NULL){
        fprintf(stderr, "Function compar cannot be NULL\n");
        return;
    }

    if (nitems < 2)
        return;

    if (nitems < k){
        selectionsort(base, nitems, size, compar);
        return;
    }

    void *pivot_buf = malloc(size);

    if (pivot_buf == NULL){
        selectionsort(base, nitems, size, compar);
        return;
    }

    quicksort((unsigned char *)base, size, 0, nitems - 1, k, compar, pivot_buf);

    free(pivot_buf);
    return;
}


STATIC void memswap(void *a, void *b, size_t size){
    if (a == b)
        return;

    unsigned char *pa = (unsigned char *)a;
    unsigned char *pb = (unsigned char *)b;

    unsigned char tmp;

    while (size--){   
        tmp = *pa;
        *pa++ = *pb;
        *pb++ = tmp;
    }
}

STATIC void selectionsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *)){
    if (nitems < 2) return;
    unsigned char *arr = (unsigned char *)base;

    for (size_t i = 0; i < nitems - 1; i++){
        size_t min_idx = i;

        for (size_t j = i + 1; j < nitems; j++){
            if (compar(arr + j * size, arr + min_idx * size) < 0){
                min_idx = j;
            }
        }

        if (min_idx != i){
            memswap(arr + min_idx * size, arr + i * size, size);
        }   
    }
}

STATIC size_t median_of_three(void *base, size_t size, size_t lo, size_t hi, int (*compar)(const void *, const void *)){
    unsigned char *arr = (unsigned char *) base;
    size_t mid = lo + (hi - lo) / 2;

    size_t a = lo, b = mid, c = hi;

    if (compar(arr + a * size, arr + b * size) > 0) { size_t tmp = a; a = b; b = tmp; }
    if (compar(arr + b * size, arr + c * size) > 0) { size_t tmp = b; b = c; c = tmp; }
    if (compar(arr + a * size, arr + b * size) > 0) { size_t tmp = a; a = b; b = tmp; }
    
    return b;
}

STATIC size_t median_of_five(void *base, size_t size, size_t lo, size_t hi, int (*compar)(const void *, const void *)){
    unsigned char *arr = (unsigned char *)base;
    size_t n = hi - lo + 1;

    /* If the range is small, reuse median_of_three */
    if (n < FIVE){
        return median_of_three(base, size, lo, hi, compar);
    }

    size_t step = n / (FIVE - 1);

    size_t idx[FIVE];

    for (size_t i = 0; i < FIVE - 1; i++){
        idx[i] = lo + (i * step);
    }
    idx[FIVE - 1] = hi;

    /* Selection sort */
    for (size_t i = 0; i < FIVE - 1; i++){
        size_t min = i;

        for (size_t j = i + 1; j < FIVE; j++){
            if (compar(arr + idx[j] * size, arr + idx[min] * size) < 0){
                min = j;
            }
        }

        if (min != i){
            size_t tmp = idx[i];
            idx[i] = idx[min];
            idx[min] = tmp;
        }
    }

    return idx[FIVE / 2]; /* with FIVE = 5 -> idx[2] */
}

STATIC size_t partition(unsigned char *base, size_t size, size_t lo, size_t hi, int (*compar)(const void *, const void *), void *pivot_buf){
    size_t i = lo - 1;
    size_t j = hi + 1;
    while(1){
        do { i++; } while (compar(base + i * size, pivot_buf) < 0);
        do { j--; } while (compar(base + j * size, pivot_buf) > 0);
        
        if (i >= j) return j;
        
        memswap(base + i * size, base + j * size, size);
    }
}

STATIC void quicksort(unsigned char *base, size_t size, size_t lo, size_t hi, size_t k, int (*compar)(const void *, const void *), void *pivot_buf){
    while (lo < hi){
        size_t n = hi - lo + 1;

        if (n < k){
            selectionsort(base + lo * size, n, size, compar);
            return;
        }

        size_t pivot = median_of_five(base, size, lo, hi, compar);
        memcpy(pivot_buf, base + pivot * size, size);

        size_t j = partition(base, size, lo, hi, compar, pivot_buf);

        if (j - lo < hi - (j + 1)){
            if (lo < j)
                quicksort(base, size, lo, j, k, compar, pivot_buf);
            lo = j + 1; 
        } else{
            if (j + 1 < hi)
                quicksort(base, size, j + 1, hi, k, compar, pivot_buf);
            hi = j;
        } 
    }
}
