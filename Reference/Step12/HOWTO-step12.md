# HOWTO: сборка, проверка и запуск микроядра RISC-V (этап 1)

## 1. Требования

- Docker с образом `riscv_os_devsecops:latest` (кросс-тулчейн `riscv64-unknown-elf-gcc 13.2`, clang-format, clang-tidy, cppcheck, gitleaks, QEMU);
- Makefile-обёртки: все команды выполняются в контейнере с монтированием рабочей директории.

## 2. Полный цикл одной командой

```bash
make clear && make build && make format && make tidy && make run
```

Что делает каждая цель:

| Цель | Действие | Артефакт/результат |
|---|---|---|
| `make clear` | Очистка `build/` и `bin/` | Чистое рабочее состояние |
| `make build` | `cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .` + `cmake --build build` | `bin/kernel.bin` (~12K), `bin/kernel.elf` (~71K) |
| `make format` | `clang-format -i` по списку файлов `src/` | Формат на месте |
| `make tidy` | `clang-tidy` c `--warnings-as-errors='*'`, `-header-filter='^/workspace/src/'`, флаги `--target=riscv64-unknown-elf -march=rv64gc -mabi=lp64d -ffreestanding -nostdlib -Isrc -Isrc/include` | Сборка без ворнингов |
| `make run` | `qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic` | Shell |

Выход из QEMU — `Ctrl-A`, затем `X`.

## 3. Контрактная проверка в Shell

Порядок и эталонный вывод:

### `info`

```
--- Диагностика ядра RISCV ---
Архитектура: RV64GC (64-bit)
Базовый адрес UART0: 0x10000000
Текущий указатель стека (SP): 0x80005f60
```

### `mem` / `memtest`

`mem` до тестов — один FREE-блок на весь объём кучи (`0x80006000..0x87e00000`).

`memtest` проверяет:

1. Выделение 64/128/256 B и коалесценцию: после освобождения p1, p2, p3 куча снова единый FREE-блок.
2. Security-чеки `kfree`:
   - Double-Free → `[SECURITY] kfree: Invalid/corrupted header ...`;
   - невыровненный указатель → `[SECURITY] kfree: unaligned pointer ...`;
   - указатель вне кучи → `[SECURITY] kfree: pointer out of heap bounds ...`.

### `usermode`

```
[KERNEL] Опускаем процессор в U-mode (entry=0x80000540, user_sp=0x80007020)...
[U-MODE] Привет из непривилегированного пространства! Запрашиваю тики...
[U-MODE] Системный вызов SYS_GET_TICKS вернул ненулевое время!
[U-MODE] Вызываю SYS_EXIT(0) для возврата в ядро...
[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: 0)
[KERNEL] Управление успешно возвращено в M-mode Shell!
```

### `sched`

Вытесняющий Round-Robin: 2 U-задачи, квант 10 мс, 6 тиков `TIMER IRQ #N`, чередование TASK A / TASK B, затем `[SCHED] Демонстрация планировщика завершена. Освобождаем память задач...`.

### `pmptest` — главная демонстрация этапа 1

```
[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...

[SECURITY] MMU Violation! Store/AMO Page Fault (VA не отображён в Sv39 или прав не хватает)
[SECURITY] mcause=15 | mepc=0x... | mtval=0x10000000 — процесс принудительно завершён!
[KERNEL] Ядро устояло, управление возвращено в M-mode Shell!
```

Интерпретация: VA `0x10000000` не входит в identity-маппинг (`0x80000000..0xBFFFFFFF`) — трансляции нет, Sv39 даёт Store Page Fault **до** того, как сработает PMP. Порядок рубежей: сначала MMU, потом PMP.

### `exit`

```
Завершение работы системы...
```

## 4. Инварианты, которые нельзя нарушать при правках

1. **TOR-цепочка PMP меняется только полным перестроением** — нижняя граница записи N берётся из верхней границы N-1; одиночная правка середины ломает соседей.
2. **Регион таблиц `[pgtables_start, pgtables_end)` обязан быть R для U-mode** — иначе обход MMU из U-задачи падает с IAF (это главный урок этапа).
3. **Не ставить бит `L`** в `pmpcfg` — M-mode должен оставаться неограниченным.
4. **Page faults 12/13/15 классифицируются в trap-обработчике** так же, как access faults 1/5/7 — оба пути завершают U-процесс и возвращают Shell (не паника).
5. **Компилятор — `gnu17`** (GCC 13 по умолчанию); фиксировать явно через `CMAKE_C_STANDARD 17` + `CMAKE_C_EXTENSIONS ON` (нужны GNU statement expressions в `csr.h`).

## 5. Типовые проблемы и решения

| Симптом | Причина | Решение |
|---|---|---|
| Красная волна в VS Code, `cannot open source file "xxx.h"`, `identifier ... is undefined` | IntelliSense не знает путей сборки | Подключить `build/compile_commands.json` в `.vscode/c_cpp_properties.json` (`"compileCommands"`), затем Reset IntelliSense Database |
| `Error: this declaration has no storage class or type specifier` у `_Static_assert` | Стоит до определения типа / файл парсится не как C11+ | Ставить сразу после определения структуры; запасной вариант — массив-трюк `typedef char check[(cond) ? 1 : -1];` |
| IAF (mcause=1) на входе в U-задачу после правки PMP | Регион таблиц не попал в TOR-цепочку | Проверить `grep -n "pmp_configure" src/` — демки должны звать версию с регионом pgtables |
| `[PANIC] Необработанное исключение: mcause=15` | Page faults не классифицированы в trap-обработчике | Добавить case 12/13/15 с тем же путём завершения, что у access faults |
| В CI падает clang-format | Файлы правились без прогона `make format` | Прогнать `make format` до коммита |

## 6. CI/CD (существующий пайплайн)

Пайплайн `build-and-sast` (запускается на push) выполняет последовательно:

1. checkout;
2. **gitleaks** — сканирование истории на утечки (`--config .gitleaks.toml`, `--redact`);
3. **clang-format** — `--dry-run --Werror` по всем файлам `src/`;
4. **cppcheck** — `--enable=warning,style,performance,portability`, `--platform=unix64`, кастомная библиотека `.infrastructure/cppcheck/kernel.cfg`, `--error-exitcode=1`;
5. **clang-tidy** — тот же набор файлов и флагов, что и локально;
6. **сборка** — кросс-сборка через CMake + тулчейн-файл;
7. **публикация артефакта** в пакетный реестр с ротацией: хранятся 10 последних версий, более старые удаляются.

Требование к коммиту: перед push локально должны быть зелёными `make format` и `make tidy` — тогда пайплайн проходит с первой итерации.

## 7. Версионирование этапов

```bash
git add -A
git commit -m "feat(mmu+pmp): Sv39 identity-маппинг + TOR-регион таблиц в PMP-профиле; trap-классификация page faults (12/13/15)"
git tag step-12 && git push local step-12
```
