#!/bin/sh
cp -f build/DateBook.so dlib0059.bin
../../tools/prcbuild -f DateBook.prc -t appl -c date -n "Date Book" dlib0059.bin resources/*

