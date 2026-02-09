
if(CONFIG_ELF_LOADER)
    set(srcs  "src/esp_elf_symbol.c"
              "src/esp_elf.c"
              "src/esp_elf_adapter.c"
              "../libpit/telnet.c"
              "../libpit/filter.c"
              "../libpumpkin/syntax.c"
              "../libpumpkin/build.c"
              "../libpumpkin/peditor.c"
              "../libpumpkin/edit.c")

    if(CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS)
        list(APPEND srcs "src/esp_all_symbol.c")
    endif()

    if(CONFIG_IDF_TARGET_ARCH_XTENSA)
        list(APPEND srcs "src/arch/esp_elf_xtensa.c")

        # ESP32-S2 need to set MMU to run ELF
        if(CONFIG_IDF_TARGET_ESP32S2 AND (CONFIG_ELF_LOADER_LOAD_PSRAM))
            list(APPEND srcs "src/soc/esp_elf_esp32s2.c")
        endif()
    elseif(CONFIG_IDF_TARGET_ARCH_RISCV)
        list(APPEND srcs "src/arch/esp_elf_riscv.c")
    endif()

    if(CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT)
        list(APPEND srcs "src/dlso/dlfcn.c"
                         "src/dlso/dlmod.c")
    endif()

    set(include_dirs "include" "../../libpumpkin" "../../libpit" "../../font" "../../PalmOS/Core/System" "../../PalmOs/PalmOne" "../../PalmOs/PalmOne/Common" "../../PalmOs/PalmOne/Common/System" "../../PalmOs/PalmOne/68K" "../../PalmOs/PalmOne/68K/System" "../../PalmOS/PalmOne/Common/Libraries/filebrowser" "../../PalmOS/PalmOne/68K/Libraries/filebrowser" "../../PalmOS/Libraries/SSL" "../../PalmOS/Libraries/CPMLib" "../../PalmOS/Libraries/Pdi" "../../PalmOS/Garmin" "../../PalmOS/Libraries/PalmOSGlue" "../../PalmOS/Libraries/INet" "../../PalmOS/Core/Hardware" "../../PalmOS/Extensions/ExpansionMgr" "../../PalmOS/Extensions/Bluetooth" "../../PalmOS" "../../PalmOS/Libraries" "../../PalmOS/Core" "../../PalmOS/Core/UI" "../../PalmOS/Core/System" "../../PalmOS/Dynamic")
    set(ldfragments "linker.lf")
endif()

set(priv_req spi_flash)

if(CONFIG_IDF_TARGET_ESP32P4)
    list(APPEND priv_req esp_mm)
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${include_dirs}
                       PRIV_REQUIRES ${priv_req}
                       LDFRAGMENTS ${ldfragments})

include(package_manager)
if(CONFIG_ELF_LOADER)
    cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -DESP32 -Wno-multichar -Wno-format)
