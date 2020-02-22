//Lizzy Hamaoka & Tristan Mullin
//2/13/2020
//Lab 2: I2C communication between devices triggered by Keypad input

#include <msp430.h> 

char data_in = 'A';
char temp;
void delay() {
    int i;
    for(i=0; i<10000; i++) {}
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
        P2DIR |= BIT0|BIT6|BIT7; //setting the data direction for port 2 pins
        P1DIR |= BIT7|BIT6|BIT5|BIT4|BIT1|BIT0; // setting the data direction for the port 1 pins

        //-- 3.1

        PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
        //-- 4. Take eUSCI_B0 out of SW reset
        UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
        //-- 5. Enable Interrupts
        UCB0IE |= UCRXIE0;
        __enable_interrupt();

    //patterns for to display on the LEDs
    int patternB[8] = {0xF2, 0xF1, 0xF3, 0xF3, 0xE3, 0xD3, 0xB3, 0x73};
    int patternC[4] = {0x10, 0x20, 0x42, 0x81};
    int patternD[6] = {0x30, 0x70, 0xF0, 0xE0, 0xC0, 0x80};

    int i;
    int length;
    while(1) {
        switch (data_in) {
            case 'B':   //Runs through pattern B if B was received from master
                length = sizeof(patternB)/sizeof(patternB[0]);
                while(data_in == 'B') {
                    for(i=0; i<length; i++) {
                        P1OUT = patternB[i];
                        if(i == 2) {
                            P2OUT = 0x80;
                        } else if (i == 3) {
                            P2OUT = 0x40;
                        } else {
                            P2OUT = 0xC0;
                        }
                        P2OUT |= BIT0;
                        delay();
                        P2OUT &= ~BIT0;
                    }
                }
                break;
            case 'C': //Runs through pattern C if C was received from master
                length = sizeof(patternC)/sizeof(patternC[0]);
                while (data_in == 'C') {
                    for(i=0; i<length; i++){
                        P1OUT = patternC[i];
                        if(i == 0 || i == 7) {
                            P2OUT = 0x80;
                        } else if (i == 1 || i == 5) {
                            P2OUT = 0x40;
                        } else {
                            P2OUT = 0x00;
                        }
                        P2OUT |= BIT0;
                        delay();
                        P2OUT &= ~BIT0;

                    }
                    for (i=length-1; i>=0; i--) {
                        P1OUT = patternC[i];
                        if(i == 0 || i == 7) {
                            P2OUT = 0x80;
                        } else if (i == 1 || i == 5) {
                            P2OUT = 0x40;
                        } else {
                            P2OUT = 0x00;
                        }
                        P2OUT |= BIT0;
                        delay();
                        P2OUT &= ~BIT0;
                    }
                }
                break;
            case 'D':   //Runs through pattern D if D was received from master
                length = sizeof(patternD)/sizeof(patternD[0]);
                while (data_in == 'D') {
                    for(i=0; i<length; i++){
                        P1OUT = patternD[i];
                        if(i == 0 || i == 10) {
                            P2OUT = 0xC0;
                        } else if (i == 1 || i == 9) {
                            P2OUT = 0x80;
                        } else {
                            P2OUT = 0x00;
                        }
                        P2OUT |= BIT0;
                        delay();
                        P2OUT &= ~BIT0;
                    }
                    for (i=length-1; i>=0; i--) {
                        P1OUT = patternD[i];
                        if(i == 0 || i == 10) {
                            P2OUT = 0xC0;
                        } else if (i == 1 || i == 9) {
                            P2OUT = 0x80;
                        } else {
                            P2OUT = 0x00;
                        }
                        P2OUT |= BIT0;
                        delay();
                        P2OUT &= ~BIT0;
                    }
                }
                break;
            case 'A':   //Runs through pattern A if A was received from master Also default pattern upon reset
                P1OUT = 0x51;
                P2OUT = 0x40;

                P2OUT |= BIT0;
                for(j=0; j<1000; j++) {}
                P2OUT &= ~BIT0;
            default:
                temp = data_in;
                break;
        }
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
#pragma vector=USCI_B0_VECTOR   //interrupt for receiving data from master device
__interrupt void USCI_B0_ISR(void) {
    switch(UCB0IV) {
        case 0x16:
            data_in = UCB0RXBUF;
            temp = (char) data_in;
            break;
        default:
            break;
    }
}
