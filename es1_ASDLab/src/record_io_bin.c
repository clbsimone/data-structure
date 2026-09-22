#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_io_bin.h"

#define N_CHARACTER     16
#define RECORD_SIZE     36

#define SIZE_ID         8
#define SIZE_FIELD1     4
#define SIZE_FIELD2     8
#define SIZE_FIELD3     16

#define OFFSET_ID       0
#define OFFSET_FIELD1   (OFFSET_ID + SIZE_ID)
#define OFFSET_FIELD2   (OFFSET_FIELD1 + SIZE_FIELD1)
#define OFFSET_FIELD3   (OFFSET_FIELD2 + SIZE_FIELD2)


int read_record(FILE *file, Record *record);
int write_record(FILE *file, const Record *record);
int load_records_from_file(FILE *file, Record **records, size_t *count);
int save_records_to_file(FILE *file, const Record *records, size_t count);
void print_record(const Record *record);

int read_record(FILE *file, Record *record){
    if (!file || !record) return -1;
    unsigned char buf[RECORD_SIZE];

    // Read exactly RECORD_SIZE (36) bytes into the buffer.
    size_t n = fread(buf, 1, RECORD_SIZE, file);

    if (n == 0){
        // EOF: no more records to read.
        if(feof(file)) return 0;
        
        fprintf(stderr, "Error: failed to read from file.\n");
        return -1;
    }

    if (n != RECORD_SIZE){
        fprintf(stderr, "Error: incomplete record (expected %d bytes, got %zu).\n", RECORD_SIZE, n);
        return -1;
    }

    // The binary format requires a little-endian host and 32-bit IEEE float.
    memcpy(&record->id,     buf + OFFSET_ID,     SIZE_ID);
    memcpy(&record->field1, buf + OFFSET_FIELD1, SIZE_FIELD1);
    memcpy(&record->field2, buf + OFFSET_FIELD2, SIZE_FIELD2);
    memcpy(record->field3,  buf + OFFSET_FIELD3, SIZE_FIELD3);

    // Ensure field3 is null-terminated so it can be safely used as a C string.
    record->field3[SIZE_FIELD3 - 1] = '\0';

    return 1;
}

int write_record(FILE *file, const Record *record){
    if (!file || !record) return -1;
    unsigned char buf[RECORD_SIZE];
    
    memcpy(buf + OFFSET_ID,     &record->id,     SIZE_ID);
    memcpy(buf + OFFSET_FIELD1, &record->field1, SIZE_FIELD1);
    memcpy(buf + OFFSET_FIELD2, &record->field2, SIZE_FIELD2);
    memcpy(buf + OFFSET_FIELD3, record->field3,  SIZE_FIELD3);

    // Write exactly RECORD_SIZE bytes to the file.
    size_t n = fwrite(buf, 1, RECORD_SIZE, file);

    if (n != RECORD_SIZE){
        fprintf(stderr, "Error: failed to write record (expected %d bytes, wrote %zu).\n", RECORD_SIZE, n);
        return -1;
    }
    
    return 1;
}

int load_records_from_file(FILE *file, Record **records, size_t *count){
    if (!file || !records || !count){
        fprintf(stderr, "Error: invalid argument to load_records_from_file.\n");
        return -1;
    }

    // Initialize the output array and record count.
    *records = NULL;
    *count = 0;

    size_t capacity = 0;

    while(1){
        Record r;
        int res = read_record(file, &r);

        if (res == 0) 
            // EOF
            break;
        if (res < 0){
            fprintf(stderr, "Error: could not read record from file.\n");
            free(*records);
            *records = NULL;
            *count = 0;
            return -1;
        }
        
        if (*count == capacity){
            size_t new_capacity = (capacity == 0) ? 16 : capacity * 2;
            Record *tmp = realloc(*records, new_capacity * sizeof(Record));

            if (!tmp){
                fprintf(stderr, "Error: memory allocation failed while loading records.\n");
                free(*records);
                *records = NULL;
                *count = 0;
                return -1;
            }

            *records = tmp;
            capacity = new_capacity;
        }

        // Append the newly read record.
        (*records)[*count] = r;
        (*count)++;
    }

    return 1;
}

int save_records_to_file(FILE *file, const Record *records, size_t count){
    if (!file){
        fprintf(stderr, "Error: file pointer is NULL in save_records_to_file.\n");
        return -1;
    }

    if (!records && count > 0){
        fprintf(stderr, "Error: records pointer is NULL but count > 0.\n");
        return -1;
    }

    for (size_t i = 0; i < count; i++){
        if (write_record(file, &records[i]) < 0){
            fprintf(stderr, "Error: failed to write record %zu.\n", i);
            return -1;
        }
    }

    return 1;
}

void print_record(const Record *record){
    if (!record){
        fprintf(stderr, "Error: NULL record passed to print_record.\n");
        return;
    }

    printf("%-12llu  %-14f  %-14lld  %-16s",
           (unsigned long long)record->id,
           record->field1,
           (long long)record->field2,
           record->field3);
}
