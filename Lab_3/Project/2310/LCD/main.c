#include <msp430.h> 
/**
 * main.c
 */

char data_in = 'x';

void delay(unsigned int i) {
    //TB0CCR0 = i;
    //TB0CTL |= TBCLR;
    //while(TB0R != TB0CCR0) {}
    //P2OUT ^= BIT0;
    unsigned int j;
    for(j = 0; j < 10000; j=j+1) {}
    P2OUT ^= BIT0;
}

void toggleEnable() {
    P2OUT |= BIT0;
    P2OUT &= ~BIT0;
}

void writeToLCD(int nibble, int delayNum) {
    P1OUT = nibble;
    toggleEnable();
    delay(delayNum);
}


void init_LCD() {

    //2 is ~75us
    // 4 is ~150us
    // 135 is ~4.15ms
    // 525 is ~16ms

    delay(525); // Start up delay

    //function set 8-bit, delay > 4.1ms
    writeToLCD(0x30, 135);
    //function set 8-bit, delay > 150us
    writeToLCD(0x30, 4);
    //function set, delay 8-bit, > 40us
    writeToLCD(0x30, 4);

    //function set 4-bit, delay > 40us
    writeToLCD(0x30, 4);

    //function set 4-bit
    writeToLCD(0x20, 0);
    writeToLCD(0x80, 0); //NF**, N = # of lines, F = Char font

    //Display off
    writeToLCD(0x00, 0);
    writeToLCD(0x10, 0); //1DCB, D = toggle display on/off, C = cursor, B = blink

    //Entry Mode Set
    writeToLCD(0x00, 0);
    writeToLCD(0x60, 0); //01I/DS, I/D = increment/decrement, S = shifts the display when 1

    //Display on
    writeToLCD(0x00, 0);
    writeToLCD(0xE0, 0); //1DCB, D = toggle display on/off, C = cursor, B = blink
    /*
    //function set 4bit
    P1OUT = 0x20;
    toggleEnable();
    delay(8);     // 135 is ~4.15ms

    //function set
    P1OUT = 0x20;
    writeToLCD();
    P1OUT = 0x80;
    writeToLCD();

    //display on
    P1OUT = 0x00;
    writeToLCD();
    P1OUT = 0xE0;
    writeToLCD();

    //display clear
    //P1OUT = 0x00;
    //writeToLCD();
    //delay(110);
    //P1OUT = 0x10;
    //writeToLCD();
    //delay(110);

    //Entry mode set
    P1OUT = 0x00;
    writeToLCD();
    P1OUT = 0x60;
    writeToLCD();
*/
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
    //TB0CTL |= TBCLR;
    //TB0CTL |= TBSSEL__ACLK;
    //TB0CTL |= MC__UP;

    //-- 3. Configure Pins
    P1SEL1 &= ~BIT3;        //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;        //P1.2 = SDA
    P1SEL0 |= BIT2;
    //-- 3.1
    P1DIR |= BIT7|BIT6|BIT5|BIT4;
    P2DIR |= BIT7|BIT6|BIT0;

    P1OUT &= ~BIT7|~BIT6|~BIT5|~BIT4;
    //rs 2.6, en 2.0
    P2OUT &= ~BIT6|~BIT0; //Clear RS and enable;

    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCRXIE0;
    __enable_interrupt();

    //init_LCD();

    while(1) {
        delay(1);
        /*
        switch(data_in) {
            case '0':
                break;
            case '1':
                delay(1000);
                break;
            case '2':
                break;
            case '3':
                break;
            case '4':
                break;
            case '5':
                break;
            case '6':
                break;
            case '7':
                break;
            case '8':
                break;
            case '9':
                break;
            case 'A':
                break;
            case 'B':
                break;
            case 'C':
                break;
            case 'D':
                break;
            case '*':
                break;
            case '#':
                break;
            default:
                break;
        }*/
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
