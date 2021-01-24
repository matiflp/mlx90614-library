/*
 * mlx90614.c
 *
 *  Created on: 27 ago. 2019
 *      Author: Matias-Jesus
 */

#include "mlx90614.h"
//***************************************************************************************************************
void MLX90614_initPort(void)
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
//**********************************************************************************************************************************************************
static void MLX90614_initWrite(void)
{
  UCB0CTLW0 |= UCTR;                        // UCTR=1 => Transmit Mode (R/W bit = 0)
  UCB0IFG &= ~(UCTXIFG0 | UCSTPIFG);
  UCB0IE &= ~UCRXIE0;                       // disable Receive ready interrupt
  UCB0IE |= (UCTXIE0 | UCSTPIE);            // enable Transmit ready interrupt
}
//**********************************************************************************************************************************************************
static void MLX90614_initRead(void)
{
  UCB0CTLW0 &= ~UCTR;                       // UCTR=0 => Receive Mode (R/W bit = 1)
  UCB0IFG &= ~(UCRXIFG0 | UCSTPIFG);
  UCB0IE &= ~(UCTXIE0 | UCSTPIE);           // disable Transmit ready interrupt
  UCB0IE |= UCRXIE0;                        // enable Receive ready interrupt
}
//**********************************************************************************************************************************************************
float MLX90614_getTemp(uint8_t command)
{
    float aux = 0.0;
    uint8_t temp[3];                            // Recieved value byte storage

    // Send object temperature read command
    MLX90614_initWrite();                       // Change to transmitter.

    UCB0CTLW0 |= UCTXSTT;
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = command;                        // Send temperature command
    __bis_SR_register(LPM3_bits + GIE);

    // Receive Bytes
    MLX90614_initRead();                        // Change to receive

    UCB0CTLW0 |= UCTXSTT;                       // Send restart
    __bis_SR_register(LPM3_bits + GIE);         // Wait for restart

    temp[0] = UCB0RXBUF;
    __bis_SR_register(LPM3_bits + GIE);         // Wait for RX interrupt flag

    UCB0CTLW0 |= UCTXSTP;

    temp[1] = UCB0RXBUF;                        // 1st byte.
    __bis_SR_register(LPM3_bits + GIE);

    temp[2] = UCB0RXBUF;                        // 2nd byte.

    UCB0IE |= UCSTPIE;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE &= ~(UCRXIE0 | UCSTPIE);

    //calculate Temperature
    uint16_t tempVals = ( ((uint16_t) temp[1]) << 8 ) | ( (uint16_t) temp[0] );
    aux = ((float) tempVals) * 0.02 - 273.15;

    return (aux);
}
//***************************************************************************************************************
void MLX90614_delay_ms(const uint16_t ms)
{
    TA1CTL = TACLR;
    TA1CCR0 = ms;
    TA1CCTL0 |= CCIE;
    TA1EX0 = TAIDEX_3;
    TA1CTL = TASSEL_1 + ID_3 + MC_1;
    __bis_SR_register(LPM3_bits + GIE);
}
//***************************************************************************************************************
void MLX90614_sleepMode(void)
{
    UCB0CTLW0 |= UCTR;

    UCB0CTLW0 |= UCTXSTT;
    __bis_SR_register(LPM3_bits + GIE);

    // Send object temperature sleep mode command
    UCB0TXBUF = MLX90614_SLEEP;
    __bis_SR_register(LPM3_bits + GIE);

    MLX90614_I2C_SCL_LO
}
//***************************************************************************************************************
void MLX90614_exitSleepMode(void)
{
    //SCL pin high and then PWM/SDA pin low for at least tDDQ > 33ms
    MLX90614_I2C_SCL_HI
    MLX90614_I2C_SDA_LO

    MLX90614_delay_ms(40);

    UCB0CTLW0 |= UCTXSTP;
    while(UCB0CTLW0 & UCTXSTP);
}
//***************************************************************************************************************
void MLX90614_showTemp(float g_Temp)
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
//********************************************************************************************************************************************************************
// I2C interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCIB0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:                 // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:                 // Vector 8: STPIFG

        __bic_SR_register_on_exit(LPM3_bits + GIE);
        break;

    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 14: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 16: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 18: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 20: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 22: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 24: RXIFG0
    case USCI_I2C_UCTXIFG0:                 // Vector 26: TXIFG0

        __bic_SR_register_on_exit(LPM3_bits + GIE);
        break;

    case USCI_I2C_UCBCNTIFG: break;         // Vector 28: BCNTIFG
    case USCI_I2C_UCCLTOIFG: break;         // Vector 30: clock low timeout
    case USCI_I2C_UCBIT9IFG: break;         // Vector 32: 9th bit
    default: break;
  }
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
