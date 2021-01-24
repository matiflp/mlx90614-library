/*
 * main.c
 *
 *  Created on: 27 ago. 2019
 *      Author: Matías López - Jesús López
 */

#include "mlx90614.h"

int main(void)
{
    // Local Variable
    volatile float  aux = 0.0;

    // Stop WATCHDOG
    WDTCTL = WDTPW | WDTHOLD;

    // Configure LCD
    Init_LCD();

    // Energized the sensor
    MLX90614_VCC_HI;
    MLX90614_delay_ms(35);   //Requiere un tiempo para que el sensor este disponible una vez energizado.

    // Configure I2C
    MLX90614_initPort();

    // Desabilita el modo de alta impedancia habilitando la configuración establecida previamente.
    PM5CTL0 &= ~LOCKLPM5;

    while(1){

         // Object Temperature
         aux = MLX90614_getTemp(MLX90614_TOBJ1);
         MLX90614_showTemp(aux);
         //sleepMode();
         //exitSleepMode();

         // Object Temperature
         //aux = getTemp(MLX90614_TA);
         //showTemp(aux);
     }
}
