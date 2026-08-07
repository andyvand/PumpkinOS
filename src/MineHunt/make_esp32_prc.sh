#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp bmp
touch res.flag
../../tools/prcbuild -f MineHunt.prc -t appl -c mine -n "MineHunt" bmp/*
