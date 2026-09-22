#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "record_io_bin.h"
#include "hybridsort.h"


static int cmp_by_id(const void *pa, const void *pb){
    const Record *a = pa, *b = pb;
    return (a->id > b->id) - (a->id < b->id);
}

static int cmp_by_field1(const void *pa, const void *pb){
    const Record *a = pa, *b = pb;
    if (isnan(a->field1)) return isnan(b->field1) ? 0 : 1;
    if (isnan(b->field1)) return -1;
    return (a->field1 > b->field1) - (a->field1 < b->field1);
}

static int cmp_by_field2(const void *pa, const void *pb){
    const Record *a = pa, *b = pb;
    return (a->field2 > b->field2) - (a->field2 < b->field2);
}

static int cmp_by_field3(const void *pa, const void *pb){
    const Record *a = pa, *b = pb;
    return strncmp(a->field3, b->field3, sizeof(a->field3));
}

void sort_records(FILE *in_file, FILE *out_file, size_t field, size_t k){
    Record *records = NULL;
    size_t count    = 0;

    if(load_records_from_file(in_file, &records, &count) < 0){
        fprintf(stderr, "Error: failed to load records.\n");
        exit(EXIT_FAILURE);
    }

    int (*cmp)(const void *, const void *) = NULL;

    switch (field)
    {
    case 1:
        cmp = cmp_by_id;
        break;
    case 2:
        cmp = cmp_by_field1;
        break;
    case 3:
        cmp = cmp_by_field2;
        break;
    case 4:
        cmp = cmp_by_field3;
        break;
    default:
        fprintf(stderr, "Error: invalid field.\n");
        free(records);
        exit(EXIT_FAILURE);
    }

    hybridsort(records, count, sizeof(Record), k, cmp);
    if (save_records_to_file(out_file, records, count) < 0) {
        free(records);
        exit(EXIT_FAILURE);
    }

    free(records);
    return;
}

int main(int argc, char const *argv[]){
    if (argc != 5){
        fprintf(stderr, "Error:\n Correct: usage %s <input_file> <output_file> <field 1-4> <k>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    
    const char *in_name  = argv[1];
    const char *out_name = argv[2];

    if (strcmp(in_name, out_name) == 0){
        fprintf(stderr, "Error: input and output file must be different.\n");
        exit(EXIT_FAILURE);
    }

    char *end;
    errno = 0;
    unsigned long field = strtoul(argv[3], &end, 10);
    if (errno || *end || end == argv[3] || field < 1 || field > 4) {
        fprintf(stderr, "Error: field must be an integer from 1 to 4.\n");
        return EXIT_FAILURE;
    }
    errno = 0;
    unsigned long k = strtoul(argv[4], &end, 10);
    if (errno || *end || end == argv[4] || argv[4][0] == '-' || k > SIZE_MAX) {
        fprintf(stderr, "Error: k must be a nonnegative integer.\n");
        return EXIT_FAILURE;
    }

    FILE *in_file   = fopen(in_name, "rb");

    if (!in_file){
        fprintf(stderr, "Error opening input file\n");
        return EXIT_FAILURE;
    }
    
    FILE *out_file  = fopen(out_name, "wb");

    if (!out_file){
        fprintf(stderr, "Error opening output file\n");
        fclose(in_file);
        return EXIT_FAILURE;
    }

    sort_records(in_file, out_file, field, k);

    fclose(in_file);
    if (fclose(out_file) != 0) {
        perror("Error closing output file");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
