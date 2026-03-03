#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f Preferences.prc -t appl -c pref -n "Preferences" resources/*

