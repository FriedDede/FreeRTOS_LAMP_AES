#!/bin/bash

cd Demo/RISC-V-Qemu-virt_GCC
make DEBUG=1
cd ../..
riscv32-unknown-elf-objdump -d Demo/RISC-V-Qemu-virt_GCC/build/RTOSDemo.axf > disam.txt