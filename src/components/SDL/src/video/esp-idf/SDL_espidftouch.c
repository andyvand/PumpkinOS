#include "events/SDL_touch_c.h"
#include "events/SDL_mouse_c.h"
#include "video/SDL_sysvideo.h"
#include "SDL_espidftouch.h"
#include <stdbool.h>

#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#if BSP_CAPS_TOUCH == 1
#include "bsp/touch.h"
esp_lcd_touch_handle_t touch_handle;   // LCD touch handle
#endif
#include "esp_log.h"

//#define ESPIDF_TOUCH_ID       1
#define ESPIDF_TOUCH_FINGER     1


void ESPIDF_InitTouch(void)
{
#if CONFIG_BSP_TOUCH_ENABLED
    bsp_i2c_init();

    /* Initialize touch */
    bsp_touch_new(NULL, &touch_handle);

    SDL_AddTouch(SDL_MOUSE_TOUCHID, SDL_TOUCH_DEVICE_DIRECT, "mouse_input");
    ESP_LOGI("SDL", "ESPIDF_InitTouch");
#endif
}

#if CONFIG_BSP_TOUCH_ENABLED
static SDL_Window *espidf_get_window(void)
{
    SDL_VideoDevice *device = SDL_GetVideoDevice();
    if (!device) {
        return NULL;
    }
    return device->windows;
}
#endif

void ESPIDF_PumpTouchEvent(void)
{
#if CONFIG_BSP_TOUCH_ENABLED
    SDL_Window *window;
    static bool was_pressed = false;
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;
    bool pressed;

    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    if (!touch_handle) {
        return;
    }

    esp_lcd_touch_read_data(touch_handle);
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
    pressed = touchpad_pressed && touchpad_cnt > 0;

    window = espidf_get_window();
    if (!window) {
        return;
    }

    if (pressed != was_pressed) {
        was_pressed = pressed;
        ESP_LOGI("SDL", "touchpad_pressed: %d, [%d, %d]", pressed, touchpad_x[0], touchpad_y[0]);
        if (pressed) {
            last_x = touchpad_x[0];
            last_y = touchpad_y[0];
            SDL_PerformWarpMouseInWindow(window, (float)touchpad_x[0], (float)touchpad_y[0], false);
            SDL_SendMouseButton(0, window, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT, true);
        } else {
            // Release: report up at last known position; don't warp to (0,0)
            SDL_SendMouseButton(0, window, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT, false);
        }
    } else if (pressed) {
        // Held: emit motion only when the position actually changed
        if (touchpad_x[0] != last_x || touchpad_y[0] != last_y) {
            last_x = touchpad_x[0];
            last_y = touchpad_y[0];
            SDL_SendMouseMotion(0, window, SDL_TOUCH_MOUSEID, false,
                                (float)touchpad_x[0], (float)touchpad_y[0]);
        }
    }
#endif
}

int ESPIDF_CalibrateTouch(float screenX[], float screenY[], float touchX[], float touchY[])
{
    return 0;
}

void ESPIDF_ChangeTouchMode(int raw)
{
    return;
}

void ESPIDF_ReadTouchRawPosition(float* x, float* y)
{
    return;
}

void ESPIDF_QuitTouch(void)
{
    // ts_close(ts);
}
