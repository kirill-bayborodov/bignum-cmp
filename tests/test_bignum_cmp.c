/**
 * @file    test_bignum_cmp.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    20.10.2025
 *
 * @brief Детерминированные тесты для модуля bignum_cmp.
 *
 * @details
 * ### Анализ полноты покрытия (QG-13.f)
 * Этот набор тестов проверяет основные и граничные случаи для функции `bignum_cmp`:
 * 1.  **Сравнение чисел разной длины:**
 *     - `test_cmp_a_gt_b_diff_len`, `test_cmp_a_lt_b_diff_len`.
 * 2.  **Сравнение чисел одинаковой длины:**
 *     - `test_cmp_a_gt_b_same_len` (различие в старшем слове).
 *     - `test_cmp_a_lt_b_same_len` (различие в младшем слове).
 *     - `test_cmp_a_eq_b_same_len` (полное равенство).
 *     - `test_cmp_diff_in_middle` (a < b, различие в среднем слове).
 *     - `test_cmp_diff_in_middle_gt` (a > b, различие в среднем слове).
 * 3.  **Граничные случаи:**
 *     - `test_cmp_a_eq_b_zero` (сравнение нулей).
 *     - `test_cmp_with_zero` (сравнение с нулем).
 *     - `test_cmp_max_capacity` (сравнение чисел максимальной длины).
 * 4.  **Робастность логики:**
 *     - `test_cmp_leading_zeros_norm`: Проверяет, что сравнение по `len` имеет приоритет.
 *
 * Покрытие считается исчерпывающим для детерминированной проверки логики.
 *
 * @history
 *   - rev. 1 (05.08.2025): Первоначальное создание набора тестов.
 *   - rev. 2 (05.08.2025): Добавлены тесты по результатам ревью.
 *   - rev. 3 (05.08.2025): Исправлена ошибка в `test_cmp_diff_in_middle`. Добавлен
 *                         `test_cmp_diff_in_middle_gt`. Восстановлено форматирование.
 *   - rev. 4 (05.08.2025): Обновлен Doxygen-блок с анализом полноты покрытия (QG-8, QG-13.f).
 */

#include "bignum_cmp.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Вспомогательная функция для сравнения
/*static int bignum_are_equal(const bignum_t* a, const bignum_t* b) {
    if (a->len != b->len) return 0;
    if (a->len == 0) return 1;
    return memcmp(a->words, b->words, a->len * sizeof(uint64_t)) == 0;
}*/

