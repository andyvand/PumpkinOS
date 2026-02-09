#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
cp -f build/Launcher.so dlib0059.bin
../../tools/prcbuild -f Launcher.prc -t appl -c lnch -n "Launcher" dlib0059.bin resources/*

