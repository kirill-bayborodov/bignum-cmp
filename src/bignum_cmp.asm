; -----------------------------------------------------------------------------
; @file    bignum_cmp.asm
; @author  git@bayborodov.com
; @version 1.0.5
; @date    25.06.2026
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
    ; ============================================================
    ; Шаг 1. NULL-чек (branchless)
    ;   al = (a == NULL), cl = (b == NULL)
    ;   r10d = 1, если хоть один NULL, иначе 0
    ; ============================================================
    xor     eax, eax
    test    rdi, rdi
    sete    al                          ; al = (a == NULL)
    mov     r10d, eax
    test    rsi, rsi
    sete    al                          ; al = (b == NULL)
    or      r10d, eax                   ; r10d = 1 если NULL

    ; Подготовим «штраф» NULL в r11d: 0 по умолчанию, INT_MIN если NULL
    xor     r11d, r11d                  ; r11d = 0
    mov     ecx, BIGNUM_CMP_ERROR_NULL
    test    r10d, r10d
    cmovne  r11d, ecx                   ; r11d = INT_MIN если NULL

    ; ============================================================
    ; Шаг 2. Если NULL — сразу выходим через .return,
    ;        где подставим штраф в eax
    ; ============================================================
    test    r10d, r10d
    jnz     .return_null

    ; ============================================================
    ; Шаг 3. Загрузка длин
    ; ============================================================
    mov     r8d, [rdi + BIGNUM_OFFSET_LEN]
    mov     r9d, [rsi + BIGNUM_OFFSET_LEN]

    ; ============================================================
    ; Шаг 4. Сравнение длин (branchless)
    ; ============================================================
    xor     eax, eax                    ; eax = 0 (накопитель)
    cmp     r8d, r9d
    je      .same_len

    ; Длины разные: eax = sign(a.len - b.len)
    mov     eax, BIGNUM_CMP_GREATER     ; eax = 1
    mov     ecx, BIGNUM_CMP_LESS        ; ecx = -1
    cmovl   eax, ecx                    ; eax = -1 если a.len < b.len
    jmp     .return_ok

.same_len:
    ; ============================================================
    ; Шаг 5. Если len == 0 — оба нуля, возвращаем 0
    ; ============================================================
    test    r8d, r8d
    jz      .return_ok                  ; eax = 0, оба нули

    ; ============================================================
    ; Шаг 6. Пословное сравнение (branchless внутри итерации)
    ;   ecx — счётчик: len, len-1, ..., 1
    ;   eax — накопитель: 0, пока длины равны ищем первое различие
    ; ============================================================
    mov     ecx, r8d                    ; ecx = len

.loop:
    dec     ecx
    mov     rdx, [rdi + rcx*8]          ; a->words[i]
    mov     r8,  [rsi + rcx*8]          ; b->words[i]
    cmp     rdx, r8

    ; edx = sign(rdx - r8) в виде 1 / -1 / 0
    mov     edx, BIGNUM_CMP_GREATER     ; edx = 1
    mov     r9d, BIGNUM_CMP_LESS        ; r9d = -1
    cmovb   edx, r9d                    ; edx = -1 если a.words[i] < b.words[i]
    cmove   edx, eax                    ; edx = 0 если равны (eax тут = 0)

    ; Обновляем eax только если он ещё 0 (= длины были равны)
    test    eax, eax
    cmove   eax, edx
    test    ecx, ecx
    jnz     .loop                       ; счётчик не ноль → следующая итерация

    jmp     .return_ok

.return_null:
    mov     eax, r11d                   ; eax = INT_MIN

.return_ok:
    ret
