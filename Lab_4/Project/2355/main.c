//Lizzy Hamaoka & Tristan Mullin
//3/3/2020
//Lab 3: Setting up an LCD to display characters from keypad input
#include <msp430.h> 


/**
 * main.c
 */
char input;
int data_ready = 0;

void delay(int delayNum) {
    int i;
    for(i = 0; i < delayNum; i++) {}
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //-- 1.A Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 1.B Put eUSCI_A0 into software reset
    UCA0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset

    //-- 2.A Configure eUSCI_B0
    UCB0CTLW0 |= UCSSEL_3;  //Choose BRCLK=SMCLK=1MHz
    UCB0BRW = 10;           //Divide BRCLK by 10 for SCL=100kHz
    UCB0CTLW0 |= UCMODE_3;  //Put into I2C mode
    UCB0CTLW0 |= UCMST;     //Put into master mode
    UCB0CTLW0 |= UCTR;      //Put into Tx mode
    UCB0I2CSA = 0x0042;     //Initial Slave address = 0x42
    UCB0CTLW1 |= UCASTP_2;  //Auto STOP when UCB0TBCNT reached
    UCB0TBCNT = 0x01;       //Send one byte of data

    //-- 2.B Configure eUSCI_A0
    UCA0CTLW0 |= UCSSEL__ACLK;  //Choose BRCLK=ACLK=32.768kHz
    UCA0BRW = 3;
    UCA0MCTLW |= 0x9200;


    //-- 3.A Configure Pins
    P1SEL1 &= ~BIT3;        //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;        //P1.2 = SDA
    P1SEL0 |= BIT2;

    //-- 3.B
    P1SEL1 &= ~BIT6;        //P1.6 = RX
    P1SEL0 |= BIT6;
    P1SEL1 &= ~BIT7;        //P1.7 = TX
    P1SEL0 |= BIT7;

    //-- 3.C
    P2DIR = 0xF0;           //Upper 4 bits set as outputs, Lower 4 bits set as inputs
    P2OUT = 0x0F;
    P2REN = 0x0F;


    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM

    //-- 4.A Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 4.B Take eUSCI_A0 out of SW reset
    UCA0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_A0 in SW reset

    //-- 5. Enable Interrupts
    UCB0IE |= UCTXIE0;      //Enable I2C Tx0 IRQ
    UCA0IE |= UCRXIE;
    P2IFG = 0x00;
    P2IES = 0x0F;
    P2IE = 0x0F;
    __enable_interrupt();   //Enable Maskable IRQs

    int i = 0;
    while(1){
        if(data_ready == 1) {
            UCB0I2CSA = 0x0042;     //Slave address = 0x42 LED
            UCB0CTLW0 |= UCTXSTT;
            delay(100);
            //for(i=0; i<100; i++){}
            UCB0I2CSA = 0x0043;     //Slave address = 0x43 LCD
            UCB0CTLW0 |= UCTXSTT;
            delay(100);
            //for(i=0; i<100; i++){}
            UCA0TXBUF = input;      //sending data back to terminal
            delay(10000);
            data_ready = 0;
        }
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch (UCB0IV) {
        case 0x18: //Vector 24 TXIFG0
            UCB0TXBUF = input;
            break;
        default:
            break;
    }
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_RX_ISR(void) {
    input = UCA0RXBUF;
    data_ready = 1;
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void){
    switch(P2IV){
        case 0x02:
            P2DIR = 0x0F;
            P2OUT = 0xF0;
            P2REN = 0xF0;
            if(P2IN == 0x70) {
                input = 'A'; //A
                data_ready = 1;
            } else if (P2IN == 0xB0) {
                input = 'B'; //B
                data_ready = 1;
            } else if (P2IN == 0xD0) {
                input = 'C'; //C
                data_ready = 1;
            } else if (P2IN == 0xE0) {
                input = 'D'; //D
                data_ready = 1;
            }
            break;
        case 0x04:
            P2DIR = 0x0F;
            P2OUT = 0xF0;
            P2REN = 0xF0;
            if(P2IN == 0x70) {
                input = '3'; //3
                data_ready = 1;
            } else if (P2IN == 0xB0) {
                input = '6'; //6
                data_ready = 1;
            } else if (P2IN == 0xD0) {
                input = '9'; //9
                data_ready = 1;
            } else if (P2IN == 0xE0) {
                input = '#'; //#
                data_ready = 1;
            }
            break;
        case 0x06:
            P2DIR = 0x0F;
            P2OUT = 0xF0;
            P2REN = 0xF0;
            if(P2IN == 0x70) {
                input = '2'; //2
                data_ready = 1;
            } else if (P2IN == 0xB0) {
                input = '5'; //5
                data_ready = 1;
            } else if (P2IN == 0xD0) {
                input = '8'; //8
                data_ready = 1;
            } else if (P2IN == 0xE0) {
                input = '0'; //0
                data_ready = 1;
            }
            break;
        case 0x08:
            P2DIR = 0x0F;
            P2OUT = 0xF0;
            P2REN = 0xF0;
            if(P2IN == 0x70) {
                input = '1'; //1
                data_ready = 1;
            } else if (P2IN == 0xB0) {
                input = '4'; //4
                data_ready = 1;
            } else if (P2IN == 0xD0) {
                input = '7'; //7
                data_ready = 1;
            } else if (P2IN == 0xE0) {
                input = '*'; //'*'
                data_ready = 1;
            }
            break;
        default:
            break;
    }
    //resets the direction, input mask, and resistors
    P2DIR = 0xF0;           //Upper 4 bits set as outputs, Lower 4 bits set as inputs
    P2OUT = 0x0F;
    P2REN = 0x0F;
    P2IFG = 0x00;   //clear the interrupt flag
}
