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
#include <bignum_common.h>
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


/** @brief Тест: a > b, разная длина. */
int test_cmp_a_gt_b_diff_len() {
    bignum_t a, b;
    uint64_t a_data[] = {1, 1};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 1);
    return bignum_cmp(&a, &b) == 1 ;
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
/*int test_cmp_a_eq_b_zero() {
    bignum_t a, b;
    bignum_init_from_array(&a, NULL, 0);
    bignum_init_from_array(&b, NULL, 0);
    return bignum_cmp(&a, &b) == 0;
}*/

/** @brief Тест: сравнение с нулем. */
/*int test_cmp_with_zero() {
    bignum_t a, b;
    uint64_t a_data[] = {100};
    bignum_init_from_array(&a, a_data, 1);
    bignum_init_from_array(&b, NULL, 0);
    return bignum_cmp(&a, &b) == 1 && bignum_cmp(&b, &a) == -1;
}*/

/**

 * @brief Тест: a == b, оба числа равны 0.

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_a_eq_b_zero(void)

{

    bignum_t a, b;


    /* Валидный способ получить канонический ноль:

     *   bignum_init_u64(&x, 0)  →  len=0, words=0.

     * (Вызов bignum_init_from_array(&x, NULL, 0) — UB:

     *  при src==NULL функция возвращает BIGNUM_ERROR_NULL_ARG

     *  и не трогает dst.) */

    bignum_init_u64(&a, 0);

    bignum_init_u64(&b, 0);


    /* Проверяем обе стороны — иначе не поймаем баг «cmp всегда 0». */

    if (bignum_cmp(&a, &b) != 0) return 0;

    if (bignum_cmp(&b, &a) != 0) return 0;


    return 1;

}


/**

 * @brief Тест: сравнение с нулем.

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_with_zero(void)

{

    bignum_t a, b;


    uint64_t a_data[] = { 100 };

    bignum_init_from_array(&a, a_data, 1);  /* a = 100, len=1 */


    /* Валидный ноль вместо (NULL, 0). */

    bignum_init_u64(&b, 0);                  /* b = 0,  len=0 */


    int r_ab = bignum_cmp(&a, &b);   /* a > b  →  > 0 */

    int r_ba = bignum_cmp(&b, &a);   /* b < a  →  < 0 */


    /* Проверяем знак, а не строгое равенство ±1:

     * корректная реализация может вернуть, например, 5 или -42. */

    if (r_ab <= 0) return 0;

    if (r_ba >= 0) return 0;


    return 1;

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
/*int test_cmp_leading_zeros_norm() {
    bignum_t a, b;
    // a = {1, 2, 0}, b = {MAX, MAX}
    // a.len = 3, b.len = 2. Ожидается a > b, т.к. len(a) > len(b)
    uint64_t a_data[] = {1, 2, 0};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == 1;
}*/

/**

 * @brief Тест: нормализация «срезает» трейлинг-нули, поэтому

 *        a_data с хвостовым нулём становится короче ожидаемого.

 *

 *  Исходный сценарий пользователя, но с корректным ожиданием:

 *  a = {1, 2, 0} после нормализации имеет len=2,

 *  b = {MAX, MAX} имеет len=2,

 *  по значению a < b (2*2^64+1 << 2^128-2^64).

 *

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_leading_zeros_norm(void)

{

    bignum_t a, b;


    uint64_t a_data[] = { 1, 2, 0 };

    uint64_t b_data[] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };


    bignum_init_from_array(&a, a_data, 3);

    bignum_init_from_array(&b, b_data, 2);


    /* Контракт: после init_from_array трейлинг-нули срезаны.

     * Поэтому a.len == 2, b.len == 2. */

    if (a.len != 2) return 0;

    if (b.len != 2) return 0;


    /* По значению a < b. Проверяем знак, а не строгое равенство -1. */

    int r = bignum_cmp(&a, &b);

    if (r >= 0) return 0;   /* ожидаем a < b */


    return 1;

}


/**

 * @brief Тест: два числа с одинаковым значением, но разным

 *        количеством ведущих нулей в исходном массиве должны

 *        быть равны после нормализации.

 *

 *  Это и есть «ведущие нули» в правильной постановке: они не

 *  должны влиять на результат сравнения.

 *

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_leading_zeros_equal(void)

{

    bignum_t a, b;


    uint64_t a_data[] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };          /* 2 слова */

    uint64_t b_data[] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0, 0 };    /* 4 слова с 2 хвостовыми нулями */


    bignum_init_from_array(&a, a_data, 2);

    bignum_init_from_array(&b, b_data, 4);


    /* Оба нормализуются до одинакового значения */

    if (a.len != 2) return 0;

    if (b.len != 2) return 0;

    if (bignum_cmp(&a, &b) != 0) return 0;


    return 1;

}

/** @brief Тест: различаются несколько слов, должно победить самое старшее. */
int test_cmp_multiple_diffs() {
    bignum_t a, b;
    // a = {99, 99, 99}, b = {99, 1, 1}
    // Старшее words[2]: 99 > 1 → a > b
    uint64_t a_data[] = {99, 99, 99};
    uint64_t b_data[] = {99, 1, 1};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 3);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: младшее слово различается сильно, но старшее нет — выигрывает старшее. */
