; -----------------------------------------------------------------------------
; @file    bignum_cmp.asm
; @author  git@bayborodov.com
; @version 1.0.0
; @date    20.11.2025
;
; @brief Ассемблерная реализация модуля сравнения больших чисел (bignum_t).
;
; @details
;   Эта реализация предназначена для архитектуры x86-64 (синтаксис YASM)
;   и следует System V AMD64 ABI. Она предоставляет полный функционал,
;   аналогичный эталонной C-версии.
;
; @history
;   - rev. 1 (05.08.2025): Первоначальная реализация, неполная и с нарушениями QG.
;   - rev. 2 (05.08.2025): Полная переработка. Реализованы все функции,
;                         добавлен раздел "Алгоритм", код приведен в полное
;                         соответствие с согласованным ABI и QG.
;   - rev. 3 (20.11.2025): Removed version control functions and .data section
; -----------------------------------------------------------------------------

section .text

; =============================================================================
; @brief Сравнивает два больших беззнаковых числа.
;
; @details
; ### Алгоритм
; 1.  Сохраняется регистр `rbx`, так как он является callee-saved.
; 2.  Проверяются входные указатели `a` (в `rdi`) и `b` (в `rsi`) на `NULL`.
;     Если один из них `NULL`, возвращается `INT_MIN` (0x80000000).
; 3.  Сравнивается количество "слов" (`len`) в каждом числе.
;     - Если `a->len > b->len`, возвращается `1`.
;     - Если `a->len < b->len`, возвращается `-1`.
; 4.  Если длины равны и равны нулю, числа считаются равными, возвращается `0`.
; 5.  Если длины равны и не равны нулю, запускается цикл сравнения.
; 6.  Цикл проходит от `len - 1` до `0`. На каждой итерации сравниваются
;     `a->words[i]` и `b->words[i]`.
; 7.  При первом же различии возвращается `1` или `-1`.
; 8.  Если цикл завершается без нахождения различий, числа равны, возвращается `0`.
; 9.  Перед выходом восстанавливается `rbx`.
;
; @abi        System V AMD64 ABI
; @param[in] rdi: const bignum_t *a (указатель на структуру)
; @param[in] rsi: const bignum_t *b (указатель на структуру)
; @return eax: bignum_cmp_status_t (1, 0, -1, 0x80000000 (ошибка) )
; @retval 1 (a > b)
; @retval 0 (a == b)
; @retval -1 (a < b)
; @retval 0x80000000 (ошибка)
; @clobbers   rbx, r8–r15, rcx, rdx
;
; =============================================================================

; Определение смещений внутри структуры bignum_t
%define BIGNUM_WORDS_OFFSET 0
%define BIGNUM_LEN_OFFSET   256  ; 32 words * 8 bytes/word
; --- Константы ---
BIGNUM_CAPACITY         equ 32
BIGNUM_WORD_SIZE        equ 8
BIGNUM_BITS             equ BIGNUM_CAPACITY * 64
BIGNUM_OFFSET_WORDS     equ 0
BIGNUM_OFFSET_LEN       equ BIGNUM_CAPACITY * BIGNUM_WORD_SIZE

BIGNUM_CMP_GREATER      equ 1
BIGNUM_CMP_EQ           equ 0
BIGNUM_CMP_LESS         equ -1
BIGNUM_CMP_ERROR_NULL   equ 0x80000000


global bignum_cmp
bignum_cmp:
    push    rbx         ; QG-35.a: Сохраняем callee-saved регистр

    ; --- Проверка на NULL ---
    test    rdi, rdi
    jz      .error_null
    test    rsi, rsi
    jz      .error_null

    ; --- Сравнение длин (len) ---
    mov     ecx, [rdi + BIGNUM_LEN_OFFSET]  ; ecx = a->len
    mov     edx, [rsi + BIGNUM_LEN_OFFSET]  ; edx = b->len
    cmp     ecx, edx
    jg      .a_is_greater
    jl      .b_is_greater

    ; --- Длины равны, проверяем, не равны ли они нулю ---
    test    ecx, ecx
    jz      .are_equal  ; Если len == 0, числа равны (оба 0)

    ; --- Цикл сравнения "слов" (limbs) от старшего к младшему ---
.compare_loop:
    ; rcx содержит текущую длину (от len до 1)
    ; rdi и rsi - базовые указатели
    ; Используем rbx как callee-saved для временного хранения
    mov     rax, [rdi + rcx * 8 - 8]
    mov     rbx, [rsi + rcx * 8 - 8]
    
    cmp     rax, rbx
    jg      .a_is_greater
    jl      .b_is_greater

    dec     rcx
    jnz     .compare_loop

.are_equal:
    xor     eax, eax    ; return 0
    jmp     .exit

.a_is_greater:
    mov     eax, BIGNUM_CMP_GREATER      ; return 1
    jmp     .exit

.b_is_greater:
    mov     eax, BIGNUM_CMP_LESS     ; return -1
    jmp     .exit

.error_null:
    mov     eax, BIGNUM_CMP_ERROR_NULL ; return INT_MIN

.exit:
    pop     rbx         ; QG-35.a: Восстанавливаем callee-saved регистр
    ret
