#!/bin/sh
cp -f build/MemoPad.so dlib0059.bin
../../tools/prcbuild -f MemoPad.prc -t appl -c memo -n "Memo Pad" dlib0059.bin resources/*