int test_cmp_lsb_dominates_msb() {
    bignum_t a, b;
    // a = {MAX, 0}, b = {1, MAX}
    // Старшее words[1]: 0 < 1 → a < b, даже хотя words[0] у a больше
    // a = {MAX, 0}, b = {0, 1}. Старшее words[1] у b=1 > у a=0 → b > a
    uint64_t a_data2[] = {0xFFFFFFFFFFFFFFFF, 0};
    uint64_t b_data2[] = {0xFFFFFFFFFFFFFFFF, 1};
    bignum_init_from_array(&a, a_data2, 2);
    bignum_init_from_array(&b, b_data2, 2);
    return bignum_cmp(&a, &b) == -1;
}

/** @brief Тест: оба числа len=1, a < b. */
int test_cmp_len1_a_lt_b() {
    bignum_t a, b;
    uint64_t a_data[] = {0x1234};
    uint64_t b_data[] = {0x1235};
    bignum_init_from_array(&a, a_data, 1);
    bignum_init_from_array(&b, b_data, 1);
    return bignum_cmp(&a, &b) == -1;
}

/** @brief Тест: оба числа len=1, равны. */
int test_cmp_len1_eq() {
    bignum_t a, b;
    uint64_t data[] = {42};
    bignum_init_from_array(&a, data, 1);
    bignum_init_from_array(&b, data, 1);
    return bignum_cmp(&a, &b) == 0;
}

/** @brief Тест: переход через границу 2^64. */
int test_cmp_max_word_boundary() {
    bignum_t a, b;
    // a = {0, 1}  (= 2^64)
    // b = {MAX, 0} (= 2^64 - 1)
    // a > b
    uint64_t a_data[] = {0, 1};
    uint64_t b_data[] = {0xFFFFFFFFFFFFFFFF, 0};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: b содержит a как префикс, но короче. */
int test_cmp_proper_prefix() {
    bignum_t a, b;
    // a = {1, 2, 3}, b = {1, 2}
    // a > b по длине
    uint64_t a_data[] = {1, 2, 3};
    uint64_t b_data[] = {1, 2};
    bignum_init_from_array(&a, a_data, 3);
    bignum_init_from_array(&b, b_data, 2);
    return bignum_cmp(&a, &b) == 1;
}

/** @brief Тест: a содержится как префикс в b. */
int test_cmp_proper_prefix_lt() {
    bignum_t a, b;
    uint64_t a_data[] = {1, 2};
    uint64_t b_data[] = {1, 2, 3};
    bignum_init_from_array(&a, a_data, 2);
    bignum_init_from_array(&b, b_data, 3);
    return bignum_cmp(&a, &b) == -1;
}

/**

 * @brief Тест: анти-симметрия на нетривиальных числах.

 *

 *  Проверяет, что bignum_cmp(a,b) и bignum_cmp(b,a) дают

 *  строго противоположные знаки при a != b. Ловит классический

 *  баг «всегда возвращаю 1».

 *

 *  Также проверяет случай одинаковой длины, но различающихся

 *  только в старшем слове: если cmp смотрит только на длину и

 *  игнорирует содержимое слов, тест упадёт.

 *

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_symmetry() {

    bignum_t small, big;


    uint64_t small_data[] = { 5 };

    uint64_t big_data[]   = { 0xFFFFFFFFFFFFFFFFULL, 1 };  /* 2^128 - 2^64 + 1 > 5 */


    bignum_init_from_array(&small, small_data, 1);

    bignum_init_from_array(&big,   big_data,   2);


    int r_sb = bignum_cmp(&small, &big);

    int r_bs = bignum_cmp(&big,   &small);


    /* a < b → cmp(a,b) < 0, и обратное направление даёт > 0 */

    if (r_sb >= 0) return 0;

    if (r_bs <= 0) return 0;


    /* Граничный случай: одинаковая длина, различие только в старшем слове */

    bignum_t x, y;

    uint64_t x_data[] = { 0, 0, 7 };

    uint64_t y_data[] = { 0, 0, 8 };

    bignum_init_from_array(&x, x_data, 3);

    bignum_init_from_array(&y, y_data, 3);


    if (bignum_cmp(&x, &y) >= 0) return 0;   /* x < y  по старшему слову */

    if (bignum_cmp(&y, &x) <= 0) return 0;   /* симметрия */


    return 1;

}


/**

 * @brief Тест: рефлексивность — bignum_cmp(&a, &a) == 0.

 *

 *  Ловит баг «перед сравнением сдвигаю указатель» — тогда

 *  cmp(&a,&a) прочитает words[1..] вместо words[0..] и почти

 *  наверняка найдёт различие с самим собой.

 *

 * @return 1 если тест пройден, 0 если провален.

 */

int test_cmp_self() {

    bignum_t a;

    uint64_t a_data[] = { 0xDEADBEEFULL, 0xCAFEBABEULL, 0x1234567890ABCDEFULL };

    bignum_init_from_array(&a, a_data, 3);


    if (bignum_cmp(&a, &a) != 0) return 0;


    /* Рефлексивность должна работать и для нуля */

    bignum_t zero;

    bignum_init_u64(&zero, 0);

    if (bignum_cmp(&zero, &zero) != 0) return 0;


    return 1;

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
    RUN_TEST(test_cmp_leading_zeros_equal);    
    RUN_TEST(test_cmp_multiple_diffs);
    RUN_TEST(test_cmp_lsb_dominates_msb);
    RUN_TEST(test_cmp_len1_a_lt_b);    
    RUN_TEST(test_cmp_len1_eq);
    RUN_TEST(test_cmp_max_word_boundary);
    RUN_TEST(test_cmp_proper_prefix);
    RUN_TEST(test_cmp_proper_prefix_lt);    
    RUN_TEST(test_cmp_symmetry);
    RUN_TEST(test_cmp_self);
    
    printf("--- All deterministic tests passed ---\n");
    return 0;
}

