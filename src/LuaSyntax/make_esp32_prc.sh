#!/bin/sh
cp -f build/LuaSyntax.so dlib0059.bin
../../tools/prcbuild -f LuaSyntax.prc -t plgi -c LuaS -n "LuaSyntax" dlib0059.bin

