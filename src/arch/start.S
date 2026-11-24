.section .text.init
.global _start
.global trap_vector
.global enter_user_mode

_start:
    # 1. Отключаем все ядра, кроме основного (hart 0)
    csrr a0, mhartid            # Прочитать ID текущего ядра
    bnez a0, park               # Если это не ядро 0, отправляем в бесконечный цикл

    # 2. Настраиваем указатель стека (SP) и сохраняем вершину стека ядра в mscratch
    la sp, kernel_stack_top
    csrw mscratch, sp

    # 3. Регистрируем обработчик ловушек в mtvec (режим Direct, младшие 2 бита = 0)
    la t0, trap_vector
    csrw mtvec, t0

    # 4. Переходим к коду на C с сохранением адреса возврата
    jal ra, kmain

    # 5. Ловушка на случай, если kmain всё-таки завершился
tail_loop:
    wfi
    j tail_loop

park:
    wfi
    j park

.align 4
trap_vector:
    # Атомарно меняем местами sp (пользовательский) и mscratch (стек ядра)
    csrrw sp, mscratch, sp

    # Выделяем 256 байт (32 регистра * 8 байт) на стеке ядра для сохранения контекста
    addi sp, sp, -256

    sd ra,   0(sp)
    sd gp,   8(sp)
    sd tp,   16(sp)
    sd t0,   24(sp)
    sd t1,   32(sp)
    sd t2,   40(sp)
    sd s0,   48(sp)
    sd s1,   56(sp)
    sd a0,   64(sp)
    sd a1,   72(sp)
    sd a2,   80(sp)
    sd a3,   88(sp)
    sd a4,   96(sp)
    sd a5,   104(sp)
    sd a6,   112(sp)
    sd a7,   120(sp)
    sd s2,   128(sp)
    sd s3,   136(sp)
    sd s4,   144(sp)
    sd s5,   152(sp)
    sd s6,   160(sp)
    sd s7,   168(sp)
    sd s8,   176(sp)
    sd s9,   184(sp)
    sd s10,  192(sp)
    sd s11,  200(sp)
    sd t3,   208(sp)
    sd t4,   216(sp)
    sd t5,   224(sp)
    sd t6,   232(sp)

    # Сохраняем исходный пользовательский sp (сейчас лежит в mscratch) во фрейм
    csrr t0, mscratch
    sd t0,   240(sp)

    # Передаём текущий trap_frame_t* в a0; C-обработчик вернёт trap_frame_t* следующего потока
    mv a0, sp
    jal ra, trap_handler
    mv sp, a0

    # # Запоминаем вершину стека ядра для следующего входа в trap_vector
    # addi t0, sp, 256
    # csrw mscratch, t0

    # Загружаем пользовательский sp выбранного потока в mscratch для финального обмена
    ld t0,   240(sp)
    csrw mscratch, t0

    # Восстанавливаем регистры общего назначения
    ld ra,   0(sp)
    ld gp,   8(sp)
    ld tp,   16(sp)
    ld t0,   24(sp)
    ld t1,   32(sp)
    ld t2,   40(sp)
    ld s0,   48(sp)
    ld s1,   56(sp)
    ld a0,   64(sp)
    ld a1,   72(sp)
    ld a2,   80(sp)
    ld a3,   88(sp)
    ld a4,   96(sp)
    ld a5,   104(sp)
    ld a6,   112(sp)
    ld a7,   120(sp)
    ld s2,   128(sp)
    ld s3,   136(sp)
    ld s4,   144(sp)
    ld s5,   152(sp)
    ld s6,   160(sp)
    ld s7,   168(sp)
    ld s8,   176(sp)
    ld s9,   184(sp)
    ld s10,  192(sp)
    ld s11,  200(sp)
    ld t3,   208(sp)
    ld t4,   216(sp)
    ld t5,   224(sp)
    ld t6,   232(sp)

    # Освобождаем фрейм на стеке ядра и возвращаем U-sp в sp, а стек ядра в mscratch
    addi sp, sp, 256
    csrrw sp, mscratch, sp

    mret



# ============================================================================
# ОПУСКАНИЕ В USER MODE: void enter_user_mode(uintptr_t entry, uintptr_t user_sp)
# a0 = адрес точки входа функции в U-mode, a1 = вершина стека пользователя
# ============================================================================
enter_user_mode:
    # Сохраняем адрес возврата и callee-saved регистры ядра, чтобы SYS_EXIT вернулся в shell
    addi sp, sp, -112
    sd ra,  0(sp)
    sd s0,  8(sp)
    sd s1,  16(sp)
    sd s2,  24(sp)
    sd s3,  32(sp)
    sd s4,  40(sp)
    sd s5,  48(sp)
    sd s6,  56(sp)
    sd s7,  64(sp)
    sd s8,  72(sp)
    sd s9,  80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    # Сохраняем текущий sp ядра в mscratch для возврата при ловушке
    csrw mscratch, sp

    # Настраиваем mstatus: сбрасываем биты MPP (биты 12:11 = 00 -> U-mode), ставим MPIE (бит 7 = 1)
    csrr t0, mstatus
    li t1, ~(3 << 11)
    and t0, t0, t1
    ori t0, t0, (1 << 7)
    csrw mstatus, t0

    # Записываем адрес точки входа U-mode в mepc и переключаем sp на стек пользователя
    csrw mepc, a0
    mv sp, a1

    # Аппаратный переход в User Mode!
    mret

# Точка возврата в ядро при вызове системного вызова SYS_EXIT из U-mode
.global exit_user_mode_return
exit_user_mode_return:
    # В a0 передан код возврата процесса; текущий sp указывает на сохранённый фрейм enter_user_mode
    ld ra,  0(sp)
    ld s0,  8(sp)
    ld s1,  16(sp)
    ld s2,  24(sp)
    ld s3,  32(sp)
    ld s4,  40(sp)
    ld s5,  48(sp)
    ld s6,  56(sp)
    ld s7,  64(sp)
    ld s8,  72(sp)
    ld s9,  80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)
    addi sp, sp, 112
    ret
