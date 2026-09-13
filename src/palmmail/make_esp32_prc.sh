#!/bin/sh
# Builds the resources-only Mail.prc used by the ESP32 firmware (the code is
# compiled into the firmware) and copies it into the LittleFS image source tree.
set -e
cd "$(dirname "$0")"
../../tools/pilrc -q -allowLargeRes resource.rcp resources
touch res.flag
../../tools/prcbuild -f Mail_esp32.prc -t appl -c PMal -n "Mail" resources/*
cp Mail_esp32.prc ../data/vfs/app_install/Mail.prc
echo "installed ../data/vfs/app_install/Mail.prc"
