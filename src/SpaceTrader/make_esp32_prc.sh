#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f SpaceTrader.prc -t appl -c STra -n "SpaceTrader" resources/*

