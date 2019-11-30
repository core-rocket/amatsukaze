#include "mbed.h"
 
DigitalOut myled(LED1);
PwmOut     servo_DOWN(p21);
PwmOut     servo_UP(p25);
DigitalIn  signal(p30);
 
#define ROCK_DOWN 0.0015
#define OPEN_DOWN 0.0024
 
#define ROCK_UP 0.0015
#define OPEN_UP 0.0006
 
int main(){
    while(1){
        servo_DOWN.pulsewidth(ROCK_DOWN);
        servo_UP.pulsewidth(ROCK_UP);
        while(signal == 1){
            servo_DOWN.pulsewidth(OPEN_DOWN);
            servo_UP.pulsewidth(OPEN_UP);
            myled = 1;
        }
        myled = 0;
    }
    return 0;
}
