// ============================================================================
//  Stage 1: "Hello, serial" — the ESP-IDF / C version.
//
//  Same goal as the C++/Arduino project's Stage 1: before touching the display
//  (where many things break at once), prove the whole pipeline — compile C →
//  flash → read output — with a trivial program.
//
//  What's different from Arduino? There is no setup()/loop(). ESP-IDF hands you
//  ONE entry point, app_main(), which runs on a FreeRTOS task. It runs once; if
//  you want to keep doing something you write the loop yourself. (Arduino was
//  writing that loop for you the whole time — see docs/lesson-01-first-light.md.)
// ============================================================================

#include "freertos/FreeRTOS.h"   // the real-time OS ESP-IDF is built on
#include "freertos/task.h"       // vTaskDelay, task APIs
#include "esp_log.h"             // ESP_LOGI(): logging with levels + a tag
#include "esp_timer.h"           // esp_timer_get_time(): microseconds since boot

// A "tag" groups log lines from this file. The monitor prints it in colour:
//   I (1234) tdisplay: ...   <- I = Info level, 1234 = ms, tdisplay = this tag
static const char *TAG = "tdisplay";

void app_main(void)
{
    ESP_LOGI(TAG, "=== T-Display S3 Pro is alive! (ESP-IDF / C) ===");

    int counter = 0;
    while (1) {                                    // WE write the forever-loop now
        // esp_timer_get_time() returns int64 microseconds; /1000 -> milliseconds.
        ESP_LOGI(TAG, "tick %d  (uptime %lld ms)",
                 ++counter, esp_timer_get_time() / 1000);

        // Don't busy-wait: hand the CPU back to FreeRTOS for ~1 second.
        // pdMS_TO_TICKS converts milliseconds into the OS's tick units.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
