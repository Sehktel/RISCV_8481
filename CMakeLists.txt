cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/riscv64-toolchain.cmake" CACHE FILEPATH "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(RISCV LANGUAGES C ASM)

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld")
set(OUTPUT_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.c" "src/*.S" "src/*.s")

if(NOT SOURCES)
    message(FATAL_ERROR "В папке '${CMAKE_CURRENT_SOURCE_DIR}/src' не найдено файлов для сборки!")
endif()

add_executable(kernel.elf ${SOURCES})

target_include_directories(kernel.elf PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_options(kernel.elf PRIVATE
    "-T${LINKER_SCRIPT}" "-nostdlib" "-nostartfiles" "-Wl,--gc-sections")

add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary "${OUTPUT_BIN_DIR}/kernel.elf" "${OUTPUT_BIN_DIR}/kernel.bin"
    COMMENT "Генерация сырого бинарника: bin/kernel.bin")
