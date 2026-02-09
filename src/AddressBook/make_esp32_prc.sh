#!/bin/sh
cp -f build/AddressBook.so dlib0059.bin
../../tools/prcbuild -f AddressBook.prc -t appl -c addr -n "Address Book" dlib0059.bin resources/*

