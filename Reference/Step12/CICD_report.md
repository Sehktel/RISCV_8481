```
$ make ci-verify VERBOSE=1
/usr/bin/docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace -w /workspace riscv_os_devsecops:latest sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
-- Configuring done (0.0s)
-- Generating done (0.0s)
CMake Warning:
  Manually-specified variables were not used by the project:

    CMAKE_TOOLCHAIN_FILE


-- Build files have been written to: /workspace/build
[ 10%] Built target shell
[ 30%] Built target arch
[ 50%] Built target drivers
[ 65%] Built target klib
[ 80%] Built target mm
[ 90%] Built target sched
[100%] Built target kernel.elf
/usr/bin/docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace -w /workspace riscv_os_devsecops:latest python3 .infrastructure/scripts/smoke_test.py 1
[CI] OK: отрицательный кейс ловится (FAIL-путь валидатора жив)
[CI]   ✓ info: диагностика ядра (UART)
[CI]   ✓ MMU: Sv39 стартовал
[CI]   ✓ MMU: трансляция активна
[CI]   ✓ heap: коалесценция до единого блока
[CI]   ✓ heap: тест безопасности дошёл
[CI]   ✓ heap: double-free пойман
[CI]   ✓ usermode: ecall вернул тики
[CI]   ✓ usermode: SYS_EXIT
[CI]   ✓ pmptest: классификация page fault
[CI]   ✓ pmptest: точный адрес нарушения
[CI]   ✓ pmptest: возврат в Shell
[CI]   ✓ exit: штатное завершение
[CI] PASS: полный контракт отработал (MMU, heap, usermode, pmptest, exit)
===== QEMU TRANSCRIPT =====
[HEAP] init: 0x80006000..0x87e00000 (132096000 байт)
[MMU] Sv39 инициализирован: L1@0x80004000, L2@0x80003000, identity 1GiB @0x80000000
[MMU] Трансляция включена (satp=0x80004) — действует в S/U-mode
--- RISCV Микроядро: Shell загружен ---
sh> info
info
--- Диагностика ядра RISCV ---
Архитектура: RV64GC (64-bit)
Базовый адрес UART0: 0x10000000
Текущий указатель стека (SP): 0x80005f60
sh> memtest
memtest
=== 1. Тест выделения и коалесценции ===
=== Карта кучи (Heap Map) ===
[0] Block: 0x80006000 | Payload: 0x80006020 | Size: 64 B | USED
[1] Block: 0x80006060 | Payload: 0x80006080 | Size: 128 B | USED
[2] Block: 0x80006100 | Payload: 0x80006120 | Size: 256 B | USED
[3] Block: 0x80006220 | Payload: 0x80006240 | Size: 132095424 B | FREE
Всего блоков: 4 | Занято: 448 B | Свободно: 132095424 B
--- Освобождаем средний блок (p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80006000 | Payload: 0x80006020 | Size: 64 B | USED
[1] Block: 0x80006060 | Payload: 0x80006080 | Size: 128 B | FREE
[2] Block: 0x80006100 | Payload: 0x80006120 | Size: 256 B | USED
[3] Block: 0x80006220 | Payload: 0x80006240 | Size: 132095424 B | FREE
Всего блоков: 4 | Занято: 320 B | Свободно: 132095552 B
--- Освобождаем p1 (слияние вперёд с p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80006000 | Payload: 0x80006020 | Size: 224 B | FREE
[1] Block: 0x80006100 | Payload: 0x80006120 | Size: 256 B | USED
[2] Block: 0x80006220 | Payload: 0x80006240 | Size: 132095424 B | FREE
Всего блоков: 3 | Занято: 256 B | Свободно: 132095648 B
--- Освобождаем p3 (слияние назад, куча должна стать единой) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80006000 | Payload: 0x80006020 | Size: 132095968 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132095968 B
=== 2. Тест безопасности (Security Checks) ===
Попытка Double-Free на p3:
[SECURITY] kfree: Invalid/corrupted header at pointer: 0x0000000080006120
Попытка освободить невыровненный указатель:
[SECURITY] kfree: unaligned pointer 0x80005003
Попытка освободить указатель вне кучи (например, из стека/кода):
[SECURITY] kfree: pointer out of heap bounds 0x80000010
sh> usermode
usermode
[KERNEL] Опускаем процессор в U-mode (entry=0x80000540, user_sp=0x80007020)...
[U-MODE] Привет из непривилегированного пространства! Запрашиваю тики...
[U-MODE] Системный вызов SYS_GET_TICKS вернул ненулевое время!
[U-MODE] Вызываю SYS_EXIT(0) для возврата в ядро...
[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: 0)
[KERNEL] Управление успешно возвращено в M-mode Shell!
sh> pmptest
pmptest
[PMP] text: 0x80000000..0x80002f18 | heap: 0x80006000..0x87e00000 | ustack: 0x80006020
[KERNEL] Запускаем задачу-нарушитель U-mode со строгим PMP...
[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...
[SECURITY] MMU Violation! Store/AMO Page Fault (VA не отображён в Sv39 или прав не хватает)
[SECURITY] mcause=15 | mepc=0x8000059c | mtval=0x10000000 — процесс принудительно завершён!
[KERNEL] Ядро устояло, управление возвращено в M-mode Shell!
sh> exit
exit
Завершение работы системы...

===== END TRANSCRIPT =====
```