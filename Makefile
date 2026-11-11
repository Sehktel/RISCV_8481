.PHONY: all clean clear run debug gdb docker-build docker-clear


all: docker-clear docker-build run
# # Сборка по умолчанию: автоматически вызывает ваш CMake локально
# all:
# 	@mkdir -p build
# 	@if [ ! -f build/CMakeCache.txt ]; then \
# 		cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .; \
# 	fi
# 	@cmake --build build

# Безопасная сборка ядра внутри Docker-образа с помощью вашего CMake
docker-build:
	@mkdir -p build bin
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"

# Запуск скомпилированного ядра в QEMU
run:
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

# Запуск QEMU в режиме отладки (замораживает процессор и ждет GDB на порту 1234)
debug:
	@echo "========================================================="
	@echo "QEMU ожидает GDB на порту 1234. Откройте новый терминал и выполните: make gdb"
	@echo "========================================================="
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic -s -S

# Подключение отладчика gdb-multiarch к QEMU
gdb:
	gdb-multiarch -ex "target remote localhost:1234" bin/kernel.elf

# Очистка локального кэша CMake и папки сборки
clean:
	rm -rf build
	rm -f bin/kernel.elf bin/kernel.bin

clear: clean

# Очистка кэша сборки внутри Docker-контейнера
docker-clear:
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		rm -rf build bin/*
