/*
 * mlx90614.h
 *
 *  Created on: 27 ago. 2019
 *      Author: Matías-Jesús
 */

#ifndef MLX90614_H_
#define MLX90614_H_

/********************************************************************************************/
/*    Includes                                                                              */
/********************************************************************************************/
#include "driverlib.h"
#include "hal_LCD.h"

/********************************************************************************************/
/*    Defines                                                                             */
/********************************************************************************************/
#define MLX90614_I2C_ADDRESS 0x5A
// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM - Section 8.4.5 of data sheet explains why 0x20 is added to these addresses
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F
// Command to into Sleep Mode
#define MLX90614_SLEEP 0xFF     //Enter SLEEP mode

/********************************************************************************************/
/*     I2C                                                                                  */
/********************************************************************************************/
#define MLX90614_I2C_PORT_SEL      P5SEL0
#define MLX90614_I2C_PORT_OUT      P5OUT
#define MLX90614_I2C_PORT_REN      P5REN
#define MLX90614_I2C_PORT_DIR      P5DIR
#define MLX90614_I2C_SDA_PIN       BIT2         // UCB0SDA pin
#define MLX90614_I2C_SCL_PIN       BIT3         // UCB0SCL pin
#define MLX90614_I2C_SCL_CLOCK_DIV 0x0A         // SCL clock divider
// To exit from Sleep Mode (Wake up request)
#define MLX90614_I2C_SDA_LO { MLX90614_I2C_PORT_DIR |= MLX90614_I2C_SDA_PIN; MLX90614_I2C_PORT_OUT &= ~MLX90614_I2C_SDA_PIN; }
#define MLX90614_I2C_SDA_HI { MLX90614_I2C_PORT_DIR |= MLX90614_I2C_SDA_PIN; MLX90614_I2C_PORT_OUT |=  MLX90614_I2C_SDA_PIN; }
#define MLX90614_I2C_SCL_HI { MLX90614_I2C_PORT_DIR |= MLX90614_I2C_SCL_PIN; MLX90614_I2C_PORT_OUT |=  MLX90614_I2C_SCL_PIN; }
#define MLX90614_I2C_SCL_LO { MLX90614_I2C_PORT_DIR |= MLX90614_I2C_SCL_PIN; MLX90614_I2C_PORT_OUT &= ~MLX90614_I2C_SCL_PIN; }

/********************************************************************************************/
/*    Power Supply                                                                          */
/********************************************************************************************/
#define MLX90614_VCC_PORT_DIR P1DIR
#define MLX90614_VCC_PORT_OUT P1OUT
#define MLX90614_VCC_PIN BIT4
#define MLX90614_VCC_LO { MLX90614_VCC_PORT_DIR |= MLX90614_VCC_PIN; MLX90614_VCC_PORT_OUT &= ~MLX90614_VCC_PIN; }
#define MLX90614_VCC_HI { MLX90614_VCC_PORT_DIR |= MLX90614_VCC_PIN; MLX90614_VCC_PORT_OUT |=  MLX90614_VCC_PIN; }

/********************************************************************************************/
/*     Function Prototypes                                                                  */
/********************************************************************************************/
void  MLX90614_initPort         (void);
static void MLX90614_initWrite  (void);
static void MLX90614_initRead   (void);
float MLX90614_getTemp          (uint8_t);
void  MLX90614_sleepMode        (void);
void  MLX90614_delay_ms         (uint16_t);
void  MXL90614_exitSleepMode    (void);
void  MLX90614_showTemp         (float);

#endif /* MLX90614_H_ */