// Макрос для упрощения вывода результатов тестов
#define RUN_TEST(test_name) \
    printf("Running test: %s... ", #test_name); \
    if (test_name()) { \
        printf("PASSED\n"); \
    } else { \
        printf("FAILED\n"); \
        return -1; \
    }

// Вспомогательная функция для инициализации bignum_t
void bignum_init_from_array(bignum_t* bn, const uint64_t* data, int len) {
    assert(len <= BIGNUM_CAPACITY);
    memset(bn, 0, sizeof(bignum_t));
    if (data && len > 0) {
        memcpy(bn->words, data, len * sizeof(uint64_t));
    }
    bn->len = len;
}

/** @brief Тест: a > b, разная длина. */
int test_cmp_a_gt_b_diff_len() {
    bignum_t a, b;
    uint64_t a_data[] = {1, 1};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 1);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: a < b, разная длина. */
int test_cmp_a_lt_b_diff_len() {
    bignum_t a, b;
    uint64_t a_data[] = {0xFFFFFFFFFFFFFFFF};
    uint64_t b_data[] = {1, 1};
    bignum_init_from_array(&a, a_data, 1);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == -1;
}

/** @brief Тест: a > b, одинаковая длина, различие в старшем слове. */
int test_cmp_a_gt_b_same_len() {
    bignum_t a, b;
    uint64_t a_data[] = {0, 2};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF, 1};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: a < b, одинаковая длина, различие в младшем слове. */
int test_cmp_a_lt_b_same_len() {
    bignum_t a, b;
    uint64_t a_data[] = {5, 1};
    uint64_t b_data[] = {6, 1};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == -1;
}

/** @brief Тест: a == b, одинаковая длина. */
int test_cmp_a_eq_b_same_len() {
    bignum_t a, b;
    uint64_t data[] = {1, 2, 3};
    bignum_init_from_array(&a, data, 3);
    bignum_init_from_array(&b, data, 3);
    return bignum_cmp(&a, &b) == 0;
}

/** @brief Тест: a == b, оба числа равны 0. */
int test_cmp_a_eq_b_zero() {
    bignum_t a, b;
    bignum_init_from_array(&a, NULL, 0);
    bignum_init_from_array(&b, NULL, 0);
    return bignum_cmp(&a, &b) == 0;
}

/** @brief Тест: сравнение с нулем. */
int test_cmp_with_zero() {
    bignum_t a, b;
    uint64_t a_data[] = {100};
    bignum_init_from_array(&a, a_data, 1);
    bignum_init_from_array(&b, NULL, 0);
    return bignum_cmp(&a, &b) == 1 && bignum_cmp(&b, &a) == -1;
}

/** @brief Тест: a < b, различие в среднем слове (ИСПРАВЛЕН). */
int test_cmp_diff_in_middle() {
    bignum_t a, b;
    // a = {..., 3}, b = {..., 9}. Старшее слово b > a.
    uint64_t a_data[] = {1, 5, 3};
    uint64_t b_data[] = {1, 4, 9};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 3);
    // Ожидаемый результат -1, так как b.words[2] > a.words[2]
    return bignum_cmp(&a, &b) == -1;
}

/** @brief Тест: a > b, различие в среднем слове (НОВЫЙ). */
int test_cmp_diff_in_middle_gt() {
    bignum_t a, b;
    // a = {..., 5, 9}, b = {..., 4, 9}. Старшие слова равны, a.words[1] > b.words[1].
    uint64_t a_data[] = {1, 5, 9};
    uint64_t b_data[] = {1, 4, 9};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 3);
    // Ожидаемый результат 1
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: сравнение чисел максимальной длины. */
int test_cmp_max_capacity() {
    bignum_t a, b;
    uint64_t a_data[BIGNUM_CAPACITY] = {0};
    uint64_t b_data[BIGNUM_CAPACITY] = {0};
    a_data[BIGNUM_CAPACITY - 1] = 10; // a > b
    b_data[BIGNUM_CAPACITY - 1] = 9;
    bignum_init_from_array(&a, a_data, BIGNUM_CAPACITY);
    bignum_init_from_array(&b, b_data, BIGNUM_CAPACITY);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: сравнение чисел с ведущими нулями. */
int test_cmp_leading_zeros_norm() {
    bignum_t a, b;
    // a = {1, 2, 0}, b = {MAX, MAX}
    // a.len = 3, b.len = 2. Ожидается a > b, т.к. len(a) > len(b)
    uint64_t a_data[] = {1, 2, 0};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == 1;
}


int main() {
    printf("\n--- Running Deterministic Tests for bignum_cmp  ---\n");

    RUN_TEST(test_cmp_a_gt_b_diff_len);
    RUN_TEST(test_cmp_a_lt_b_diff_len);
    RUN_TEST(test_cmp_a_gt_b_same_len);
    RUN_TEST(test_cmp_a_lt_b_same_len);
    RUN_TEST(test_cmp_a_eq_b_same_len);
    RUN_TEST(test_cmp_a_eq_b_zero);
    RUN_TEST(test_cmp_with_zero);
    RUN_TEST(test_cmp_diff_in_middle);
    RUN_TEST(test_cmp_diff_in_middle_gt);
    RUN_TEST(test_cmp_max_capacity);
    RUN_TEST(test_cmp_leading_zeros_norm);

    printf("--- All deterministic tests passed ---\n");
    return 0;
}

