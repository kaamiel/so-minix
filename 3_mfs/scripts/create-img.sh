#!/bin/bash

qemu-img create -f qcow2 -o backing_file=../src/minix.img minix.img

