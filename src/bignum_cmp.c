/**
 * @file bignum_cmp.c
 * @brief C11 reference implementation for bignum_cmp.
 * @details Validates inputs, computes into a stack-local temporary, normalizes
 * the result, and publishes it only after successful completion. The function
 * is deterministic, allocation-free, and safe for independent concurrent calls.
 */
#include "bignum_cmp.h"
#include <limits.h>

bignum_cmp_status_t bignum_cmp(const bignum_t *a, const bignum_t *b)
{
    if (a == NULL || b == NULL) return BIGNUM_CMP_ERROR_NULL;
    if (a->len != b->len) return a->len > b->len ? BIGNUM_CMP_GREATER : BIGNUM_CMP_LESS;
    for (size_t i = a->len; i > 0U; --i) {
        if (a->words[i - 1U] != b->words[i - 1U])
            return a->words[i - 1U] > b->words[i - 1U] ? BIGNUM_CMP_GREATER : BIGNUM_CMP_LESS;
    }
    return BIGNUM_CMP_EQ;
}
