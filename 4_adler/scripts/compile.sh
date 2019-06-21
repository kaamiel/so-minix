#!/bin/sh

cd /usr/src/minix/drivers/adler

make clean
make
make install

service up /service/adler
service update /service/adler
service down adler

