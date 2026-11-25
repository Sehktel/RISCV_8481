## `CMakeLists.txt`

**Строк: 52**

```cmake
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

# Сырой бинарник для QEMU (-bios none -kernel) и ELF для GDB — оба в bin/
find_program(OBJCOPY riscv64-unknown-elf-objcopy REQUIRED)
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    COMMAND ${OBJCOPY} -O binary $<TARGET_FILE:kernel.elf> ${CMAKE_SOURCE_DIR}/bin/kernel.bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf> ${CMAKE_SOURCE_DIR}/bin/kernel.elf
    COMMENT "Генерация артефактов: bin/kernel.bin и bin/kernel.elf"
    VERBATIM)
```

## `src/drivers/CMakeLists.txt`

**Строк: 3**

```cmake
add_library(drivers STATIC
    uart.c
    timer.c)
```

## `src/mm/CMakeLists.txt`

**Строк: 2**

```cmake
add_library(mm STATIC
    heap.c)
```

## `src/arch/CMakeLists.txt`

**Строк: 4**

```cmake
add_library(arch STATIC
    csr.c
    pmp.c
    start.S)
```

## `src/lib/CMakeLists.txt`

**Строк: 3**

```cmake
add_library(klib STATIC
    console.c
    string.c)
```

## `src/sched/CMakeLists.txt`

**Строк: 2**

```cmake
add_library(sched STATIC
    sched.c)
```

## `src/shell/CMakeLists.txt`

**Строк: 2**

```cmake
add_library(shell STATIC
    shell.c)
```
