#!/bin/bash
gdb -ex "target remote localhost:1234" -ex "b kernel_start" -ex "c" build/init/kernel.elf
