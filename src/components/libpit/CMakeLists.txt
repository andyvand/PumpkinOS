idf_component_register(
    SRCS "../../main/script.c" "main.c" "sig.c" "sys.c" "math.c" "malloc.c" "strtol.c" "strtoul.c" "ctype.c" "bsearch.c" "printf.c" "string.c" "ptr.c" "debug.c" "builtin.c" "list.c" "sock.c" "io.c" "loadfile.c" "util.c" "bytes.c" "ts.c" "yuv.c" "graphic.c" "vfont.c" "surface.c" "time.c" "timeutc.c" "media.c" "xalloc.c" "endianness.c" "gps.c" "vfs.c" "vfslocal.c" "filter.c" "telnet.c" "ctelnet.c" "login.c" "pterm.c" "bmp.c" "findargs.c" "rgb.c" "average.c" "custom_malloc.c" "threadudp.c" "mutex.c" "httpc.c" "httpd.c" "template.c"
    INCLUDE_DIRS "." "../../libpit" "../../font" 
    REQUIRES driver fatfs esp_timer libmath
    PRIV_REQUIRES freertos
)

if (${SO_PROJECT})
target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -DESP32 -DCRDATE=1770388845 -DESP32_ERRNO -fPIC -Wno-multichar -Wno-format)
else()
target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -DESP32 -DCRDATE=1770388845 -Wno-multichar -Wno-format)
endif()
