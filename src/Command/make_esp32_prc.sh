#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
cp -f build/Command.so dlib0059.bin
../../tools/prcbuild -f Command.prc -t appl -c CmdP -n "Command" dlib0059.bin resources/*

