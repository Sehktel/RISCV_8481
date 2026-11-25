# ==============================================================================
# Makefile — RISC-V OS (DevSecOps)
#
# Архитектура сборки:
#   Весь тулчейн (CMake, clang-format, cppcheck, Doxygen) живёт в контейнере
#   riscv_os_devsecops. На хосте атомарные цели исполняются через контейнер,
#   внутри CI-контейнера (act/Gitea Actions) — нативно, потому что docker
#   внутри контейнера недоступен.
# ==============================================================================

# ---------------------------------------------------------------
# Режим исполнения атомарных целей.
# Переменная CI выставляется act/Gitea Actions автоматически;
# внутри job-контейнера DOCKER_RUN пуст и команды идут напрямую.
# ---------------------------------------------------------------
ifneq ($(CI),)
DOCKER_RUN =
else
DOCKER_BIN := $(shell command -v docker 2>/dev/null)
ifeq ($(strip $(DOCKER_BIN)),)
DOCKER_BIN := /usr/bin/docker
endif

DOCKER_RUN = $(DOCKER_BIN) run --rm \
	--user $$(id -u):$$(id -g) \
	-v $$(pwd):/workspace \
	-w /workspace \
	riscv_os_devsecops:latest
endif

SOURCES := $(shell find src -name '*.c' -not -path '*/.*')
HEADERS := $(shell find src -name '*.h' -not -path '*/.*')

GITLEAKS_CONFIG = --source . --config .gitleaks.toml
GITLEAKS_ARGS = --source . --config .gitleaks.toml --platform gitea --verbose --redact

.PHONY: all \
		clean clear docker-clear\
		build       docker-build  \
		format      docker-format \
		style       docker-style \
		tidy        docker-tidy \
		sast        docker-sast \
		secrets-staged \
		secrets     docker-secrets \
		docs        docker-docs
		run debug gdb

# Базовый конвейер: чистая сборка в контейнере и запуск в QEMU
all: docker-clear build run

# ==============================================================================
# 1. АТОМАРНЫЕ ЦЕЛИ
# ==============================================================================

## Сборка ядра (CMake + кросс-тулчейн RISC-V)
build:
	$(DOCKER_RUN) sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"

## Проверка стиля кода (clang-format, только проверка)
style:
	$(DOCKER_RUN) sh -c "clang-format --dry-run --Werror $(SOURCES) $(HEADERS)"

## Автоматическое форматирование кода (clang-format)
format:
	$(DOCKER_RUN) sh -c "clang-format -i $(SOURCES) $(HEADERS)"

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

## Глубокий статический анализ кода (clang-tidy: bugprone-*, cert-*)
tidy:
	$(DOCKER_RUN) sh -c "clang-tidy $(SOURCES) \
	         --warnings-as-errors='*' \
	         -header-filter='^/workspace/src/' \
	         -- \
	         --target=riscv64-unknown-elf \
	         -march=rv64gc \
	         -mabi=lp64d \
	         -ffreestanding \
	         -nostdlib \
	         -Isrc \
	         -Isrc/include"

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
docker-tidy: tidy
docker-secret: secret
docker-docs: docs

# ==============================================================================
# 3. ГЕНЕРАЦИЯ ДОКУМЕНТАЦИИ
# ==============================================================================

## HTML-документация и графы вызовов (Doxygen)
docs:
	@mkdir -p docs/generated
	$(DOCKER_RUN) doxygen docs/Doxyfile
	@echo "Документация и Call-графы сгенерированы в папке: docs/generated/html/index.html"

## Аудит всей истории Git на наличие секретов (для CI)
secrets:
	$(DOCKER_RUN) sh -c "HOME=/tmp git config --global --add safe.directory '*' && HOME=/tmp gitleaks detect $(GITLEAKS_CONFIG) --platform gitea --verbose --redact"

## Аудит только подготовленных к коммиту изменений (для pre-commit хука)
secrets-staged:
	$(DOCKER_RUN) sh -c "HOME=/tmp git config --global --add safe.directory '*' && HOME=/tmp gitleaks protect --staged $(GITLEAKS_CONFIG) --verbose --redact"

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
