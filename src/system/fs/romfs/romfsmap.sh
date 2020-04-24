#!/usr/bin/env bash

root=$(pwd)
data=../../../../build/romfs

mkdir -p res/romfs

cd $(dirname $0)

rm -rf "${data}"
mkdir -p "${data}"

/usr/bin/python romfsmap.py "${root}/res/romfs" "${data}"
