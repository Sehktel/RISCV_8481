# CMakefile

```cmake
cmake_minimum_required(VERSION 3.16)

project(RISCV LANGUAGES C ASM)

# Указываем ТОЧНЫЙ путь к вашему скрипту линкера
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld")

# Задаем абсолютный путь к целевой директории вывода (коренная папка bin/)
set(OUTPUT_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Перенаправляем вывод исполняемых файлов (.elf) напрямую в bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}")
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}")

# Автоматически ищем исходники в папке src/
file(GLOB_RECURSE SOURCES
    "src/*.c"
    "src/*.S"
    "src/*.s"
)

# Проверка: если ничего не нашли, выведем понятную ошибку
if(NOT SOURCES)
    message(FATAL_ERROR "Критическая ошибка: В папке '${CMAKE_CURRENT_SOURCE_DIR}/src' не найдено файлов для сборки!")
endif()

# Создаем исполняемый ELF-файл ядра (он скомпилируется в bin/kernel.elf)
add_executable(kernel.elf ${SOURCES})

# Настройки линкера для bare-metal с использованием вашего src/linker.ld
target_link_options(kernel.elf PRIVATE
    "-T${LINKER_SCRIPT}"
    "-nostdlib"
    "-nostartfiles"
    "-Wl,--gc-sections"
)

# Включаем пути поиска заголовочных файлов
target_include_directories(kernel.elf PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Автоматическая генерация сырого бинарника (.bin) строго в папке bin/
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary "${OUTPUT_BIN_DIR}/kernel.elf" "${OUTPUT_BIN_DIR}/kernel.bin"
    COMMENT "Генерация сырого бинарника: bin/kernel.bin"
)
```

# cmake/riscv64-toolchain.cmake

```
#set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Задаем префикс кросс-компилятора
set(TOOLCHAIN_PREFIX riscv64-unknown-elf-)

# Пути к компиляторам
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump)

# Защита от проверочных бинарников CMake (они падают без линкера на bare-metal)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Общие флаги архитектуры (для XuanTie C906 / Milk-V Duo)
set(ARCH_FLAGS "-march=rv64gc -mabi=lp64d -mcmodel=medany")

set(CMAKE_C_FLAGS   "${ARCH_FLAGS} -ffreestanding -Wall -Wextra -O0 -g" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${ARCH_FLAGS} -x assembler-with-cpp -g" CACHE STRING "" FORCE)
```

# Вывод

Проект переведен на CMake сборку.
Предыдущий Makefile сохранен с постфиксом _prev