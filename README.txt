FREERTOS LAMP, SCRIPTS GUIDE;

-> make_qemu.sh 
    build for simulation in QEMU
-> make.sh
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
    to use gdb run the scrip, then open another terminal and run:
    riscv32-unknown-elf-gdb -x gdb_cmd.txt