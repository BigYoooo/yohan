#include "mbed.h"
#include "PinNames.h"
#include "gpio_irq_api.h"

using namespace std::chrono;

InterruptIn button(BUTTON1);
DigitalOut led(LED1);
Timer t;
Ticker flipper;
int var = 1;

//void turn_on()
//{
//    led = 1;
//    t.start();
//}
//
//void turn_off()
//{
//    led = 0;
//    t.stop();
//    t.reset();
//}

void flip(){
    led = !led;
}

void decrement(){
    var = var - 1;
    flipper.detach();
    flipper.attach(&flip, var);
}

void increment(){
    var = var + 1;
    flipper.detach();
    flipper.attach(&flip, var);
}

int main() {
    button.fall(&decrement);
    button.rise(&increment);
    flipper.attach(&flip, var); 

    
    
    //    if (duration_cast<milliseconds>(t.elapsed_time()).count() > 0)
    //    {
    //        printf("The time taken was %llu milliseconds\n", duration_cast<milliseconds>(t.elapsed_time()).count());
    //    }

     while (1) {
        printf("mypin has value : %d \n\r", button.read());
        ThisThread::sleep_for(250);
    }
}
