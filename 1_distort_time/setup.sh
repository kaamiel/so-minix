#!/bin/sh

patch -p1 < distort-time.patch &&
echo "patch -p1 < distort-time.patch done." &&

make -C /usr/src/minix/servers/pm &&
echo "make -C /usr/src/minix/servers/pm done." &&
make install -C /usr/src/minix/servers/pm &&
echo "make install -C /usr/src/minix/servers/pm done." &&

make -C /usr/src/lib/libc &&
echo "make -C /usr/src/lib/libc done." &&
make install -C /usr/src/lib/libc &&
echo "make install -C /usr/src/lib/libc done." &&

make hdboot -C /usr/src/releasetools &&
echo "make hdboot -C /usr/src/releasetools done." &&

echo "rebooting..." &&
reboot

