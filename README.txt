FREERTOS for LAMP:
    Clone with --recursive to collect all the submodules

scripts user guide:

-> make_qemu.sh 
    build for simulation in QEMU
-> make_lamp.sh <CPU_ID>
    build for lamp core #CPU_ID
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
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git (not necessary if the repo has been cloned with --recursive)
    cd riscv-gnu-toolchain
    ./configure --prefix=($path to "FreeRTOS/toolchain/riscv") --with-arch=rv32g --with-abi=ilp32
    make

    then you need to add $prefix/bin to PATH
    read the readme in the riscv-gnu-toolchain repo for any doubt about those commands