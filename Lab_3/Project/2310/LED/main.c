#include <msp430.h> 

char data_in = '0';

void sendHex() {
    int i;
    P2OUT &= ~BIT6;
    P2OUT |= BIT6;
    for(i = 4; i > 0; i--) {
        P2OUT |= BIT0;
        P2OUT &= ~BIT0;
        P1OUT = P1OUT << 1;
    }
    data_in = 'x';
}

int main(void) {
        WDTCTL = WDTPW | WDTHOLD;
        //-- 1. Put eUSCI_B0 into software reset
        UCB0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
        //-- 2. Configure eUSCI_B0
        UCB0CTLW0 |= UCMODE_3;  //Put into I2C mode
        UCB0I2COA0 = 0x0042;     //Slave address = 0x42
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

    while(1) {
        switch (data_in) {
        //all outputs are reversed to feed dff correctly
            case '1':
                P1OUT = 0x80;
                sendHex();
                break;
            case '2':
                P1OUT = 0x40;
                sendHex();
                break;
            case '3':
                P1OUT = 0xC0;
                sendHex();
                break;
            case '4':
                P1OUT = 0x20;
                sendHex();
                break;
            case '5':
                P1OUT = 0xA0;
                sendHex();
                break;
            case '6':
                P1OUT = 0x60;
                sendHex();
                break;
            case '7':
                P1OUT = 0xE0;
                sendHex();
                break;
            case '8':
                P1OUT = 0x10;
                sendHex();
                break;
            case '9':
                P1OUT = 0x90;
                sendHex();
                break;
            case 'A':
                P1OUT = 0x50;
                sendHex();
                break;
            case 'B':
                P1OUT = 0xD0;
                sendHex();
                break;
            case 'C':
                P1OUT = 0x30;
                sendHex();
                break;
            case 'D':
                P1OUT = 0xB0;
                sendHex();
                break;
            case '*':
                P1OUT = 0x70;
                sendHex();
                break;
            case '#':
                P1OUT = 0xF0;
                sendHex();
                break;
            case '0':
                P1OUT = 0x00;
                sendHex();
                break;
            default:
                break;
        }
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
