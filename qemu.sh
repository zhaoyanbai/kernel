#!/bin/bash
#
# 检查serial_monitor进程是否在运行
process_name="serial_monitor"
if ! pgrep -x "$process_name" > /dev/null
then
    echo "process $process_name is not running."
    exit 1
fi

echo "process $process_name is running."


qemu-system-i386 \
    -boot d \
    -serial tcp::6666,server,nowait \
    -drive file=HDa.IMG,format=raw,index=0,media=disk \
    -drive file=kernel.iso,index=1,media=cdrom \
    -drive file=HDb.IMG,format=raw,index=2,media=disk \
    -name kernel \
    -device ich9-ahci,id=ahci \
    -vga std \
    -display cocoa \
    -s -S \
    &

    #-serial mon:stdio \
    #-serial tcp::12345,server,nowait \
#   -serial tcp::8888,server,nowait \
#    -device ich9-ahci,id=ahci \
#    -machine accel=tcg \
#    -serial stdio \
#qemu-system-x86_64 -boot d -s -S -drive file=HD.IMG,format=raw,index=0,media=disk -drive file=kernel.iso,index=1,media=cdrom &
#

# i386-elf-gdb KERNEL.ELF -x gdbscript

pid=$!
echo "pid is ${pid}"

i386-elf-gdb KERNEL.ELF -x gdbscript; kill -9 $pid

echo "kill pid ${pid}"

#x86_64-elf-gdb -x gdbscript; kill -9 $pid


# qemu-system-i386 -drive file=HD.IMG,format=raw,index=0,media=disk -cdrom kernel.iso


# -cdrom kernel.iso
# -drive file=kernel.iso,index=2,media=cdrom

# connect a CDROM to the slave of ide0
# -drive if=ide,index=1,media=cdrom

# -hda,-hdb,-hdc,-hdd
# -drive file=file,index=0,media=disk
# -drive file=file,index=1,media=disk
# -drive file=file,index=2,media=disk
# -drive file=file,index=3,media=disk


# -s shorthand for -gdb tcp::1234
# -S freeze CPU at startup (use 'c' to start execution)
