#include <msp430.h> 
/**
 * main.c
 */

void delay() {
    int i;
    for(i = 10000; i > 0; i--) {}
}

void initLCD() {
    delay();
    P2OUT &= BIT0;
    P2OUT &= BIT6;
    P1OUT = 0x30;

    delay();

    P2OUT &= BIT0;
    P2OUT &= BIT6;
    P1OUT = 0x30;

    delay();

    P2OUT &= BIT0;
    P2OUT &= BIT6;

    P1OUT = 0x30;
    P1OUT = 0x20;

    P1OUT = 0x20;
    P1OUT = 0xC0;

    P1OUT = 0x00;
    P1OUT = 0x80;

    P1OUT = 0x00;
    P1OUT = 0x10;

    P1OUT = 0x00;
    P1OUT = 0x40;

    P1OUT = 0x00;
    P1OUT = 0xF0;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    //-- 1. Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 2. Configure eUSCI_B0
    UCB0CTLW0 |= UCMODE_3;  //Put into I2C mode
    UCB0I2COA0 = 0x0043;     //Slave address = 0x42
    UCB0CTLW0 &= ~UCMST;     //Put into slave mode
    UCB0CTLW0 &= ~UCTR;      //Put into Rx mode
    UCB0I2COA0 |= UCGCEN;
    UCB0I2COA0 |= UCOAEN;
    //-- 3. Configure Pins
    P1SEL1 &= ~BIT3;        //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;        //P1.2 = SDA
    P1SEL0 |= BIT2;
    //-- 3.1
    P2DIR |= BIT0|BIT6;//|BIT7;
    P1DIR |= BIT7|BIT6|BIT5|BIT4;//|BIT1|BIT0;
    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCRXIE0;
    __enable_interrupt();


    initLCD();

    while(1) {

    }
	return 0;
}
