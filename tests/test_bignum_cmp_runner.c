/**
 * @file    test_bignum_cmp_runner.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    03.12.2025
 *
 * @brief Интеграционный тест‑раннер для библиотеки libbignum_cmp.a.
 * @details Применяется для проверки достаточности сигнатур 
 *          в файле заголовка (header) при сборке и линковке
 *          статической библиотеки libbignum_cmp.a
 *
 * @history
 *   - rev. 1 (03.12.2025): Создание теста
 */  
#include "bignum_cmp.h"
#include <assert.h>
#include <stdio.h>  
int main() {
 printf("Running test: test_bignum_cmp_runner... "); 
 bignum_t a = {0};  
 bignum_t b = {0};  	
 bignum_cmp(&a, &b);  
 assert(1);
 printf("PASSED\n");   
 return 0;  
}