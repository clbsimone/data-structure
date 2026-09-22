#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hash_table.h"

#define WORD_BUFFER_SIZE 1024

int compare_str(const void *a, const void *b){
    return strcmp((const char *)a, (const char *)b);
}

/* Hash function for strings (djb2 algorithm) */
unsigned long hash_str(const void *key){
    const unsigned char *s = (const unsigned char *)key;
    unsigned long h = 5381;
    unsigned char c;

    while ((c = *s++)){
        h = ((h << 5) + h) + c; // h * 33 + c
    }

    return h;
}

static int parse_argument(int argc, char **argv, const char **filepath, int *min_len){
    if (argc != 3){
        fprintf(stderr, "Usage: %s <text_file> <min_word_length>\n", argv[0]);
        return 0;
    }

    *filepath = argv[1];
    char *end;
    errno = 0;
    long parsed = strtol(argv[2], &end, 10);
    if (errno || end == argv[2] || *end || parsed < 0 || parsed > INT_MAX) {
        fprintf(stderr, "Error: minimum word length must be a nonnegative integer.\n");
        return 0;
    }
    *min_len = (int)parsed;

    if (*min_len < 0){
        fprintf(stderr, "Error: minimum word length must be >= 0\n");
        return 0;
    }

    return 1;
}

static int process_word(HashTable *ht, const char *word, int min_len){
    int len = (int)strlen(word);

    if (len < min_len){
        return 1;
    }

    int *count = (int *)hash_table_get(ht, word);

    if (!count){
        /* First occurrence of this word */
        char *key_copy = malloc(strlen(word) + 1); // Allocate memory for the key.
        if (!key_copy){
            return 0;
        }
        memcpy(key_copy, word, strlen(word) + 1); // Copy the complete string, including its terminator.

        int *new_count = (int *)malloc(sizeof(int));
        if (!new_count){
            free(key_copy);
            return 0;
        }

        *new_count = 1;
        hash_table_put(ht, key_copy, new_count);
        if (hash_table_get(ht, key_copy) != new_count) {
            free(key_copy);
            free(new_count);
            return 0;
        }
    } else {
        /* Word already exists -> increment counter */
        if (*count == INT_MAX) return 0;
        (*count)++;
    }
    return 1;
}

static int read_file_and_fill_table(FILE *f, HashTable *ht, int min_len){
    char word[WORD_BUFFER_SIZE];
    int idx = 0;
    int c;

    while ((c = fgetc(f)) != EOF){
        if (isalpha((unsigned char)c)){
            if (idx < WORD_BUFFER_SIZE - 1){
                word[idx++] = (char)tolower((unsigned char)c);
            }
        } else {
            if (idx > 0){
                word[idx] = '\0';
                if (!process_word(ht, word, min_len)) return 0;
                idx = 0;
            }
        }
    }

    /* Handle the last word if file does not end with a separator */
    if (idx > 0){
        word[idx] = '\0';
        if (!process_word(ht, word, min_len)) return 0;
    }
    return !ferror(f);
}

static int find_most_frequent_word(HashTable *ht, char **best_word, int *best_count){
    int size = hash_table_size(ht);
    if (size <= 0)
        return 0;

    void **keys = hash_table_keyset(ht);
    if (!keys)
        return 0;

    *best_word = NULL;
    *best_count = 0;

    for (size_t i = 0; i < (size_t)size; i++){
        char *w = (char *)keys[i];
        int *count = (int *)hash_table_get(ht, w);

        if (count != NULL && (*count > *best_count ||
            (*count == *best_count && (!*best_word || strcmp(w, *best_word) < 0)))){
            *best_count = *count;
            *best_word = w;
        }
    }

    free(keys);
    return (*best_word != NULL);
}

static void free_table_and_data(HashTable *ht){
    int size = hash_table_size(ht);
    if (size <= 0){
        hash_table_free(ht);
        return;
    }

    void **keys = hash_table_keyset(ht);
    if (!keys){
        hash_table_free(ht);
        return;
    }

    for (size_t i = 0; i < (size_t)size; i++){
        char *w = (char *)keys[i];
        int *count = (int *)hash_table_get(ht, w);
        hash_table_remove(ht, w);
        free(count);
        free(w);
    }

    free(keys);
    hash_table_free(ht);
}

int main(int argc, char **argv){
        const char *filepath;
        int min_len;

        if (!parse_argument(argc, argv, &filepath, &min_len)){
            return EXIT_FAILURE;
        }

        FILE *f = fopen(filepath, "r");
        if (!f){
            fprintf(stderr, "Error: cannot open file %s\n", filepath);
            return EXIT_FAILURE;
        }

        HashTable *ht = hash_table_create(compare_str, hash_str);
        if (!ht){
            fprintf(stderr, "Error: cannot create hash table\n");
            fclose(f);
            return EXIT_FAILURE;
        }

        int ok = read_file_and_fill_table(f, ht, min_len);
        fclose(f);
        if (!ok) {
            fprintf(stderr, "Error: failed to read or count words.\n");
            free_table_and_data(ht);
            return EXIT_FAILURE;
        }

        /* Find the most frequent word */
        char *best_word = NULL;
        int best_count = 0;

        if (find_most_frequent_word(ht, &best_word, &best_count))
        {
            printf("'%s' %d\n", best_word, best_count);
        }

        /* Free all the allocated memory */
        free_table_and_data(ht);

        return EXIT_SUCCESS;
    }
