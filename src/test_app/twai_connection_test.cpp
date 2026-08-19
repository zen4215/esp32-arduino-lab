#include <Arduino.h>
#include <driver/twai.h>

constexpr uint32_t SERIAL_BAUDRATE = 9600; 
const String PROJECT_NAME = "esp-can-analyzer";

const String DEBUG_OK_HEADER    = "[ OK ]   :";
const String DEBUG_ERR_HEADER   = "[ ERR ]  :";
const String DEBUG_INFO_HEADER  = "[ INFO ] :";

twai_general_config_t TWAI_GENERAL_CONFIG = {
    .mode           = TWAI_MODE_NO_ACK,
    .tx_io          = (gpio_num_t)32,
    .rx_io          = (gpio_num_t)33,
    .clkout_io      = TWAI_IO_UNUSED,
    .bus_off_io     = TWAI_IO_UNUSED,
    .tx_queue_len   = 5,
    .rx_queue_len   = 5,
    .alerts_enabled = TWAI_ALERT_NONE,
    .clkout_divider = 0,
    .intr_flags     = 0
};
constexpr twai_timing_config_t TWAI_TIMING_CONFIG = TWAI_TIMING_CONFIG_1MBITS();
constexpr twai_filter_config_t TWAI_FILTER_CONFIG = TWAI_FILTER_CONFIG_ACCEPT_ALL();

constexpr uint16_t TEST_TIMEOUT = 1000;
constexpr uint16_t TEST_RETRY_MAX = 5;

void twai_transmit_test(bool loopback);
void twai_receive_test(bool clear_flag = false);

void twai_connection_test_setup() {
    Serial.begin(SERIAL_BAUDRATE);

    Serial.println("\n\n" + PROJECT_NAME);

    if(twai_driver_install(&TWAI_GENERAL_CONFIG, &TWAI_TIMING_CONFIG, &TWAI_FILTER_CONFIG) == ESP_OK) Serial.println(DEBUG_OK_HEADER + "twai driver has just installed.");
    else Serial.println(DEBUG_ERR_HEADER + "twai driver install fail.");
    if(twai_start() == ESP_OK) Serial.println(DEBUG_OK_HEADER + "twai has just started.");
    else Serial.println(DEBUG_ERR_HEADER + "twai start fail.");

    Serial.println(DEBUG_INFO_HEADER + "twai test start. single shot mode is enable.");
    Serial.println(DEBUG_INFO_HEADER + "twai loopback(no ack mode and self reception request) test start.");

    Serial.println(DEBUG_INFO_HEADER + "twai transmit test start.");
    twai_transmit_test(true);
    Serial.println(DEBUG_INFO_HEADER + "twai receive test start.");
    twai_receive_test(true);

    Serial.println(DEBUG_INFO_HEADER + "twai normal(normal mode and self reception request is disable) test start.");

    Serial.println(DEBUG_INFO_HEADER + "twai transmit test start.");
    twai_transmit_test(false);
    Serial.println(DEBUG_INFO_HEADER + "twai receive test start.");
    twai_receive_test(true);

    Serial.println(DEBUG_INFO_HEADER + "setup() has just finished.");
}

void twai_transmit_test(bool loopback) {
    twai_message_t msg;

    msg.ss = 1;
    msg.self = loopback ? 1 : 0,
    msg.data_length_code = 0;

    const String ERR_MSG = DEBUG_ERR_HEADER + "twai transmit is fail ";

    static uint retry_count = 0;
    switch(twai_transmit(&msg, TEST_TIMEOUT)) {
        case ESP_ERR_INVALID_ARG:
            Serial.println(ERR_MSG + "by invalid arg.");
            break;

        case ESP_ERR_NOT_SUPPORTED:
            Serial.println(ERR_MSG + "because driver is listen only mode.");
            break;

        case ESP_ERR_TIMEOUT:
            Serial.println(ERR_MSG + "because its timeout. timeout is " + String(TEST_TIMEOUT) + ".");

            if(retry_count >= TEST_RETRY_MAX) {
                Serial.println(DEBUG_ERR_HEADER + "timeout retry over. retry max is " + String(TEST_RETRY_MAX) + ".");
            } else {
                Serial.println(DEBUG_INFO_HEADER + "retry twai transmit. retry count is " + String(retry_count) + ".");
                retry_count++;
                twai_transmit_test(loopback);
            }
            break;

        case ESP_FAIL:
            Serial.println(ERR_MSG + ".");
            break;

        case ESP_OK:
            Serial.println(DEBUG_OK_HEADER + "twai transmit has completed.");
            break;
    }
}

void twai_receive_test(bool clear_flag) {
    twai_message_t msg;

    const String ERR_MSG = DEBUG_ERR_HEADER + "twai receive is fail ";
    static size_t retry_count;

    if(clear_flag) {
        retry_count = 1;
    }

    switch(twai_receive(&msg, TEST_TIMEOUT)) {
        case ESP_ERR_INVALID_ARG:
            Serial.println(ERR_MSG + "by invalid arg.");
            break;

        case ESP_ERR_TIMEOUT:
            Serial.println(ERR_MSG + "because its timeout. timeout is " + String(TEST_TIMEOUT) + ".");

            if(retry_count > TEST_RETRY_MAX) {
                Serial.println(DEBUG_ERR_HEADER + "timeout retry over. retry max is " + String(TEST_RETRY_MAX) + ".");
            } else {
                Serial.println(DEBUG_INFO_HEADER + "retry twai receive. retry count is " + String(retry_count) + ".");
                retry_count++;
                twai_receive_test();
            }
            break;

        case ESP_OK:
            Serial.println(DEBUG_OK_HEADER + "twai receive has completed.");
            break;
    }
}

