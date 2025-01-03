/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>

#include "mbed.h"

#include "hpma115.h"

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers

#include "lora_radio_helper.h"

using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER 10000

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS 10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER 3

namespace
{
#define WAIT 1s
}
using namespace sixtron;
HPMA115 sensor(PA_9, PA_10);
static DigitalOut led1(LED1);
static hpma115_data_t data;
uint8_t coef;
HPMA115::ErrorType err;

void dust()
{
    err = sensor.read_measurement(&data);
    if (err == HPMA115::ErrorType::Ok)
    {
        printf("Data: ");
        if (data.pm1_pm4_valid)
        {
            printf("PM1.0: %d, PM4.0: %d ", data.pm1_0, data.pm4_0);
        }
        printf("PM10: %d, PM2.5: %d\n", data.pm10, data.pm2_5);
    }

    ThisThread::sleep_for(WAIT);
}

/**
 * Dummy sensor class object
 */
char payload[128];
// char *payload = ["{\"pm1_0\": , \"temperature1\": 32.5}"];

static void get_sensor_measure()
{
    dust();
    sprintf(payload, "{\"pm1_0\": %u, \"pm2_5\": %u, \"pm4_0\": %u, \"pm10\": %u}", data.pm1_0, data.pm2_5, data.pm4_0, data.pm10);
    printf(payload);
}

/**
 * This event queue is the global event queue for both the
 * application and stack. To conserve memory, the stack is designed to run
 * in the same thread as the application and the application is responsible for
 * providing an event queue to the stack that will be used for ISR deferment as
 * well as application information event queuing.
 */
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

/**
 * Entry point for application
 */

/*
 * Copyright (c) 2021, Koncepto.io
 * SPDX-License-Identifier: Apache-2.0
 */

int main(void)
{

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

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK)
    {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER) != LORAWAN_STATUS_OK)
    {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK)
    {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS)
    {
    }
    else
    {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    uint16_t packet_len;
    int16_t retcode;
    int32_t sensor_value;

    // TODO: Read sensor data
    memcpy(tx_buffer, payload, strlen(payload));
    packet_len = strlen(payload);

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);

    if (retcode < 0)
    {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                                              : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK)
        {
            // retry in 3 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON)
            {
                ev_queue.call_in(3000, send_message);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0)
    {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++)
    {
        printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");

    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    get_sensor_measure();
    switch (event)
    {
    case CONNECTED:
        printf("\r\n Connection - Successful \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        else
        {
            ev_queue.call_every(TX_TIMER, send_message);
        }

        break;
    case DISCONNECTED:
        ev_queue.break_dispatch();
        printf("\r\n Disconnected Successfully \r\n");
        break;
    case TX_DONE:
        printf("\r\n Message Sent to Network Server \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    case TX_TIMEOUT:
    case TX_ERROR:
    case TX_CRYPTO_ERROR:
    case TX_SCHEDULING_ERROR:
        printf("\r\n Transmission Error - EventCode = %d \r\n", event);
        // try again
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    case RX_DONE:
        printf("\r\n Received message from Network Server \r\n");
        receive_message();
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        printf("\r\n Error in reception - Code = %d \r\n", event);
        break;
    case JOIN_FAILURE:
        printf("\r\n OTAA Failed - Check Keys \r\n");
        break;
    case UPLINK_REQUIRED:
        printf("\r\n Uplink required by NS \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    default:
        MBED_ASSERT("Unknown Event");
    }
}

// EOF