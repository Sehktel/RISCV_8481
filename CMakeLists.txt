cmake_minimum_required(VERSION 3.16)

# Кросс-компиляция: не пытаемся линковать ABI-тест хостовым компилятором
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(riscv_kernel
    VERSION 0.11.0
    DESCRIPTION "RISC-V educational microkernel"
    LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ARCH_FLAGS
    -march=rv64gc
    -mabi=lp64d
    -mcmodel=medany
    -ffreestanding
    -nostdlib)

add_compile_options(${ARCH_FLAGS} -O2 -g -Wall -Wextra -Werror)
add_link_options(${ARCH_FLAGS}
    -T ${CMAKE_SOURCE_DIR}/src/linker.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/kernel.map)

# Общие заголовки — всем модулям
include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_subdirectory(src/arch)
add_subdirectory(src/drivers)
add_subdirectory(src/lib)
add_subdirectory(src/mm)
add_subdirectory(src/sched)
add_subdirectory(src/shell)

add_executable(kernel.elf src/main.c)

# Порядок важен: зависимые раньше зависимостей (классическое правило ld)
target_link_libraries(kernel.elf PRIVATE
    shell
    sched
    drivers
    klib
    mm
    arch)

# Сырой бинарник для QEMU (-bios none -kernel)
find_program(OBJCOPY riscv64-unknown-elf-objcopy REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${OBJCOPY} -O binary $<TARGET_FILE:kernel.elf> ${CMAKE_SOURCE_DIR}/bin/kernel.bin
    COMMENT "Генерация сырого бинарника: bin/kernel.bin"
    VERBATIM)
