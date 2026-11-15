# ==============================================================================
# Makefile — RISC-V OS (DevSecOps)
#
# Архитектура сборки:
#   Весь тулчейн (CMake, clang-format, cppcheck, Doxygen) живёт в контейнере
#   riscv_os_devsecops. Атомарные цели (секция 1) выполняются ВНУТРИ контейнера,
#   поэтому на хосте достаточно только Docker, QEMU и GDB.
# ==============================================================================


# Абсолютный путь к docker: хуки и CI могут запускать make
# с урезанным PATH, поэтому имя бинаря не ищем через PATH.
DOCKER_BIN := $(shell command -v docker 2>/dev/null)
ifeq ($(strip $(DOCKER_BIN)),)
DOCKER_BIN := /usr/bin/docker
endif


.PHONY: all build style sast format clean clear \
        docker-build docker-clear docker-format docker-style docker-sast \
        run debug gdb

# Базовый конвейер: чистая сборка в контейнере и запуск в QEMU
all: docker-clear build run

# Единая команда запуска контейнера с правами текущего пользователя
DOCKER_RUN = docker run --rm \
	--user $$(id -u):$$(id -g) \
	-v $$(pwd):/workspace \
	-w /workspace \
	riscv_os_devsecops:latest

# ==============================================================================
# 1. АТОМАРНЫЕ ЦЕЛИ (всегда выполняются внутри контейнера)
# ==============================================================================

## Сборка ядра (CMake + кросс-тулчейн RISC-V)
build:
	$(DOCKER_RUN) sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"

## Статический анализ кода (cppcheck)
sast:
	$(DOCKER_RUN) cppcheck --enable=warning,style,performance,portability \
	         --platform=unix64 \
	         --library=.infrastructure/cppcheck/kernel.cfg \
	         --error-exitcode=1 \
	         --suppress=missingIncludeSystem \
	         --suppress=unusedFunction \
	         --inline-suppr \
	         src/

## Проверка стиля кода (clang-format, только проверка)
style:
	$(DOCKER_RUN) sh -c "find src/ $ -name '*.c' -o -name '*.h' $ | xargs clang-format --dry-run --Werror"

## Автоматическое форматирование кода (clang-format)
format:
	$(DOCKER_RUN) sh -c "find src/ $ -name '*.c' -o -name '*.h' $ | xargs clang-format -i"

## Очистка артефактов сборки
clean:
	$(DOCKER_RUN) sh -c "rm -rf build bin/*"

clear: clean

# ==============================================================================
# 2. ПСЕВДОНИМЫ ДЛЯ СОВМЕСТИМОСТИ (docker-*)
# ==============================================================================

docker-build: build
docker-style: style
docker-sast: sast
docker-format: format
docker-clear: clean

# ==============================================================================
# 3. ГЕНЕРАЦИЯ ДОКУМЕНТАЦИИ
# ==============================================================================

## HTML-документация и графы вызовов (Doxygen)
docs:
	@mkdir -p docs/generated
	$(DOCKER_RUN) doxygen docs/Doxyfile
	@echo "Документация и Call-графы сгенерированы в папке: docs/generated/html/index.html"

# ==============================================================================
# 4. ЭМУЛЯЦИЯ И ОТЛАДКА НА ХОСТЕ (QEMU / GDB)
# ==============================================================================

## Запуск ядра в QEMU
run:
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

## Запуск QEMU в режиме ожидания GDB (порт 1234)
debug:
	@echo "========================================================="
	@echo "QEMU ожидает GDB на порту 1234. Откройте новый терминал и выполните: make gdb"
	@echo "========================================================="
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic -s -S

## Подключение gdb-multiarch к запущенному QEMU
gdb:
	gdb-multiarch -ex "target remote localhost:1234" bin/kernel.elf
