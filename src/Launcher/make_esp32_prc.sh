#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f Launcher.prc -t appl -c lnch -n "Launcher" resources/*

