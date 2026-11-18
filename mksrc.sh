#!/usr/bin/env bash
set -euo pipefail

dirs=(
  src/include
  src/arch
  src/drivers
  src/lib
  src/mm
  src/sched
  src/shell
)

files=(
  src/include/types.h
  src/include/riscv.h
  src/include/uart.h
  src/include/string.h
  src/include/mm.h
  src/include/sched.h
  src/include/shell.h
  src/arch/start.S
  src/arch/pmp.c
  src/drivers/uart.c
  src/lib/string.c
  src/mm/heap.c
  src/sched/sched.c
  src/shell/shell.c
  src/main.c
  src/linker.ld
)

for d in "${dirs[@]}"; do
  mkdir -p "$d"
done

# for f in "${files[@]}"; do
#   # не затираем существующие файлы, просто создаём пустые
#   [ -e "$f" ] || : > "$f"
# done
for f in "${files[@]}"; do
  if [[ "$f" == *.h ]] && [ ! -e "$f" ]; then
    guard=$(echo "${f#src/include/}" | tr 'a-z./' 'A-Z__')
    printf '#ifndef %s\n#define %s\n\n#endif /* %s */\n' "$guard" "$guard" "$guard" > "$f"
  elif [ ! -e "$f" ]; then
    : > "$f"
  fi
done

echo "Структура создана:"
find src -type f | sort
