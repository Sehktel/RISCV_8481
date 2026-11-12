# Используем стабильный Ubuntu в качестве легковесной базы
FROM ubuntu:24.04

# Исключаем интерактивные диалоги при установке пакетов
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем базовые системные утилиты сборки и DevSecOps инструменты
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    make \
    python3 \
    python3-pip \
    python3-pexpect \
    autoconf \
    automake \
    autotools-dev \
    curl \
    libmpc-dev \
    libmpfr-dev \
    libgmp-dev \
    gawk \
    bison \
    flex \
    texinfo \
    gperf \
    libtool \
    patchutils \
    bc \
    zlib1g-dev \
    libexpat1-dev \
    ninja-build \
    # Тулчейн кросс-компиляции RISC-V и отладки
    gcc-riscv64-unknown-elf \
    gdb-multiarch \
    # Эмулятор для bare-metal тестирования
    qemu-system-misc \
    # Утилиты статического анализа (SAST) для следующих этапов
    cppcheck \
    clang-tidy \
    # Инструменты автогенерации документации и графов вызовов
    doxygen \
    graphviz \
    # Очистка кэша apt для уменьшения размера Docker-образа
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию внутри контейнера
WORKDIR /workspace

# По умолчанию запускаем bash
CMD ["/bin/bash"]
