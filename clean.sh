#!/bin/bash
BENCH_NAME="RTOSDemo.axf"

rm ${BENCH_NAME}*

cd Demo/RISC-V-Qemu-virt_GCC
make clean
