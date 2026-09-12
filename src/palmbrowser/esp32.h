#ifndef ESP32_H
#define ESP32_H

/* build parameters for the ESP32 PumpkinOS firmware build, in the same
   form as the other PumpkinOS app components (see MineHunt/esp32.h) */

#define PUMPKIN_APP 1
#define SYS_CPU 3
#define SYS_SIZE 1
#define SYS_OS 1
#define SYS_ENDIAN 1
#define SYSTEM_NAME "pit"
#define SYSTEM_VERSION "1.0"
#define SYSTEM_OS "ESP32"
#define SOEXT ".so"
#define ESP32
#define APPNAME "Browser"
#define APPID 'Brws'

#ifndef BROWSER_PUMPKIN
#define BROWSER_PUMPKIN 1
#endif
#ifndef IS_LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN 1
#endif

#endif /* ESP32_H */
