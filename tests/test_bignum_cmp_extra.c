/**
 * @file    test_bignum_cmp_extra.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    20.10.2025
 *
 * @brief Дополнительные тесты (на робастность) для модуля bignum_cmp.
 *
 * @details
 * Этот набор тестов проверяет поведение функции `bignum_cmp` при передаче
 * некорректных входных данных, в частности `NULL`-указателей.
 * Согласно API, в таких случаях функция должна возвращать `INT_MIN`.
 *
 * Тест на перекрытие буферов (QG-13.d) не применяется, так как функция
 * `bignum_cmp` является read-only и не модифицирует входные данные,
 * что исключает риски, связанные с перекрытием памяти.
 *
 * @history
 *   - rev. 1 (05.08.2025): Первоначальное создание набора экстра-тестов.
 */

#include "bignum_cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>    // SIZE_MAX

// Макрос для упрощения вывода результатов тестов
#define RUN_TEST(test_name) \
    printf("Running test: %s... ", #test_name); \
    if (test_name()) { \
        printf("PASSED\n"); \
    } else { \
        printf("FAILED\n"); \
        return -1; \
    }

/**
 * @brief Тест: первый аргумент `a` равен NULL.
 * @details Проверяет, что `bignum_cmp(NULL, &b)` возвращает `INT_MIN`.
 */
int test_robustness_null_a() {
    bignum_t b;
    uint64_t b_data[] = {1};
    b.len = 1;
    memcpy(b.words, b_data, sizeof(b_data));

    // В release-сборке ожидаем INT_MIN.
    // В debug-сборке сработает assert, поэтому этот тест нужно запускать
    // в режиме, где assert не прерывает выполнение, или с NDEBUG.
    return bignum_cmp(NULL, &b) == INT_MIN;
}

/**
 * @brief Тест: второй аргумент `b` равен NULL.
 * @details Проверяет, что `bignum_cmp(&a, NULL)` возвращает `INT_MIN`.
 */
int test_robustness_null_b() {
    bignum_t a;
    uint64_t a_data[] = {1};
    a.len = 1;
    memcpy(a.words, a_data, sizeof(a_data));

    return bignum_cmp(&a, NULL) == INT_MIN;
}

/**
 * @brief Тест: оба аргумента равны NULL.
 * @details Проверяет, что `bignum_cmp(NULL, NULL)` возвращает `INT_MIN`.
 */
int test_robustness_both_null() {
    return bignum_cmp(NULL, NULL) == INT_MIN;
}


int main() {
    printf("\n--- Running Extra (Robustness) Tests for bignum_cmp ---\n");

    RUN_TEST(test_robustness_null_a);
    RUN_TEST(test_robustness_null_b);
    RUN_TEST(test_robustness_both_null);

    printf("--- All extra tests passed ---\n");
    return 0;
}
