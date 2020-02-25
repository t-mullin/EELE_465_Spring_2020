#include <msp430.h> 
/**
 * main.c
 */

char data_in;

void delay(int i) {
    TB0CCR0 = i;
    TB0CTL |= TBCLR;
    while(TB0R != TB0CCR0) {}
}

void toggleEnable() {
    P2OUT |= BIT0;
    delay(2);
    P2OUT &= ~BIT0;
}

void writeToLCD() {
    toggleEnable();
    delay(3);
}

void init_LCD() {
    //2 is ~75us
    // 525 is ~16ms
    delay(525); // Start up delay
    P1OUT = 0x30;
    toggleEnable();

    delay(135);     // 135 is ~4.15ms
    P1OUT = 0x30;
    toggleEnable();

    delay(4);       // 4 is ~150us
    P1OUT = 0x30;
    writeToLCD();

    P1OUT = 0x20;
    writeToLCD();

    P1OUT = 0x20;
    writeToLCD();
    P1OUT = 0x80;
    writeToLCD();

    P1OUT = 0x00;
    writeToLCD();
    P1OUT = 0xC0;
    writeToLCD();

    P1OUT = 0x00;
    writeToLCD();
    delay(55);
    P1OUT = 0x10;
    writeToLCD();
    delay(55);

    P1OUT = 0x00;
    writeToLCD();
    P1OUT = 0x60;
    writeToLCD();

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

    //-- 2.1 - Configure Timers
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;

    //-- 3. Configure Pins
    P1SEL1 &= ~BIT3;        //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;        //P1.2 = SDA
    P1SEL0 |= BIT2;
    //-- 3.1
    P1DIR |= BIT7|BIT6|BIT5|BIT4;
    P2DIR |= BIT7|BIT6|BIT0;

    P1OUT &= ~BIT7|~BIT6|~BIT5|~BIT4;
    //rs 2.6, rw 2.7, en 2.0
    P2OUT &= ~BIT7|~BIT6|~BIT0; //Clear RS, R/W, and enable;

    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCRXIE0;
    __enable_interrupt();

    init_LCD();

    while(1) {

    }
	return 0;
}

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch(UCB0IV) {
        case 0x16:
            data_in = UCB0RXBUF;
            break;
        default:
            break;
    }
}
