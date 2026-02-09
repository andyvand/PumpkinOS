#!/bin/sh
cp -f build/vi.so dlib0059.bin
../../tools/prcbuild -f vi.prc -t plgi -c viEd -n "vi" dlib0059.bin

