#!/bin/bash
qemu-system-riscv32 -nographic -machine virt -net none \
  -chardev stdio,id=con,mux=on -serial chardev:con \
  -mon chardev=con,mode=readline -bios none \
  -smp 1 -kernel ./Demo/RISC-V-Qemu-virt_GCC/build/RTOSDemo.axf -s -S 