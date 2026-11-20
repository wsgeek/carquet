/**
 * @file stats_order.h
 * @brief The sort order of a column's statistics, shared by the page and
 * column writers.
 *
 * Parquet's TYPE_DEFINED_ORDER: integers signed unless the logical type
 * says unsigned; BOOLEAN false before true; BYTE_ARRAY and a non-decimal
 * FIXED_LEN_BYTE_ARRAY unsigned bytewise, a prefix before its extensions;
 * a DECIMAL FIXED_LEN_BYTE_ARRAY a signed big-endian two's complement.
 */

#ifndef CARQUET_WRITER_STATS_ORDER_H
#define CARQUET_WRITER_STATS_ORDER_H

#include <carquet/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum carquet_stats_order {
    CARQUET_STATS_ORDER_SIGNED_I32,
    CARQUET_STATS_ORDER_UNSIGNED_I32,
    CARQUET_STATS_ORDER_SIGNED_I64,
    CARQUET_STATS_ORDER_UNSIGNED_I64,
    CARQUET_STATS_ORDER_FLOAT,
    CARQUET_STATS_ORDER_DOUBLE,
    CARQUET_STATS_ORDER_UNSIGNED_BYTES,
    CARQUET_STATS_ORDER_SIGNED_BIG_ENDIAN,
} carquet_stats_order_t;

static inline bool carquet_logical_type_is_unsigned_integer(
    carquet_physical_type_t physical_type,
    const carquet_logical_type_t* logical_type) {
    if (!logical_type || logical_type->id != CARQUET_LOGICAL_INTEGER ||
        logical_type->params.integer.is_signed) {
        return false;
    }

    switch (physical_type) {
        case CARQUET_PHYSICAL_INT32:
            return logical_type->params.integer.bit_width > 0 &&
                   logical_type->params.integer.bit_width <= 32;
        case CARQUET_PHYSICAL_INT64:
            return logical_type->params.integer.bit_width > 32 &&
                   logical_type->params.integer.bit_width <= 64;
        default:
            return false;
    }
}

static inline carquet_stats_order_t carquet_stats_order_of(
    carquet_physical_type_t physical_type,
    const carquet_logical_type_t* logical_type) {
    bool is_unsigned = carquet_logical_type_is_unsigned_integer(physical_type, logical_type);
    switch (physical_type) {
        case CARQUET_PHYSICAL_INT32:
            return is_unsigned ? CARQUET_STATS_ORDER_UNSIGNED_I32 : CARQUET_STATS_ORDER_SIGNED_I32;
        case CARQUET_PHYSICAL_INT64:
            return is_unsigned ? CARQUET_STATS_ORDER_UNSIGNED_I64 : CARQUET_STATS_ORDER_SIGNED_I64;
        case CARQUET_PHYSICAL_FLOAT:
            return CARQUET_STATS_ORDER_FLOAT;
        case CARQUET_PHYSICAL_DOUBLE:
            return CARQUET_STATS_ORDER_DOUBLE;
        case CARQUET_PHYSICAL_FIXED_LEN_BYTE_ARRAY:
            return logical_type && logical_type->id == CARQUET_LOGICAL_DECIMAL
                ? CARQUET_STATS_ORDER_SIGNED_BIG_ENDIAN
                : CARQUET_STATS_ORDER_UNSIGNED_BYTES;
        default:
            return CARQUET_STATS_ORDER_UNSIGNED_BYTES;
    }
}

static inline int carquet_compare_i32_values(int32_t lhs, int32_t rhs, bool is_unsigned) {
    if (is_unsigned) {
        uint32_t lhs_u = (uint32_t)lhs;
        uint32_t rhs_u = (uint32_t)rhs;
        return (lhs_u > rhs_u) - (lhs_u < rhs_u);
    }
    return (lhs > rhs) - (lhs < rhs);
}

static inline int carquet_compare_i64_values(int64_t lhs, int64_t rhs, bool is_unsigned) {
    if (is_unsigned) {
        uint64_t lhs_u = (uint64_t)lhs;
        uint64_t rhs_u = (uint64_t)rhs;
        return (lhs_u > rhs_u) - (lhs_u < rhs_u);
    }
    return (lhs > rhs) - (lhs < rhs);
}

/* Compares two PLAIN-encoded statistics values of one column. */
static inline int carquet_stats_compare(
    carquet_stats_order_t order,
    const uint8_t* lhs, size_t lhs_size,
    const uint8_t* rhs, size_t rhs_size) {
    switch (order) {
        case CARQUET_STATS_ORDER_SIGNED_I32:
        case CARQUET_STATS_ORDER_UNSIGNED_I32: {
            int32_t l, r;
            memcpy(&l, lhs, sizeof(l));
            memcpy(&r, rhs, sizeof(r));
            return carquet_compare_i32_values(l, r, order == CARQUET_STATS_ORDER_UNSIGNED_I32);
        }
        case CARQUET_STATS_ORDER_SIGNED_I64:
        case CARQUET_STATS_ORDER_UNSIGNED_I64: {
            int64_t l, r;
            memcpy(&l, lhs, sizeof(l));
            memcpy(&r, rhs, sizeof(r));
            return carquet_compare_i64_values(l, r, order == CARQUET_STATS_ORDER_UNSIGNED_I64);
        }
        case CARQUET_STATS_ORDER_FLOAT: {
            float l, r;
            memcpy(&l, lhs, sizeof(l));
            memcpy(&r, rhs, sizeof(r));
            return (l > r) - (l < r);
        }
        case CARQUET_STATS_ORDER_DOUBLE: {
            double l, r;
            memcpy(&l, lhs, sizeof(l));
            memcpy(&r, rhs, sizeof(r));
            return (l > r) - (l < r);
        }
        case CARQUET_STATS_ORDER_SIGNED_BIG_ENDIAN: {
            /* Equal widths (a FIXED_LEN_BYTE_ARRAY): the sign byte signed,
             * the rest unsigned. */
            if (lhs_size > 0) {
                int l0 = (int8_t)lhs[0];
                int r0 = (int8_t)rhs[0];
                if (l0 != r0) return (l0 > r0) - (l0 < r0);
            }
            int c = lhs_size > 1 ? memcmp(lhs + 1, rhs + 1, lhs_size - 1) : 0;
            return (c > 0) - (c < 0);
        }
        case CARQUET_STATS_ORDER_UNSIGNED_BYTES:
        default: {
            size_t common = lhs_size < rhs_size ? lhs_size : rhs_size;
            int c = common > 0 ? memcmp(lhs, rhs, common) : 0;
            if (c != 0) return (c > 0) - (c < 0);
            return (lhs_size > rhs_size) - (lhs_size < rhs_size);
        }
    }
}

#endif /* CARQUET_WRITER_STATS_ORDER_H */
