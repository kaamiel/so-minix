#!/bin/bash

qemu-system-x86_64 -curses -drive file=minix.img -enable-kvm -localtime -net user,hostfwd=tcp::10022-:22 -net nic,model=virtio -drive file=extra.img,format=raw,index=1,media=disk -m 1024M

