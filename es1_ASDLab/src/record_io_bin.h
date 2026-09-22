#ifndef RECORD_IO_BIN_H_SIMONE_BASSO
#define RECORD_IO_BIN_H_SIMONE_BASSO

/**
 * Structure representing a single record in the binary file.
 *
 * Layout in the file (36 bytes total, little-endian):
 *   - bytes  0–7   : uint64_t id      (8 bytes)
 *   - bytes  8–11  : float field1     (4 bytes)
 *   - bytes 12–19  : int64_t field2   (8 bytes)
 *   - bytes 20–35  : char field3[16]  (16 bytes, padded with '\0')
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct _Record {
    uint64_t id;
    float field1;
    int64_t field2;
    char field3[16];
} Record;

/**
 * @brief Read a single record from a binary file.
 *
 * Reads exactly 36 bytes from the file, converts them into a Record
 * structure (assuming little-endian encoding), and stores the result
 * into the variable pointed to by @p record.
 *
 * @param file   Pointer to an open binary file.
 * @param record Pointer to a Record structure to fill.
 * @return - 1 if a record was successfully read,
 * @return - 0 if end-of-file was reached,
 * @return - -1 on read error or truncated record 
 */
int read_record(FILE *file, Record *record);

/**
 * @brief Write a single Record to a binary file.
 *
 * Writes exactly 36 bytes to the output file in the expected binary format.
 *
 * @param file   Pointer to an open binary file.
 * @param record Pointer to the Record to write.
 * @return - 1 on success,
 * @return - -1 on write error
 */
int write_record(FILE *file, const Record *record);

/**
 * @brief Load all records from a binary file into a dynamically allocated array.
 *
 * The function reads records until EOF, expanding the array as needed.
 * The array must later be freed with `free()`.
 *
 * @param file    Pointer to an open binary file.
 * @param records Output parameter: pointer to a dynamically allocated array of Record.
 * @param count   Output parameter: number of records loaded.
 * @return - 1 on success,
 * @return - -1 on error (in which case `records` is set to NULL and `count` to 0)
 */
int load_records_from_file(FILE *file, Record **records, size_t *count);

/**
 * @brief Save an array of records to a binary file.
 *
 * Writes exactly @p count records into the binary file using write_record().
 *
 * @param file    Pointer to an open binary file.
 * @param records Pointer to an array of records to write.
 * @param count   Number of records to write.
 * 
 * @return - 1 on success,
 * @return - -1 on error
 */
int save_records_to_file(FILE *file, const Record *records, size_t count);

/**
 * @brief Print a record to standard output in a human-readable format.
 *
 * @param record Pointer to the Record to print.
 */
void print_record(const Record *record);

#endif
