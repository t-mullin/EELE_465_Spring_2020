//Lizzy Hamaoka & Tristan Mullin
//5/4/2020
//Lab 6 & 7: Temperature readings from LM92, Time readings DS1337 RTC being displayed on a LCD, and
//Thermoelectric cooler state controller.
#include <msp430.h> 

int data_ready = 0;
int messagePosition = 0;
char message[7];

//sends the hex values to the dff
void sendHex() {
    int i;
    P2OUT &= ~BIT6;
    P2OUT |= BIT6;
    for(i = 4; i > 0; i--) {
        P2OUT |= BIT0;
        P2OUT &= ~BIT0;
        P1OUT = P1OUT << 1;
    }
    data_ready = 0;
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
        P2DIR |= BIT0|BIT6;
        P1DIR |= BIT7|BIT6|BIT5|BIT4;
        PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
        //-- 4. Take eUSCI_B0 out of SW reset
        UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
        //-- 5. Enable Interrupts
        UCB0IE |= UCRXIE0;
        __enable_interrupt();

    while(1) {
        if(data_ready == 1) {
            switch (message[0]) {
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
                    data_ready = 0;
                    break;
            }
        }
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
//I2C
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch(UCB0IV) {
        case 0x16:
            if(messagePosition == (sizeof(message) - 1)) {
                message[messagePosition] = UCB0RXBUF;
                messagePosition = 0;
                data_ready = 1;
            } else {
                message[messagePosition] = UCB0RXBUF;
                messagePosition++;
            }
            break;
        default:
            break;
    }
}
