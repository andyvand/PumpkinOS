#!/bin/sh
cp -f build/ToDoList.so dlib0059.bin
../../tools/prcbuild -f ToDoList.prc -t appl -c todo -n "To Do List" dlib0059.bin resources/*

