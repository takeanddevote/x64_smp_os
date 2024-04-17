#!/bin/bash
has_k_option="false" # kill qemu进程，qemu如果添加-nographic选项，则无法通过ctr+c结束qemu。
has_s_option="false" # 重新编译内核，并启动qemu server。

while getopts ":ks" opt; do
    case $opt in
        k)
            has_k_option="true"
            ;;
        s)
            has_s_option="true"
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            ;;
    esac
done

if $has_k_option == "true"; then
    pid=$(ps -aux | grep 'qemu-system' | grep -v 'grep' | awk '{print $2}')
    if [ -n "$pid" ]; then
        echo "kill qemu $pid success..."
        kill $pid
    else
        echo "qemu not running..."
    fi
elif $has_s_option == "true"; then
    make gdbqemu
else
    gdb -ex "target remote localhost:1234" -ex "b printk" -ex "b printk.outStr" -ex "c" .build/vmlinux_x64
fi



