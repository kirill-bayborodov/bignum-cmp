; -----------------------------------------------------------------------------
; @file    bignum_cmp.asm
; @author  git@bayborodov.com
; @version 1.0.7
; @date    01.07.2026
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
;   - rev. 4 (08.06.2026): Исправление критической ошибки в обработке длин и сравнения беззнаковых
;   - rev. 5 (25.06.2026): compact/Conditional-Move/Branchless оптимизированная версия
;   - rev. 6 (25.06.2026): - Убран rbx: Нам вообще не нужно сохранять callee-saved регистры, 
;                            если мы сравниваем регистр напрямую с памятью (cmp rax, [rsi...]). Это убирает push/pop и экономит обращения к стеку.
;                          - Возвращен Early Exit: Цикл прерывается при первом несовпадении.
;                          - Branchless финализация: Вместо прыжков .a_is_greater / .b_is_greater используется элегантный трюк с флагами и cmovb после выхода из цикла.
;                          - Исправлен баг с size_t: Длины читаются в 64-битные регистры rcx и rdx.
;                          - Холодный код вынесен вниз: Обработка NULL убрана из кэша инструкций горячего пути.
;   - rev. 7 (01.07.2026): Микрооптимизации, unrollx2
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
align 16
bignum_cmp:
    ; 1. Быстрая проверка на NULL
    test    rdi, rdi
    jz      .error_null
    test    rsi, rsi
    jz      .error_null

    ; 2. Чтение длин
    mov     rcx, [rdi + BIGNUM_LEN_OFFSET]
    mov     rdx, [rsi + BIGNUM_LEN_OFFSET]

    ; 3. Сравнение длин
    cmp     rcx, rdx
    jne     .diff_len

    ; 4. Проверка на нули
    test    rcx, rcx
    jz      .are_equal

    ; 5. Горячий цикл (выровнен для кэша)
    align 16
.loop:
    mov     rax, [rdi + rcx*8 - 8]
    cmp     rax, [rsi + rcx*8 - 8]
    jne     .diff_words
    
    dec     rcx
    jz      .are_equal
    
    mov     rax, [rdi + rcx*8 - 8]
    cmp     rax, [rsi + rcx*8 - 8]
    jne     .diff_words
    
    dec     rcx
    jnz     .loop

.are_equal:
    xor     eax, eax                ; return 0
    ret

.diff_words:
    ; SBB Trick: CF=1 если a < b, CF=0 если a > b
    sbb     eax, eax                ; eax = -1 (если a<b) или 0 (если a>b)
    or      eax, 1                  ; eax = -1 (если a<b) или 1 (если a>b)
    ret

.diff_len:
    ; SBB Trick для длин
    sbb     eax, eax
    or      eax, 1
    ret

    ; Холодный путь
.error_null:
    mov     eax, 0x80000000         ; return INT_MIN
    ret
