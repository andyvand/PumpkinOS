#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in the src/ directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#

#include $(IDF_PATH)/make/component_common.mk

COMPONENT_ADD_INCLUDEDIRS += ../../font ../../libpit ../../libpumpkin ../../PalmOS/Core/System ../../PalmOs/PalmOne ../../PalmOs/PalmOne/Common ../../PalmOs/PalmOne/Common/System ../../PalmOs/PalmOne/68K ../../PalmOs/PalmOne/68K/System ../../PalmOS/PalmOne/Common/Libraries/filebrowser ../../PalmOS/PalmOne/68K/Libraries/filebrowser ../../PalmOS/Libraries/SSL ../../PalmOS/Libraries/CPMLib ../../PalmOS/Libraries/Pdi ../../PalmOS/Garmin ../../PalmOS/Libraries/PalmOSGlue ../../PalmOS/Libraries/INet ../../PalmOS/Core/Hardware ../../PalmOS/Extensions/ExpansionMgr ../../PalmOS/Extensions/Bluetooth ../../PalmOS ../../PalmOS/Libraries ../../PalmOS/Core ../../PalmOS/Core/UI ../../PalmOS/Core/System ../../PalmOS/Dynamic ../../tos ../../libemulation

CFLAGS += -DSYS_CPU=1 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="\"pit\"" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="\"ESP32\"" -DSOEXT="\".so\"" -DLOGTRAP_SYS -DARMEMU
