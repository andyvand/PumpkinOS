idf_component_register(
    SRCS "MemoDB.c" "MemoMain.c" "MemoRsc.c" "MemoTransfer.c"
    INCLUDE_DIRS "." "../../font" "../../libos" "../../libpit" "../../libpumpkin" "../../PalmOS/Core/System" "../../PalmOs/PalmOne" "../../PalmOs/PalmOne/Common" "../../PalmOs/PalmOne/Common/System" "../../PalmOs/PalmOne/68K" "../../PalmOs/PalmOne/68K/System" "../../PalmOS/PalmOne/Common/Libraries/filebrowser" "../../PalmOS/PalmOne/68K/Libraries/filebrowser" "../../PalmOS/Libraries/SSL" "../../PalmOS/Libraries/CPMLib" "../../PalmOS/Libraries/Pdi" "../../PalmOS/Garmin" "../../PalmOS/Libraries/PalmOSGlue" "../../PalmOS/Libraries/INet" "../../PalmOS/Core/Hardware" "../../PalmOS/Extensions/ExpansionMgr" "../../PalmOS/Extensions/Bluetooth" "../../PalmOS" "../../PalmOS/Libraries" "../../PalmOS/Core" "../../PalmOS/Core/UI" "../../PalmOS/Core/System" "../../PalmOS/Dynamic"
    REQUIRES driver fatfs esp_timer esp_wifi
)

target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -Wno-multichar -Wno-format)
