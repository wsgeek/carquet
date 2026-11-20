/**
 * @file dictionary.h
 * @brief The byte-array dictionary a column writer keeps across the pages
 * of one column chunk.
 */

#ifndef CARQUET_ENCODING_DICTIONARY_H
#define CARQUET_ENCODING_DICTIONARY_H

#include <carquet/error.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct carquet_byte_array_dictionary carquet_byte_array_dictionary_t;

carquet_byte_array_dictionary_t* carquet_byte_array_dictionary_create(void);
void carquet_byte_array_dictionary_destroy(carquet_byte_array_dictionary_t* dictionary);

/* Empties the dictionary for the next column chunk. */
carquet_status_t carquet_byte_array_dictionary_clear(carquet_byte_array_dictionary_t* dictionary);

/* The value's index, its entry added when new; indices count from 0 in
 * insertion order. */
carquet_status_t carquet_byte_array_dictionary_insert(
    carquet_byte_array_dictionary_t* dictionary,
    const uint8_t* value, size_t value_size, uint32_t* index);

uint32_t carquet_byte_array_dictionary_count(const carquet_byte_array_dictionary_t* dictionary);

/* The entries PLAIN-encoded (a 4-byte little-endian length, then the
 * bytes, per entry, in index order): the dictionary page's payload. */
const uint8_t* carquet_byte_array_dictionary_plain(
    const carquet_byte_array_dictionary_t* dictionary, size_t* size);

#ifdef __cplusplus
}
#endif

#endif /* CARQUET_ENCODING_DICTIONARY_H */
