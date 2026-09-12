#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f Browser.prc -t appl -c Brws -n "Browser" resources/*
