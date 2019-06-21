#!/bin/sh

mknod /dev/adler c 20 0
service up /service/adler -dev /dev/adler
head -c 8 /dev/adler | xargs echo
# 00000001
echo "Hello" > /dev/adler
head -c 8 /dev/adler | xargs echo
# 078b01ff
head -c 8 /dev/adler | xargs echo
# 00000001

