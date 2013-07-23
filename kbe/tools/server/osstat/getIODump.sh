#!/bin/sh

dmesg -c
#/etc/init.d/klogd stop
echo 1 > /proc/sys/vm/block_dump

# allow 30 seconds of stats to be logged
sleep $1

dmesg -c | perl iodump

echo 0 > /proc/sys/vm/block_dump
#/etc/init.d/klogd start

