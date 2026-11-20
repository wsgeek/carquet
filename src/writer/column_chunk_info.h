/**
 * @file column_chunk_info.h
 * @brief A finalized column chunk as the row group writer hands it to the
 * file writer.
 */

#ifndef CARQUET_WRITER_COLUMN_CHUNK_INFO_H
#define CARQUET_WRITER_COLUMN_CHUNK_INFO_H

#include <carquet/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct column_chunk_info {
    int64_t file_offset;
    int64_t total_compressed_size;
    int64_t total_uncompressed_size;
    int64_t num_values;

    /* Statistics: has_statistics when written; the bounds when the chunk
     * holds a non-null value. The bounds point into the column writer and
     * live until its next reset. */
    bool has_statistics;
    bool has_min_max;
    int64_t null_count;
    const uint8_t* min_value;
    size_t min_size;
    const uint8_t* max_value;
    size_t max_size;

    /* The chunk's dictionary page, first in the chunk (0: none) */
    int64_t dictionary_page_size;

    carquet_encoding_t encodings[3];
    int32_t num_encodings;

    carquet_physical_type_t type;
    carquet_compression_t compression;
    int32_t type_length;
    char* path;
} column_chunk_info_t;

#endif /* CARQUET_WRITER_COLUMN_CHUNK_INFO_H */
