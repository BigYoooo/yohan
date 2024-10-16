#include "mbed.h"
#include "PinNames.h"
#include "gpio_irq_api.h"

using namespace std::chrono;

InterruptIn button(BUTTON1);
DigitalOut led(LED1);
Timer t;


void turn_on()
{
    led = 1;
    t.start();
}

void turn_off()
{
    led = 0;
    t.stop();
    t.reset();
}

int main() {
    button.rise(&turn_on);
    button.fall(&turn_off);
     while (1) {
        printf("mypin has value : %d \n\r", button.read());
        if (duration_cast<milliseconds>(t.elapsed_time()).count() > 0)
        {
            printf("The time taken was %llu milliseconds\n", duration_cast<milliseconds>(t.elapsed_time()).count());
        }
        ThisThread::sleep_for(250);
    }
}
