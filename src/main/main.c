#include "freertos/FreeRTOS.h"
//#include "esp_wifi.h"
#include "esp_system.h"
//#include "esp_event.h"
//#include "esp_event_loop.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/i2s_std.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_task_wdt.h"
#include "esp_spiffs.h"
#include "driver/rtc_io.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "vfs.h"
#include "vfslocal.h"
#include "ptr.h"
#include "sys.h"
#include "thread.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "script.h"
#include "debug.h"

#include <string.h>

#ifndef CONFIG_STORAGE_LITTLEFS
#define CONFIG_STORAGE_LITTLEFS 1
#endif

#if defined(LIBLSDL2)
extern int liblsdl2_load(void);
extern int liblsdl2_init(int pe, script_ref_t obj);
#else
extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
#endif

extern int libos_app_init(int pe);
extern int libos_start(int pe);
extern void malloc_init(void);

void app_task(void *arg)
{
    // Init
    debug_setsyslevel(NULL, DEBUG_INFO);
    debug_init(NULL);
    ptr_init();
    thread_init();
    vfs_init();

#if defined(LIBLSDL2)
    // SDL2
    liblsdl2_load();
    liblsdl2_init(0, 0);
#else
    // SDL3
    liblsdl3_load();
    liblsdl3_init(0, 0);
#endif

    // VFS Mount
#if CONFIG_STORAGE_LITTLEFS
    vfs_local_mount(CONFIG_BSP_LITTLEFS_MOUNT_POINT "/vfs/", "/");
#elif CONFIG_STORAGE_SPIFFS
    vfs_local_mount(CONFIG_BSP_SPIFFS_MOUNT_POINT "/vfs/", "/");
#else
    vfs_local_mount(BSP_SD_MOUNT_POINT "/vfs/", "/");
#endif

    // libos
    libos_app_init(0);
    libos_start(0);

    while (1)
    {
        vTaskDelay(1);
    }

    // finish
    vfs_finish();
    thread_close();
    debug_close();
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGI(__func__, "PumpkinOS (%s).\n", __DATE__);

#if CONFIG_STORAGE_LITTLEFS
    bsp_littlefs_mount();
#elif CONFIG_STORAGE_SPIFFS
    bsp_spiffs_mount();
#else
    bsp_sdcard_mount();
#endif

    xTaskCreatePinnedToCore(&app_task, "app_task", 20000, NULL, /*5*/2 | portPRIVILEGE_BIT, NULL, 0);
}
