#!/bin/sh

cd /
patch -p1 < kd370826.patch || echo patching failed

cd /usr/src/minix/fs/mfs
make && make install &&
echo "rebooting..." && reboot

