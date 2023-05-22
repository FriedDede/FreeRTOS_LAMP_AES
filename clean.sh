#!/bin/bash
BENCH_NAME="RTOSDemo"

rm ${BENCH_NAME}*

cd Demo/RISC-V-Qemu-virt_GCC
make clean
