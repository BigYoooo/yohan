#include "mbed.h"
#include "PinNames.h"

DigitalIn  mypin(BUTTON1);
DigitalOut myled(LED1);

int main() {

     while (1) {
        printf("mypin has value : %d \n\r", mypin.read());
        myled = mypin;
        ThisThread::sleep_for(250);
    }
}
