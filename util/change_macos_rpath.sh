#!/bin/sh
for i in bin/* pumpkin; do install_name_tool -change ../../bin/libpit.dylib @rpath/libpit.dylib $i; done
for i in bin/* pumpkin; do install_name_tool -change ../../bin/libpumpkin.dylib @rpath/libpumpkin.dylib $i; done
install_name_tool -add_rpath @executable_path pumpkin
