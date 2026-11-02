#!/bin/bash
sudo apt update
sudo apt install -y gcc-riscv64-unknown-elf
sudo apt install -y qemu-system-misc
sudo apt install -y autoconf automake autotools-dev curl python3 python3-pip python3-tomli libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev meson ninja-build git cmake libglib2.0-dev expect device-tree-compiler libslirp-dev libzstd-dev libncurses-dev 
sudo apt install -y gdb-multiarch