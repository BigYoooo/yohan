
/*
 * Copyright (c) 2021, Koncepto.io
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "hpma115.h"

namespace {
#define WAIT 1s
}
using namespace sixtron;
static DigitalOut led1(LED1);
static hpma115_data_t data;

int main()
{
    uint8_t coef;
    HPMA115::ErrorType err;

    HPMA115 sensor(PA_9, PA_10);

    printf("\n\n------------\nHPMA115 Example\n");

    err = sensor.stop_measurement();
    assert(err == HPMA115::ErrorType::Ok);

    err = sensor.stop_autosend();
    assert(err == HPMA115::ErrorType::Ok);

    err = sensor.set_adjust_coef(200);
    assert(err == HPMA115::ErrorType::Ok);

    err = sensor.read_adjust_coef(&coef);
    assert(err == HPMA115::ErrorType::Ok);
    assert(coef == 200);

    err = sensor.set_adjust_coef(100);
    assert(err == HPMA115::ErrorType::Ok);

    err = sensor.start_measurement();
    assert(err == HPMA115::ErrorType::Ok);

    while (true) {
        led1 = !led1;
        err = sensor.read_measurement(&data);
        if (err == HPMA115::ErrorType::Ok) {
            printf("Data: ");
            if (data.pm1_pm4_valid) {
                printf("PM1.0: %d, PM4.0: %d ", data.pm1_0, data.pm4_0);
            }
            printf("PM10: %d, PM2.5: %d\n", data.pm10, data.pm2_5);
        }

        ThisThread::sleep_for(WAIT);
    }
}
