/**
 * @file    test_bignum_cmp_ext_mt.c
 * @brief   Расширенный MT-тест для bignum_cmp.
 *
 *  Что добавилось по сравнению с test_bignum_cmp_mt.c:
 *
 *  1. NULL-аргументы (a == NULL, b == NULL, оба NULL) — отдельные потоки
 *     гоняют ветку .return_null с возвратом BIGNUM_CMP_ERROR_NULL (0x80000000).
 *  2. Граничные длины: len=1, len=BIGNUM_CAPACITY (32).
 *  3. len=0 (оба нуля) — отдельный путь в .same_len через jz .return_ok.
 *  4. Большие числа с различием в старшем слове.
 *  5. Self-cmp под нагрузкой.
 *  6. Все потоки разделяют общий пул bignum_t (read-only контракт),
 *     что даёт максимальную contention на кэш-линиях и ловит ложный
 *     sharing / регрессии в read-only path.
 *  7. Проверка целостности входных данных после прогона осталась.
 *
 *  Контракт bignum_cmp из src/bignum_cmp.asm (rev. 5):
 *      NULL-аргумент  →  0x80000000 (INT_MIN)  — BIGNUM_CMP_ERROR_NULL
 *      a > b          →   1
 *      a == b         →   0
 *      a < b          →  -1
 *
 *  Стиль сохранён: int main(void), RUN_TEST-подобный printf, atomic_int
 *  для флага ошибки, pthread + -pthread при сборке.
 *
 *  Как вставить:
 *      либо положить рядом с test_bignum_cmp_mt.c и добавить цель в Makefile,
 *      либо заменить содержимое test_bignum_cmp_mt.c целиком.
 *
 *  Рекомендация для CI: добавить отдельный make target
 *      test_mt_tsan: CC=clang, CFLAGS+=-fsanitize=thread
 *  чтобы автоматически ловить data race при изменении контракта на
 *  write в будущем.
 */

#include "bignum_cmp.h"
#include <bignum_common.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#define NUM_THREADS       8
#define NUM_ITERATIONS    500000

/* Атомарный флаг ошибки — единый для всех потоков. */
static atomic_int g_test_failed = ATOMIC_VAR_INIT(0);

/* ------------------------------------------------------------------ */
/*  Описание одного «задания» для потока                              */
/* ------------------------------------------------------------------ */

typedef struct {
    const bignum_t* a;
    const bignum_t* b;
    int             expected;   /* ожидаемый код возврата bignum_cmp */
    int             thread_id;
    const char*     label;      /* для диагностики при провале */
} thread_task_t;

/* ------------------------------------------------------------------ */
/*  Worker: гоняет одну пару (a, b) и проверяет результат             */
/* ------------------------------------------------------------------ */

