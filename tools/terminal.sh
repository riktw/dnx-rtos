#!/usr/bin/env bash

if [ "$1" == "" ]; then
    dev=/dev/ttyACM0
else
    dev=$1
fi

minicom -C /tmp/minicom.log -D "$dev"  -b 115200
