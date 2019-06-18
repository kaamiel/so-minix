#!/usr/bin/expect
# send the key from host to qemu
spawn ssh-copy-id root@localhost -p 10022
expect "password"
send "root\r"
interact

