.PHONY: all clean run debug gdb

# Прокси-таргет по умолчанию: конфигурирует CMake и собирает проект
all:
	@mkdir -p build
	@if [ ! -f build/CMakeCache.txt ]; then \
		cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .; \
	fi
	@cmake --build build

# Запуск ядра в QEMU без графики (вывод идет напрямую в консоль)
run: all
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

# Запуск QEMU в режиме ожидания отладчика (порт :1234)
debug: all
	@echo "========================================================="
	@echo "QEMU запущен в режиме отладки и ожидает GDB на порту 1234"
	@echo "Для подключения откройте новый терминал и выполните: make gdb"
	@echo "========================================================="
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic -s -S

# Быстрый запуск отладчика gdb-multiarch с автоподключением к QEMU
gdb:
	gdb-multiarch -ex "target remote localhost:1234" bin/kernel.elf

# Полная очистка временных файлов CMake и скомпилированных бинарников
clean:
	rm -rf build
	rm -f bin/kernel.elf bin/kernel.bin
