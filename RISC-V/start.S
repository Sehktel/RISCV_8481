.section .text.init
.global _start

_start:
    # 1. Отключаем все ядра, кроме основного (hart 0)
    csrr a0, mhartid            # Прочитать ID текущего ядра
    bnez a0, park               # Если это не ядро 0, отправляем в бесконечный цикл

    # 2. Настраиваем указатель стека (SP)
    la sp, _stack_top

    # 3. Переходим к коду на C
    j main

park:
    wfi                         # Ожидание прерывания (сон)
    j park
