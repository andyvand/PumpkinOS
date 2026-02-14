#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event.h"
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
#include "hal/cache_hal.h"
#include "soc/sensitive_reg.h"

#include <string.h>

#if CONFIG_ENABLE_WIFI
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PWD CONFIG_WIFI_PASSWORD

#if CONFIG_WIFI_ALL_CHANNEL_SCAN
#define DEFAULT_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#elif CONFIG_WIFI_FAST_SCAN
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#else
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#endif /*CONFIG_SCAN_METHOD*/

#if CONFIG_WIFI_CONNECT_AP_BY_SIGNAL
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_WIFI_CONNECT_AP_BY_SECURITY
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#else
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#endif /*CONFIG_SORT_METHOD*/

#if CONFIG_FAST_SCAN_THRESHOLD
#define DEFAULT_RSSI CONFIG_FAST_SCAN_MINIMUM_SIGNAL
#if CONFIG_OPENTYRIAN_OPEN
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#elif CONFIG_OPENTYRIAN_WEP
#define DEFAULT_AUTHMODE WIFI_AUTH_WEP
#elif CONFIG_OPENTYRIAN_WPA
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA_PSK
#elif CONFIG_OPENTYRIAN_WPA2
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA2_PSK
#else
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif
#else
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif
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

#if CONFIG_ENABLE_WIFI
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(__func__, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(__func__, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ip_event_got_ip_t* event_ip = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(__func__, "Got IP: " IPSTR, IP2STR(&event_ip->ip_info.ip));
    }
}

static void wifi_scan(void)
{
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
            .scan_method = DEFAULT_SCAN_METHOD,
            .sort_method = DEFAULT_SORT_METHOD,
            .threshold.rssi = DEFAULT_RSSI,
            .threshold.authmode = DEFAULT_AUTHMODE,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_LOGI(__func__, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(__func__, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}
#endif

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

    // finish
    vfs_finish();
    thread_close();
    debug_close();
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGI(__func__, "PumpkinOS (%s).\n", __DATE__);

    ESP_ERROR_CHECK(nvs_flash_init());

#if CONFIG_ENABLE_WIFI
    wifi_scan();
#endif

#if CONFIG_STORAGE_LITTLEFS
    bsp_littlefs_mount();
#elif CONFIG_STORAGE_SPIFFS
    bsp_spiffs_mount();
#else
    bsp_sdcard_mount();
#endif

    xTaskCreatePinnedToCore(&app_task, "app_task", 8000, NULL, /*5*/2 | portPRIVILEGE_BIT, NULL, 0);
}
