FREERTOS for LAMP, scripts user guide:

-> make_qemu.sh 
    build for simulation in QEMU
-> make_lamp.sh
    build for lamp
    genrate:
        elf             @ Build/RTOSDemo.axf
        disassembled    @ RTOSDemo.axf_disam.txt
        vmem file       @ RTOSDemo.axf.vmem
-> clean.sh
    clean build files
-> run.sh 
    run qemu simulation
-> debug_run.sh 
    run qemu simulation with gdb debug stub
    to use gdb run the script, then open another terminal and run:
    riscv32-unknown-elf-gdb -x gdb_cmd.txt

COMPILER SETUP:
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
    cd riscv-gnu-toolchain
    ./configure --prefix=($path to "FreeRTOS/toolchain/riscv") --with-arch=rv32g --with-abi=ilp32
    make

    then you need to add $prefix/bin to PATH