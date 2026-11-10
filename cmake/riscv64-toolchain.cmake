set(CMAKE_SYSTEM_NAME Generic)
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
