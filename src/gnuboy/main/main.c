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

#include <string.h>

#define GB_FILE NULL
//#define GB_FILE "/sd/roms/gb/zelda.gb"

extern int main(int argc, const char *argv[]);

void app_task(void *arg)
{
    int argc = GB_FILE == NULL ? 1 : 2;
    const char *argv[] = { "gnuboy", GB_FILE, NULL };

    main(argc, argv);

    while (1)
    {
        vTaskDelay(1);
    }
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGI(__func__, "gnuboy (%s).\n", __DATE__);

#if CONFIG_STORAGE_LITTLEFS
    bsp_littlefs_mount();
#elif CONFIG_STORAGE_SPIFFS
    bsp_spiffs_mount();
#else
    bsp_sdcard_mount();
#endif

    xTaskCreatePinnedToCore(&app_task, "app_task", 20000, NULL, /*5*/2 | portPRIVILEGE_BIT, NULL, 0);
}
