#!/bin/bash
cd Demo/RISC-V-Qemu-virt_GCC
make -j16 DEBUG=1
cd ../..

BENCH_NAME="RTOSDemo.axf"
BENCH_FOLDER="Build/"
CC_STRIP="riscv32-unknown-elf-strip"
CC_OBJCOPY="riscv32-unknown-elf-objcopy"
CC_OBJDUMP="riscv32-unknown-elf-objdump"

# objdump
    ${CC_OBJDUMP} -d ${BENCH_FOLDER}${BENCH_NAME} > ${BENCH_NAME}_disam.txt

# Remove debug section
    ${CC_STRIP} --strip-all -o ${BENCH_NAME}_strpd.elf ${BENCH_FOLDER}${BENCH_NAME}

# Produce temporary vmem file from object file with reversed bytes within the word
    echo "Generating .vmem file ..."
    ${CC_OBJCOPY} ${BENCH_NAME}_strpd.elf ${BENCH_NAME}.vmem.tmp -O verilog \
    --remove-section=.comment \
    --remove-section=.sdata \
    --remove-section=.riscv.attributes \
    --reverse-bytes=4

# Format .vmem file to be compliant with polimi riscv core requirements
    echo "Formatting .vmem file ..."
    if [ -f vmem_formatter ]; then
        ./vmem_formatter ${BENCH_NAME}.vmem.tmp ${BENCH_NAME}.vmem
    else
        echo "[ERROR] vmem_formatter not found. Place it in the current working directory."
        exit 1;
    fi

    echo "Removing temporary files ..."
    rm ${BENCH_NAME}.vmem.tmp

    echo "Done."