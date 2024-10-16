#include "mbed.h"
#include "PinNames.h"
#include "gpio_irq_api.h"

InterruptIn button(BUTTON1);
DigitalOut led(LED1);

void turn_light()
{
    led = !led;
}

int main() {
    button.rise(&turn_light);
     while (1) {
        printf("mypin has value : %d \n\r", button.read());
        ThisThread::sleep_for(250);
    }
}