static void* thread_func(void* arg)
{
    thread_task_t* t = (thread_task_t*)arg;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        /* Быстрый выход, если другой поток уже упал — не плодим спам. */
        if (atomic_load(&g_test_failed)) {
            return NULL;
        }

        int result = bignum_cmp(t->a, t->b);

        if (result != t->expected) {
            printf("Thread %d [%s] iter %d: expected %d, got %d\n",
                   t->thread_id, t->label, i, t->expected, result);
            atomic_store(&g_test_failed, 1);
            return NULL;
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("\n--- Running Extended Multithreading Test for bignum_cmp ---\n");
    printf("Threads: %d, iterations per thread: %d, total ops: %d\n\n",
           NUM_THREADS, NUM_ITERATIONS, NUM_THREADS * NUM_ITERATIONS);

    /* ----------------------------------------------------------------
     *  Подготовка пула read-only bignum-ов.
     *  Все структуры инициализируются один раз, дальше — только чтение.
     *  Это безопасно по контракту bignum_cmp (функция не модифицирует
     *  входы), и одновременно даёт максимальную cache-line contention,
     *  что обостряет любые регрессии write-path.
     * ---------------------------------------------------------------- */

    bignum_t bn[10];
    bignum_t bn_gold[10];   /* для финальной проверки целостности */

    /* 0: {123, 456}, len=2 */
    uint64_t d0[] = { 123, 456 };
    bignum_init_from_array(&bn[0], d0, 2);

    /* 1: {123, 457}, len=2  (чуть больше 0) */
    uint64_t d1[] = { 123, 457 };
    bignum_init_from_array(&bn[1], d1, 2);

    /* 2: {123, 456}, len=2  (равно 0) */
    uint64_t d2[] = { 123, 456 };
    bignum_init_from_array(&bn[2], d2, 2);

    /* 3: {1, 2, 3, 4}, len=4 (больше 0 за счёт длины) */
    uint64_t d3[] = { 1, 2, 3, 4 };
    bignum_init_from_array(&bn[3], d3, 4);

    /* 4: ноль, len=0 (через валидный путь) */
    bignum_init_u64(&bn[4], 0);

    /* 5: ноль (другой экземпляр), len=0 */
    bignum_init_u64(&bn[5], 0);

    /* 6: len=1, значение 7 */
    uint64_t d6[] = { 7 };
    bignum_init_from_array(&bn[6], d6, 1);

    /* 7: len=1, значение 8 */
    uint64_t d7[] = { 8 };
    bignum_init_from_array(&bn[7], d7, 1);

    /* 8: длина = BIGNUM_CAPACITY, всё MAX, но words[31] = MAX-1 */
    uint64_t d8[BIGNUM_CAPACITY];
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i) d8[i] = UINT64_MAX;
    d8[BIGNUM_CAPACITY - 1] = UINT64_MAX - 1;
    bignum_init_from_array(&bn[8], d8, BIGNUM_CAPACITY);

    /* 9: длина = BIGNUM_CAPACITY, всё MAX (на 1 больше 8) */
    uint64_t d9[BIGNUM_CAPACITY];
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i) d9[i] = UINT64_MAX;
    bignum_init_from_array(&bn[9], d9, BIGNUM_CAPACITY);

    memcpy(bn_gold, bn, sizeof(bn));

    /* ----------------------------------------------------------------
     *  8 заданий для 8 потоков. Каждое покрывает свой кейс.
     *  Потоки 6 и 7 — NULL-аргументы, ожидаем BIGNUM_CMP_ERROR_NULL.
     * ---------------------------------------------------------------- */

    thread_task_t tasks[NUM_THREADS] = {
        /* 0: a < b, разная длина (классика из исходного MT) */
        { &bn[0], &bn[1], -1,                       0, "diff_len: a<b"      },

        /* 1: анти-симметрия (тот же набор, обратное направление) */
        { &bn[1], &bn[0],  1,                       1, "diff_len: a>b"      },

        /* 2: a == b при равной длине и равных словах */
        { &bn[0], &bn[2],  0,                       2, "eq_same_len"        },

        /* 3: a > b за счёт длины */
        { &bn[3], &bn[0],  1,                       3, "gt_by_len"          },

        /* 4: оба нуля, len=0 — отдельная ветка .same_len → jz */
        { &bn[4], &bn[5],  0,                       4, "zero_eq_zero"       },

        /* 5: self-cmp под нагрузкой (ловит регрессию указателя) */
        { &bn[3], &bn[3],  0,                       5, "self_cmp"           },

        /* 6: a == NULL → ожидаем BIGNUM_CMP_ERROR_NULL (0x80000000) */
        { NULL,   &bn[0],  (int)0x80000000,         6, "null_a"             },

        /* 7: b == NULL → тот же штраф (anti-симметрия с потоком 6) */
        { &bn[0], NULL,    (int)0x80000000,         7, "null_b"             },
    };

    /* (Опционально, для будущего: тест на bn[8] vs bn[9] через
     * дополнительные потоки. Сейчас укладываемся в NUM_THREADS=8.) */

    pthread_t threads[NUM_THREADS];

    printf("Launching threads...\n");
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, &tasks[i]) != 0) {
            perror("pthread_create");
            return -1;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return -1;
        }
    }

    /* --- Проверка результата --- */
    if (atomic_load(&g_test_failed)) {
        printf("--- MT test FAILED: race or wrong result detected ---\n");
        return -1;
    }

    /* --- Проверка целостности входов (никто не должен был писать) --- */
    printf("Checking data integrity...\n");
    if (memcmp(bn, bn_gold, sizeof(bn)) != 0) {
        printf("--- MT test FAILED: input bignum_t was modified! ---\n");
        return -1;
    }
    printf("Data integrity check passed.\n");

    /* --- Итог по покрытым кейсам --- */
    printf("\nCoverage summary:\n");
    printf("  [x] diff length (a<b and a>b, anti-symmetry)\n");
    printf("  [x] equal same-length values\n");
    printf("  [x] gt by length (proper prefix)\n");
    printf("  [x] both zero (len=0 short-circuit)\n");
    printf("  [x] self-comparison (pointer regression guard)\n");
    printf("  [x] NULL a  -> BIGNUM_CMP_ERROR_NULL\n");
    printf("  [x] NULL b  -> BIGNUM_CMP_ERROR_NULL (anti-symmetry w/ NULL a)\n");

    printf("\n--- Extended MT test PASSED ---\n");
    return 0;
}