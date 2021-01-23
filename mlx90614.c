/*
 * mlx90614.c
 *
 *  Created on: 27 ago. 2019
 *      Author: Matias-Jesus
 */

#include "mlx90614.h"
//***************************************************************************************************************
void mlx90614_initPort()
{
    MLX90614_I2C_PORT_SEL |= (MLX90614_I2C_SDA_PIN + MLX90614_I2C_SCL_PIN);

    // Configure USCI_B0 for I2 mode - Sending
    UCB0CTLW0 |= UCSWRST;       // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK

    UCB0BRW = 0xA;              // Baudrate = SMCLK / 10; Bit clock prescaler. Modify only when UCSWRST = 1.

    UCB0I2CSA = MLX90614_I2C_ADDRESS;   // I2C slave Address.  The I2CSAx bits contain the slave address of the
                                             // external device to be addressed by the eUSCIx_B module. It is only used
                                             // in master mode. The address is right justified. In 7-bit slave addressing
                                             // mode, bit 6 is the MSB and bits 9-7 are ignored. In 10-bit slave addressing mode, bit 9 is the MSB
    UCB0CTLW0 &=~ UCSWRST;      // clear reset register

    UCB0IE &= ~UCRXIE0;          // Ensure Interrupts off.  Interrupt disabled.
    UCB0IE &= ~UCTXIE0;          // Ensure Interrupts off.  Interrupt disabled.

}
//***************************************************************************************************************
float mlx90614_getTemp(uint8_t MLX90614_ADDRESSTEMP)
{
    float aux = 0.0;
    uint8_t g_mlxValBytes[3];           // Recieved value byte storage

    // Send object temperature read command
    UCB0CTLW0 |= UCTR;                  // Change to transmitter.

    UCB0CTLW0 |= UCTXSTT;               // Send start. Transmit START condition in master mode. Ignored in slave mode.
                                        // In master receiver mode, a repeated START condition is preceded by a NACK.
                                        // UCTXSTT is automatically cleared after START condition and address information
                                        // is transmitted. Ignored in slave mode.
    while(UCB0CTLW0 & UCTXSTT);         // Wait for TX interrupt flag

    UCB0TXBUF = MLX90614_ADDRESSTEMP;   // Send temperature command
    while(!(UCB0IFG & UCTXIFG0));       // Wait for TX interrupt flag. eUSCI_B transmit interrupt flag 0. UCTXIFG0 is
                                        // set when UCBxTXBUF is empty in master mode or in slave mode, if the slave
                                        // address defined in UCBxI2COA0 was on the bus in the same frame.

    UCB0CTLW0 &= ~UCTR;             // Change to receive

    UCB0CTLW0 |= UCTXSTT;           // Send restart
    while(UCB0CTLW0 & UCTXSTT);     // Wait for restart

    // Receive Bytes
    while(!(UCB0IFG & UCRXIFG0));   // Wait for RX interrupt flag. eUSCI_B receive interrupt flag 0. UCRXIFG0 is
                                    // set when UCBxRXBUF has received a complete character in master mode or in
                                    // slave mode, if the slave address defined in UCBxI2COA0 was on the bus in the
                                    // same frame.
    g_mlxValBytes[0] = UCB0RXBUF;   // 0th byte.
                                    // The receive-data buffer is user accessible and contains the last
                                    // received character from the receive shift register. Reading UCBxRXBUF resets
                                    // the UCRXIFGx flags.
    while(!(UCB0IFG & UCRXIFG0));   // Wait for RX interrupt flag
    g_mlxValBytes[1] = UCB0RXBUF;   // 1st byte.

    UCB0CTLW0 |= UCTXSTP;
    while(UCB0CTLW0 & UCTXSTP);     // Wait for stop
                                    // Transmit STOP condition in master mode. Ignored in slave mode. In master
                                    // receiver mode, the STOP condition is preceded by a NACK. UCTXSTP is
                                    // automatically cleared after STOP is generated. This bit is a don't care, if
                                    // automatic UCASTPx is different from 01 or 10.

    while(!(UCB0IFG & UCRXIFG0));   // Wait for RX interrupt flag
    g_mlxValBytes[2] = UCB0RXBUF;   // 2nd byte.

    //calculate Temperature
    uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
    aux = ((float) tempVals) * 0.02 - 273.15;

    return (aux);
}
//***************************************************************************************************************
void mlx90614_sleepMode()
{
    UCB0CTLW0 |= UCTR;

    UCB0CTLW0 |= UCTXSTT;
    while(UCB0CTLW0 & UCTXSTT);

    // Send object temperature sleep mode command
    UCB0TXBUF = MLX90614_SLEEP;
    while(!(UCB0IFG & UCTXIFG0));

    MLX90614_I2C_SCL_LO;
}
//***************************************************************************************************************
void mlx90614_delay_ms(uint16_t ms)
{
    TA0CTL = TACLR;
    TA0CCR0 = ms;
    TA0CCTL0 |= CCIE;
    TA0EX0 = TAIDEX_3;
    TA0CTL = TASSEL_1 + ID_3 + MC_1;
    __bis_SR_register(LPM3_bits + GIE);
}
//***************************************************************************************************************
void mlx90614_exitSleepMode()
{
    //SCL pin high and then PWM/SDA pin low for at least tDDQ > 33ms
    MLX90614_I2C_SCL_HI;
    MLX90614_I2C_SDA_LO;

    mlx90614_delay_ms(40);

    UCB0CTLW0 |= UCTXSTP;
    while(UCB0CTLW0 & UCTXSTP);
}
//***************************************************************************************************************
void mlx90614_showTemp(float g_Temp)
{
    //Show object temperature
    volatile uint16_t aux;
    showChar('T',pos1);
    aux=((int)g_Temp)/10;
    showChar(aux+48,pos2);
    aux=((int)g_Temp)%10;
    showChar(aux+48,pos3);
    // Decimal point
    LCDMEM[pos3+1] |= 0x01;
    volatile float mantisa = g_Temp - (uint16_t)g_Temp;
    volatile uint16_t dosDecimales = mantisa * 100;
    aux=((int)dosDecimales)/10;
    showChar(aux+48,pos4);
    aux=((int)dosDecimales)%10;
    showChar(aux+48,pos5);
    // Degree symbol
    LCDMEM[pos5+1] |= 0x04;
    showChar('C',pos6);
}
//***************************************************************************************************************
// Timer A0 interrupt service routine --> Timer0_A3 CC0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
    TA0CTL = MC_0;
    TA0CCR0 = 0;
    TA0CTL &= ~TAIFG;
    TA0EX0 = TAIDEX_0;
    __bic_SR_register_on_exit(LPM3_bits + GIE);
}
