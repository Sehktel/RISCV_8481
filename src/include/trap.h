#ifndef TRAP_H
#define TRAP_H

#include "types.h"

/**
 * @brief Символы трап-пути, определённые в start.S (ассемблер).
 * clangd не анализирует .S-файлы: он видит только объявления отсюда,
 * поэтому без этого заголовка IDE ругается «Function definition not found».
 */

/** Обработчик ловушек (Direct): вызывает trap_handler в C, возвращает фрейм. */
void trap_vector(void);

/** Опускает CPU в U-mode: entry в mepc, режим возврата через mstatus.MPP. */
void enter_user_mode(uintptr_t entry, uintptr_t user_sp);

/** Точка возврата из U-mode в Shell (вызывается по mepc из kill_current_task
 *  / sched_return_to_shell): восстанавливает callee-saved фрейм enter_user_mode. */
void exit_user_mode_return(void);

#endif /* TRAP_H */
