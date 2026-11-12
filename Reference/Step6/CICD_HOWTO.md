```.clang-format
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 120
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: WithoutElse
AllowShortLoopsOnASingleLine: false
PointerAlignment: Left
```

```Dockerfile
    # Утилиты статического анализа и стиля (SAST / Code Style)
    cppcheck \
    clang-tidy \
    clang-format \
```

```Makefile
.PHONY: all clean clear run debug gdb docker-build docker-clear docs format

# Автоматическое форматирование всего Си-кода внутри Docker
format:
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format -i"
	@echo "Код в папке src/ успешно отформатирован по стандарту проекта."
```

pre-commit
```bash
# 1.5 Проверка стиля Си-кода (Code Style Gate)
C_FILES=$(git diff --cached --name-only --diff-filter=d | grep -E '\.(c|h)$')
if [ ! -z "$C_FILES" ]; then
    echo "Проверка стиля кода (clang-format)..."
    docker run --rm \
        --user $(id -u):$(id -g) \
        -v "$REPO_ROOT":/workspace \
        riscv_os_devsecops:latest \
        clang-format --dry-run --Werror $C_FILES

    if [ $? -ne 0 ]; then
        echo "Ошибка стиля кода!"
        echo "Выполните 'make format' для автоматического исправления и повторите коммит."
        exit 1
    fi
fi
```