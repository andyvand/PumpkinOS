#!/bin/sh
../../../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../../../tools/prcbuild -f GnuBoy.prc -t appl -c GBoy -n "GnuBoy" resources/*

