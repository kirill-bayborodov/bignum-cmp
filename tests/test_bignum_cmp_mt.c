/**
 * @file    test_bignum_cmp_mt.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    20.10.2025
 *
 * @brief Тест на потокобезопасность для модуля bignum_cmp.
 *
 * @details
 * ### Алгоритм Теста (QG-14)
 * 1.  Определяется несколько наборов тестовых данных.
 * 2.  Создаются "золотые" копии данных для последующей проверки на неизменность.
 * 3.  Создается несколько потоков.
 * 4.  Каждый поток многократно вызывает `bignum_cmp` и проверяет результат.
 * 5.  Для сообщения об ошибках используется атомарный флаг (`atomic_int`),
 *     чтобы избежать гонок данных при записи флага.
 * 6.  После завершения потоков проверяется, не были ли изменены исходные
 *     данные, путем сравнения с "золотыми" копиями.
 *
 * ### Обоснование Потокобезопасности
 * Функция `bignum_cmp` является потокобезопасной, так как она read-only.
 * Этот тест доказывает это, проверяя отсутствие гонок (через корректность
 * результатов) и неизменность входных данных.
 *
 * @note Для сборки этого теста требуется флаг `-pthread`.
 *       Для поддержки `<stdatomic.h>` может потребоваться стандарт C11 или выше.
 *
 * @history
 *   - rev. 1 (05.08.2025): Первоначальное создание теста.
 *   - rev. 2 (05.08.2025): По результатам ревью:
 *                         - Заменен `volatile int` на `atomic_int` для флага ошибки.
 *                         - Добавлена проверка целостности входных данных после
 *                           завершения потоков.
 */

#include "bignum_cmp.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h> // Для atomic_int
#include <assert.h>
#include <string.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000

// Атомарный флаг ошибки для корректной межпоточной синхронизации.
static atomic_int g_test_failed = ATOMIC_VAR_INIT(0);

typedef struct {
    const bignum_t* a;
    const bignum_t* b;
    int expected_result;
    int thread_id;
} thread_data_t;

void bignum_init_from_array(bignum_t* bn, const uint64_t* data, int len) {
    assert(len <= BIGNUM_CAPACITY);
    memset(bn, 0, sizeof(bignum_t));
    if (data && len > 0) {
        memcpy(bn->words, data, len * sizeof(uint64_t));
    }
    bn->len = len;
}

void* thread_func(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        if (atomic_load(&g_test_failed)) {
            return NULL;
        }
        int result = bignum_cmp(data->a, data->b);
        if (result != data->expected_result) {
            printf("Thread %d: Mismatch! Expected %d, got %d\n",
                   data->thread_id, data->expected_result, result);
            atomic_store(&g_test_failed, 1);
            return NULL;
        }
    }
    return NULL;
}

int main() {
    printf("\n--- Running Multithreading (MT) Test for bignum_cmp  ---\n");


    bignum_t bn_set[4];
    bignum_t bn_set_gold[4]; // "Золотые" копии для проверки целостности
    uint64_t d0[] = {123, 456};
    uint64_t d1[] = {123, 457};
    uint64_t d2[] = {123, 456};
    uint64_t d3[] = {1, 2, 3, 4};

    bignum_init_from_array(&bn_set[0], d0, 2);
    bignum_init_from_array(&bn_set[1], d1, 2);
    bignum_init_from_array(&bn_set[2], d2, 2);
    bignum_init_from_array(&bn_set[3], d3, 4);
    memcpy(bn_set_gold, bn_set, sizeof(bn_set)); // Сохраняем эталонное состояние

    thread_data_t thread_data[NUM_THREADS] = {
        {&bn_set[0], &bn_set[1], -1, 0},
        {&bn_set[1], &bn_set[0], 1,  1},
        {&bn_set[0], &bn_set[2], 0,  2},
        {&bn_set[3], &bn_set[0], 1,  3}
    };

    pthread_t threads[NUM_THREADS];
    printf("Creating %d threads, each running %d iterations...\n", NUM_THREADS, NUM_ITERATIONS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
            perror("pthread_create"); return -1;
        }
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join"); return -1;
        }
    }

    if (atomic_load(&g_test_failed)) {
        printf("--- MT test FAILED: Race condition or incorrect result detected.\n");
        return -1;
    }

    printf("Checking data integrity...\n");
    if (memcmp(bn_set, bn_set_gold, sizeof(bn_set)) != 0) {
        printf("--- MT test FAILED: Input data was modified!\n");
        return -1;
    }
    printf("Data integrity check passed.\n");

    printf("--- MT test PASSED ---\n");
    return 0;
}
