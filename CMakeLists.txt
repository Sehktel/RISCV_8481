cmake_minimum_required(VERSION 3.16)

project(RISCV_8481 LANGUAGES C ASM)

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
