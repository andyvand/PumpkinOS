#!/bin/sh
../../tools/pilrc -q -allowLargeRes boot.rcp resources
touch res.flag
../../tools/prcbuild -f BOOT.prc -t boot -c BOOT -n "BOOT" resources/*

