#!/bin/sh

/sbin/mkfs.mfs /dev/c0d1
mkdir /root/mounted-drive
mount /dev/c0d1 /root/mounted-drive

