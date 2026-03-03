#!/bin/sh
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f Command.prc -t appl -c CmdP -n "Command" resources/*

