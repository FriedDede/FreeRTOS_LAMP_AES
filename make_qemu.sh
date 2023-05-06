#!/bin/bash
cd Demo/RISC-V-Qemu-virt_GCC
make -j16 DEBUG=1 QEMU=1
cd ../..

BENCH_NAME="RTOSDemo.axf"
BENCH_FOLDER="Build/"
CC_STRIP="riscv32-unknown-elf-strip"
CC_OBJCOPY="riscv32-unknown-elf-objcopy"
CC_OBJDUMP="riscv32-unknown-elf-objdump"

# objdump
    ${CC_OBJDUMP} -d ${BENCH_FOLDER}${BENCH_NAME} > ${BENCH_NAME}_disam.txt