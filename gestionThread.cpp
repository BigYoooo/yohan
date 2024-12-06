/*
#include "mbed.h"
#include "PinNames.h"
#include "gpio_irq_api.h"

DigitalOut led(LED1);

Thread my_thread_1(osPriorityNormal, 1024, nullptr, nullptr);
Thread my_thread_2(osPriorityNormal, 1024, nullptr, nullptr);

Mutex mtx;

void ping()
{
    mtx.lock();
    for (int i = 0; i < 100; ++i)
    {
        printf("Ping\n");
        
    }
    mtx.unlock();
}

void pong()
{
    mtx.lock();
    for (int i = 0; i < 100; ++i)
    {
        printf("Pong\n");
    }
    mtx.unlock();
}

int main()
{
    my_thread_1.start(ping);
    my_thread_2.start(pong);

    while (1)
    {
        led = 1;
        ThisThread::sleep_for(500ms);
        led = 0;
        ThisThread::sleep_for(500ms);
    }

    return 0;
}
*/