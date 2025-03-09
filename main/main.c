#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED 2

void blink(void *arg);
void tracktime(void* arg);
void wait_5min(void* arg);
void start_work_session();
void start_break_session();

TaskHandle_t blink_handle = NULL;
TaskHandle_t tracktime_handle = NULL;
TaskHandle_t wait_5min_handle = NULL;

void app_main(void) {
    start_work_session();  
}

void blink(void *arg) {
    printf("Blink task started\n");
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    while (1) {
        gpio_set_level(LED, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void tracktime(void* arg) {
    int time_passed = 0;
    printf("Embeddicat Time tracking started\n");

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        time_passed++;

        if (time_passed % 60 == 0) {
            printf("Embeddicat Pomodoro Timer: %d minutes\n", time_passed / 60);
        }

        if (time_passed / 60 >= 25) {
            printf("It's been 25 minutes! Break time!\n");
            start_break_session();  
            vTaskDelete(NULL); 
        }
        printf("DEBUG: %d\n", time_passed);
    }
}

void wait_5min(void* arg) {
    if (blink_handle != NULL) {
        vTaskDelete(blink_handle);
        blink_handle = NULL;
    }

    int time_passed = 0;
    printf("Break started! Relax for 5 minutes.\n");

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        time_passed++;

        if (time_passed % 60 == 0) {
            printf("Break Timer: %d minutes\n", time_passed / 60);
        }

        if (time_passed / 60 >= 5) {
            printf("Break's Over! Back to work!\n");
            start_work_session();  
            vTaskDelete(NULL); 
        }
    }
}

void start_work_session() {
    if (blink_handle == NULL) {
        xTaskCreatePinnedToCore(blink, "Blink GPIO2", 2048, NULL, 10, &blink_handle, 0);
    }

    if (tracktime_handle != NULL) {
        vTaskDelete(tracktime_handle);
        tracktime_handle = NULL;
    }
    if (wait_5min_handle != NULL) {
        vTaskDelete(wait_5min_handle);
        wait_5min_handle = NULL;
    }
    
    xTaskCreatePinnedToCore(tracktime, "Track Time", 4096, NULL, 10, &tracktime_handle, 1);
}

void start_break_session() {
    if (tracktime_handle != NULL) {
        vTaskDelete(tracktime_handle);
        tracktime_handle = NULL;
    }

    xTaskCreatePinnedToCore(wait_5min, "Wait 5 Min", 4096, NULL, 10, &wait_5min_handle, 1);
}
