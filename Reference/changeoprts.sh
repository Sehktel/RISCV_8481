#!/usr/bin/env bash

# Укажите путь к директории (по умолчанию — текущая директория)
TARGET_DIR="${1:-.}"

if [ ! -d "$TARGET_DIR" ]; then
    echo "Ошибка: директория '$TARGET_DIR' не найдена."
    exit 1
fi

# Ищем все .yml и .yaml файлы и обрабатываем их через встроенный Python
find "$TARGET_DIR" -type f \( -name "*.md" -o -name "*.yaml" \) -print0 | while IFS= read -r -d '' file; do
    python3 - "$file" << 'EOF'
import sys
import re

filepath = sys.argv[1]

with open(filepath, 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
in_ports = False
ports_indent = 0
changed = False

# Регулярка ищет элементы списка внутри ports:
# Группа 1: отступ + дефис + пробелы + опциональная кавычка (" или ')
# Группа 2: порт или диапазон (например, 8080:80 или 8000-8010:8000-8010)
# Группа 3: остаток строки (закрывающая кавычка, /udp, комментарии и т.д.)
pattern = re.compile(r'^(\s*-\s*["\']?)(\d+(?:-\d+)?:\d+(?:-\d+)?)(.*)$')

for line in lines:
    stripped = line.lstrip()

    # Пропускаем пустые строки и комментарии, сохраняя контекст блока ports
    if not stripped or stripped.startswith('#'):
        new_lines.append(line)
        continue

    indent = len(line) - len(stripped)

    # Определяем начало секции ports:
    if re.match(r'^ports\s*:', stripped):
        in_ports = True
        ports_indent = indent
        new_lines.append(line)
        continue

    # Если отступ меньше или равен отступу самого ключа ports:, значит секция закончилась
    if in_ports and indent <= ports_indent:
        in_ports = False

    # Если мы внутри секции ports, проверяем формат записи порта
    if in_ports:
        match = pattern.match(line)
        if match:
            line = f"{match.group(1)}127.0.0.1:{match.group(2)}{match.group(3)}\n"
            changed = True

    new_lines.append(line)

if changed:
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)
    print(f"[ОБНОВЛЕНО] {filepath}")
EOF
done

echo "Готово!"
