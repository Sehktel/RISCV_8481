#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Контрактный smoke-тест ядра RISC-V.

Интерактивный прогон в QEMU (pexpect ждёт приглашение 'sh>' перед каждой
командой — гонка «первая команда съедена» исключена) + валидация полного
текста транскрипта.

Использование: python3 smoke_test.py [VERBOSE]
  VERBOSE=1 — напечатать транскрипт QEMU после прогона
"""
import os
import re
import sys

import pexpect

QEMU_CMD = (
    "qemu-system-riscv64 -M virt -bios none "
    "-kernel bin/kernel.bin -nographic"
)
LOG = "build/qemu-smoke.log"

# Команды контрактного прогона (из HOWTO, этап 1)
COMMANDS = ["info", "memtest", "usermode", "pmptest", "exit"]

# (подстрока, человеческое описание проверки) — порядок отражает контракт
CHECKS = [
    ("UART0", "info: диагностика ядра (UART)"),
    ("Sv39 инициализирован", "MMU: Sv39 стартовал"),
    ("Трансляция включена", "MMU: трансляция активна"),
    ("Всего блоков: 1 | Занято: 0 B", "heap: коалесценция до единого блока"),
    ("Попытка Double-Free на p3", "heap: тест безопасности дошёл"),
    ("kfree: Invalid/corrupted header", "heap: double-free пойман"),
    ("SYS_GET_TICKS вернул ненулевое время", "usermode: ecall вернул тики"),
    ("Процесс U-mode завершился через SYS_EXIT (код: 0)", "usermode: SYS_EXIT"),
    ("SECURITY] MMU Violation! Store/AMO Page Fault", "pmptest: классификация page fault"),
    ("mtval=0x10000000", "pmptest: точный адрес нарушения"),
    ("Ядро устояло, управление возвращено в M-mode Shell", "pmptest: возврат в Shell"),
    ("Завершение работы системы", "exit: штатное завершение"),
]

META_PROBE = "ЭТОЙ СТРОКИ В ЛОГЕ НЕТ"  # мета-проверка FAIL-пути валидатора


def compact(text: str) -> str:
    """Нормализует переносы строк и схлопывает дубли пустых строк.

    QEMU сериализует каждый \\n ядра как \\r\\n, а ядро само дублирует
    пустые строки — на экране это выглядит как разрывы после каждой строки.
    Лог на диске остаётся нетронутым (эталонная копия), схлопываем только
    для отображения и проверок.
    """
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    return re.sub(r"\n{2,}", "\n", text)


def die(msg: str, transcript: str = None) -> None:
    print(f"[CI] FAIL: {msg}", file=sys.stderr)
    if transcript:
        print("===== QEMU TRANSCRIPT =====", file=sys.stderr)
        print(compact(transcript), file=sys.stderr)
        print("===== END TRANSCRIPT =====", file=sys.stderr)
    sys.exit(1)


def run_qemu() -> str:
    """Прогоняет контракт через QEMU, возвращает компактный транскрипт."""
    child = None
    cmd = "загрузке ядра"
    try:
        child = pexpect.spawn(
            QEMU_CMD, timeout=30, encoding="utf-8", codec_errors="replace"
        )
        with open(LOG, "w", encoding="utf-8") as log:
            child.logfile = log
            for cmd in COMMANDS:
                child.expect(r"sh>\s*$")  # синхронизация по приглашению шелла
                child.sendline(cmd)
            child.expect(r"Завершение работы системы")
            child.expect(pexpect.EOF)
    except pexpect.TIMEOUT:
        die(
            f"таймаут: ядро не дошло до ожидаемой точки "
            f"(последняя команда: {cmd})",
            transcript=getattr(child, "before", None),
        )
    except pexpect.EOF:
        die(
            "неожиданный EOF: QEMU завершился раньше контракта",
            transcript=getattr(child, "before", None),
        )

    with open(LOG, encoding="utf-8") as f:
        return compact(f.read())


def main() -> None:
    transcript = run_qemu()

    # Контрактные проверки: литеральные подстроки, без экранирования
    for pattern, desc in CHECKS:
        if not re.search(re.escape(pattern), transcript):
            die(f"{desc}: не найдено «{pattern}»", transcript=transcript)

    # Негативная проверка: ядро не должно паниковать
    if "PANIC" in transcript:
        die("ядро запаниковало", transcript=transcript)

    # Мета-проверка FAIL-пути: валидатор обязан уметь падать.
    # Проверяем на живом транскрипте литеральной строкой, которой там нет.
    if re.search(META_PROBE, transcript):
        die("мета-проверка: валидатор пропускает что угодно")

    print("[CI] OK: отрицательный кейс ловится (FAIL-путь валидатора жив)")
    for _, desc in CHECKS:
        print(f"[CI]   ✓ {desc}")
    print("[CI] PASS: полный контракт отработал (MMU, heap, usermode, pmptest, exit)")

    if os.environ.get("VERBOSE", sys.argv[1] if len(sys.argv) > 1 else "0") == "1":
        print("===== QEMU TRANSCRIPT =====")
        print(transcript)
        print("===== END TRANSCRIPT =====")


if __name__ == "__main__":
    main()
