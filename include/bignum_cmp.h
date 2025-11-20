/**
 * @file    bignum_cmp.h
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    20.11.2025
 *
 * @brief Публичный заголовочный файл для модуля сравнения больших чисел (bignum_t).
 *
 * @details Этот модуль предоставляет функцию для сравнения двух беззнаковых
 *          больших целых чисел, представленных в структуре bignum_t.
 *          Он является частью библиотеки для работы с математикой bignum.
 *
 * @history
 *   - rev. 1 (05.08.2025): Первоначальное создание документа, определение API.
 *   - rev. 2 (05.08.2025): По результатам ревью:
 *                         - Защита от включения (`include guard`) переименована в BIGNUM_CMP_H.
 *                         - Добавлена обработка NULL-аргументов (возврат INT_MIN).
 *                         - Добавлены макросы для семантической версии.
 *   - rev. 3 (20.11.2025): Removed version control functions.
 *
 * @see     bignum.h
 * @since   1.0.0
 *
 */

#ifndef bignum_cmp_H
#define bignum_cmp_H

#include <bignum.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

// Проверка на наличие определения BIGNUM_CAPACITY из общего заголовка
#ifndef BIGNUM_CAPACITY
#  error "bignum.h must define BIGNUM_CAPACITY"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Коды состояния для функции bignum_cmp.
 */
typedef enum {
    BIGNUM_CMP_GREATER         =  1, /**< `1`, если `a > b` */	
    BIGNUM_CMP_EQ              =  0, /**< `0`, если `a == b` */
    BIGNUM_CMP_LESS            = -1, /**< `-1`, если `a < b` */
    BIGNUM_CMP_ERROR_NULL      = INT_MIN  /**< `INT_MIN`, если один из входных указателей (`a` или `b`) равен `NULL`.  */    
} bignum_cmp_status_t;

/**
 * @brief Сравнивает два больших беззнаковых числа.
 *
 * @details
 * ### Алгоритм
 * 1.  Проверяются входные указатели `a` и `b` на `NULL`. Если хотя бы один
 *     из них `NULL`, возвращается `INT_MIN` для индикации ошибки.
 * 2.  Сравнивается количество использованных "слов" (`words`) в каждом числе.
 *     - Если `a->words > b->words`, число `a` больше, возвращается `1`.
 *     - Если `a->words < b->words`, число `b` больше, возвращается `-1`.
 * 3.  Если количество "слов" одинаково, производится пословное сравнение,
 *     начиная со старшего "слова" (most significant word) и двигаясь к младшему.
 * 4.  Цикл проходит от `words - 1` до `0`. На каждой итерации:
 *     - Если `a->limbs[i] > b->limbs[i]`, число `a` больше, возвращается `1`.
 *     - Если `a->limbs[i] < b->limbs[i]`, число `b` больше, возвращается `-1`.
 * 5.  Если все "слова" равны, числа равны, возвращается `0`.
 *
 * @param[in] a Указатель на первое большое число (левый операнд).
 * @param[in] b Указатель на второе большое число (правый операнд).
 *
 * @return int Возвращает:
 *         - BIGNUM_CMP_GREATER `1`, если `a > b`.
 *         - BIGNUM_CMP_EQ `0`, если `a == b`.
 *         - BIGNUM_CMP_LESS `-1`, если `a < b`.
 *         - BIGNUM_CMP_ERROR_NULL `INT_MIN`, если один из входных указателей (`a` или `b`) равен `NULL`.
 *
 */
bignum_cmp_status_t bignum_cmp(const bignum_t *a, const bignum_t *b);

#ifdef __cplusplus
}
#endif

#endif /* bignum_cmp_H */
