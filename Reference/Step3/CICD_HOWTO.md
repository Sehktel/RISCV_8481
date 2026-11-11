```Dockerfile

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
    # Очистка кэша apt для уменьшения размера Docker-образа
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию внутри контейнера
WORKDIR /workspace

# По умолчанию запускаем bash
CMD ["/bin/bash"]

```

```Makefile
# Безопасная сборка ядра внутри Docker-образа с помощью вашего CMake
docker-build:
	@mkdir -p build bin
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"


clear: clean

# Очистка кэша сборки внутри Docker-контейнера
docker-clear:
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		rm -rf build bin/*
```

main.c
```c
int main(void) {
    print("Hello World!\n");
    asm volatile("ebreak");

    // Автоматически тушим QEMU после завершения работы программы
    poweroff();

    return 0;
}
```

```bash
#!/bin/bash
sudo apt update
sudo apt install -y gcc-riscv64-unknown-elf
sudo apt install -y qemu-system-misc
sudo apt install -y autoconf automake autotools-dev curl python3 python3-pip python3-tomli libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev meson ninja-build git cmake libglib2.0-dev expect device-tree-compiler libslirp-dev libzstd-dev libncurses-dev
sudo apt install -y gdb-multiarch
sudo apt-get install -y dbus-user-session uidmap
sudo systemctl disable --now docker.service docker.socket
sudo apt-get install -y docker-doc uidmap

```