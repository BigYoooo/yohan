#include "mbed.h"
#include "PinNames.h"
#include "gpio_irq_api.h"

using namespace std::chrono;

InterruptIn button(BUTTON1);
DigitalOut led(LED1);
Timer t;
Ticker flipper;

BufferedSerial uart(PA_9, PA_10, 9600);

void lookingForData(char *mess, char *mess_data)
{
    for (int i = 0; i < 95; i++)
    {
        if (mess[i] == 0x42)
        {
            if (mess[i + 1] == 0x4D)
            {
                for (int y = 0; i < 32; i++)
                {
                    mess_data[y] = mess[i + y];
                }
            }
        }
    }
}

void checksum(char* mess){
    int head = 0x42;
    int len = 0x4D;
    int result= head + len;
    for (int i=2; i<30; i++ )
    {
        result+=(int)mess[i];
    }
    int cs = (65536-(result))%256;
    printf("\nChecksum : %d\n", cs);
}

int main()
{

    char mess_recu[128];
    char data[32];
    char buffer_write[8] = {0x68, 0x01, 0x01, 0x96};
    char buffer_write2[8] = {0x68, 0x01, 0x04, 0x93};
    int pm1;
    int pm2;
    int pm4;
    int pm10;
    uart.write(buffer_write, sizeof(buffer_write));
    ThisThread::sleep_for(500ms);
    while (1)
    {
        uart.write(buffer_write2, sizeof(buffer_write2));
        ThisThread::sleep_for(100ms);
        uart.read(mess_recu, sizeof(mess_recu));
        ThisThread::sleep_for(500ms);
        if (uart.readable())
        {
            lookingForData(mess_recu, data);
            checksum(data);
            printf("Vrai checksum : %d", data[30]);

            pm1 = data[4] * 256 + data[5];
            pm2 = data[6] * 256 + data[7];
            pm4 = data[8] * 256 + data[9];
            pm10 = data[10] * 256 + data[11];

            printf("PM1.0 : %d ug/m3\n", pm1);
            printf("PM2.0 : %d ug/m3\n", pm2);
            printf("PM4.0 : %d ug/m3\n", pm4);
            printf("PM10  : %d ug/m3\n", pm10);

            printf("\nLe buffer mess_recu : ");
            for (int i = 0; i < 10; i++)
            {
                printf("%02x ", data[i]); 
            }
            printf("\n\n");
        }

        ThisThread::sleep_for(200ms);
        printf("\e[1;1H\e[2J");
    }
}
