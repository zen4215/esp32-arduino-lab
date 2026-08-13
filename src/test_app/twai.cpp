#include "test_app.hpp"

#include <Arduino.h>
#include <nnct_lib.hpp>
#include "driver/twai.h"

nnct::components::Watchdog wd(1000);

void test_twai_setup() {
    Serial.begin(9600);

    twai_general_config_t general_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = (gpio_num_t)0,
        .rx_io = (gpio_num_t)0,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_NONE,
        .clkout_divider = 0,
        .intr_flags = 0
    };

    twai_timing_config_t timing_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if(twai_driver_install(&general_config, &timing_config, &filter_config) != ESP_OK) {
        Serial.println("twai driver install failed!");

        while(1) {

        }
    }
    
    if(twai_start() != ESP_OK) {
        Serial.println("twai start failed!");
        
        while(1) {

        }
    }
}

void test_twai_loop() {
    twai_message_t rx_msg;
    if(twai_receive(&rx_msg, portMAX_DELAY) == ESP_OK) {
        Serial.println("received!");
    }
    
    if(wd.update(millis())) {
        twai_message_t tx_msg = {
            .flags = TWAI_MSG_FLAG_NONE,
            .identifier = 0,
            .data_length_code = 0
        };
        
        twai_transmit(&tx_msg, pdMS_TO_TICKS(1000));
    }
}
