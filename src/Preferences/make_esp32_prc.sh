#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
cp -f build/Preferences.so dlib0059.bin
../../tools/prcbuild -f Preferences.prc -t appl -c pref -n "Preferences" dlib0059.bin resources/*

