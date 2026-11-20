set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(TOOLCHAIN_PREFIX riscv64-unknown-elf-)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump)

# bare-metal: проверочные бинарники CMake падают без линкера
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Целевая платформа: QEMU virt (этапы 11–18).
# Без F/D: FP-контекст в trap_frame не сохраняется — запрещаем компилятору
# генерировать FP-инструкции целиком. Для этапа 19 (C906) пересматриваем осознанно.
set(ARCH_FLAGS "-march=rv64imac_zicsr -mabi=lp64 -mcmodel=medany")

set(CMAKE_C_FLAGS "${ARCH_FLAGS} -ffreestanding -fno-builtin -ffunction-sections -fdata-sections -Wall -Wextra -Werror=implicit-function-declaration -O0 -g" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${ARCH_FLAGS} -x assembler-with-cpp -g" CACHE STRING "" FORCE)
