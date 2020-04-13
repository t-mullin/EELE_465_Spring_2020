//Lizzy Hamaoka & Tristan Mullin
//3/31/2020
//Lab 5: Temperature readings from LM19 and MSP430 being displayed on a LCD

/*
 * TODO: change some variable names to make it clearer what they are for.
 *
 * */

#include <msp430.h> 
#include <math.h>

//-- Define Constants
#define CALADC12_1_5V_30C *((unsigned int *)0x1A1A)
//-- End of Define Constants

//-- Global Variables
char input;
int data_ready = 0;
int position = 0;
int mode = 0;
int getLow = 0;
int tempLM92;
float tempTemp = 0;
int getMins = 0;
int seconds;
int minutes;
char message[7]; // ie 1230232
//-- End of Global Variables

//-- Initialization Functions
void init_I2C() {
    //-- 1.A Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 2.A Configure eUSCI_B0
    UCB0CTLW0 |= UCSSEL_3;  //Choose BRCLK=SMCLK=1MHz
    UCB0BRW = 10;           //Divide BRCLK by 10 for SCL=100kHz
    UCB0CTLW0 |= UCMODE_3;  //Put into I2C mode
    UCB0CTLW0 |= UCMST;     //Put into master mode
    UCB0CTLW0 |= UCTR;      //Put into Tx mode
    UCB0I2CSA = 0x0042;     //Initial Slave address = 0x42
    UCB0CTLW1 |= UCASTP_2;  //Auto STOP when UCB0TBCNT reached
    UCB0TBCNT = sizeof(message);       //Send eleven bytes of data
    //-- 3.A Configure Pins
    P1SEL1 &= ~BIT3;        //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;        //P1.2 = SDA
    P1SEL0 |= BIT2;
    //-- 4.A Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCTXIE0;      //Enable I2C Tx0 IRQ
    UCB0IE |= UCRXIE0;      //Enable I2C Rx0 IRQ
}

void init_UART() {
    //-- 1.B Put eUSCI_A0 into software reset
    UCA0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 2.B Configure eUSCI_A0
    UCA0CTLW0 |= UCSSEL__ACLK;  //Choose BRCLK=ACLK=32.768kHz
    UCA0BRW = 3;
    UCA0MCTLW |= 0x9200;
    //-- 3.B
    P1SEL1 &= ~BIT6;        //P1.6 = RX
    P1SEL0 |= BIT6;
    P1SEL1 &= ~BIT7;        //P1.7 = TX
    P1SEL0 |= BIT7;
    //-- 4.B Take eUSCI_A0 out of SW reset
    UCA0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_A0 in SW reset
    //-- 5. Enable Interrupts
    UCA0IE |= UCRXIE;       //Enable UART RX IRQ
}

void init_Keypad() {
    P2DIR = 0xF0;           //Upper 4 bits set as outputs, Lower 4 bits set as inputs
    P2OUT = 0x0F;
    P2REN = 0x0F;
    //-- 5. Enable Interrupts
    P2IFG = 0x00;
    P2IES = 0x0F;
    P2IE = 0x0F;

}
//-- End of Initialization Functions

//simple for loop based delay function
void delay(int delayNum) {
    int i;
    for(i = 0; i < delayNum; i++) {}
}

//changes the number of bytes that are sent by I2C
void setI2CByteNum(int bytes) {
    //-- 1.A Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;   //UCSWRST=1 for eUSCI_B0 in SW reset
    if(bytes == 7) {
        UCB0TBCNT = sizeof(message);       //Send eleven bytes of data
    } else if (bytes == 1) {
        UCB0TBCNT = 0x01;
    } else if (bytes == 2) {
        UCB0TBCNT = 0x02;
    }
    //-- 4.A Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCTXIE0;      //Enable I2C Tx0 IRQ
    UCB0IE |= UCRXIE0;      //Enable I2C Rx0 IRQ
}

//gets the temperature from LM92 temperature sensor
void getTemperature() {
    setI2CByteNum(1);
    mode = 1;
    UCB0CTLW0 |= UCTR;      //Put into Tx mode
    UCB0I2CSA = 0x0048;     //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    setI2CByteNum(2);
    UCB0CTLW0 &= ~UCTR;      //Put into Rx mode
    UCB0I2CSA = 0x0048;     //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);

    int i;
    for(i = 0; i < 3; i++) {
        message[3-i] = (tempLM92 % 10) + '0';
        tempLM92 /= 10;
    }
}

void getTime() {
    setI2CByteNum(1);
    mode = 2;
    UCB0CTLW0 |= UCTR;      //Put into Tx mode
    UCB0I2CSA = 0x0068;     //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    setI2CByteNum(1);
    UCB0CTLW0 &= ~UCTR;      //Put into Rx mode
    UCB0I2CSA = 0x0068;     //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
}

//starts the I2C and UART transmissions.
void sendDataToSlave() {
    mode = 3;
    setI2CByteNum(7);     //initialize I2C module
    UCB0CTLW0 |= UCTR;      //Put into Tx mode
    UCB0I2CSA = 0x0042;     //Slave address = 0x42 LED
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    UCB0I2CSA = 0x0043;     //Slave address = 0x43 LCD
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    UCA0TXBUF = input;      //sending data back to terminal
    delay(10000);
    data_ready = 0;
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_I2C();     //initialize I2C module
    init_UART();    //initialize UART module
    init_Keypad();  //initialize Keypad

    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM

    __enable_interrupt();   //Enable Maskable IRQs

    while(1){
        getTime();
        /*
        if(data_ready == 1) {
            switch(input) {
                case '0':
                    message[0] = '0';
                    getTemperature();
                    sendDataToSlave();
                    break;
                case '1':
                    message[0] = '1';
                    getTemperature();
                    sendDataToSlave();
                    break;
                case '2':
                    message[0] = '2';
                    getTemperature();
                    sendDataToSlave();
                    break;
            }
        }
        */
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
//I2C
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch (UCB0IV) {
        case 0x16:
            if(mode == 1) {
                if(getLow == 1) {
                    tempLM92 += UCB0RXBUF;
                    tempLM92 = tempLM92 >> 3;
                    tempTemp = tempLM92*0.0625;
                    tempLM92 = round(273.15+tempTemp);
                    getLow = 0;
                } else {
                    tempLM92 = UCB0RXBUF;
                    tempLM92 = tempLM92 << 8;
                    getLow = 1;
                }
            } else {
                if(getMins == 1) {
                    minutes = UCB0RXBUF;
                    seconds += 60*minutes;
                    getMins = 0;
                } else {
                    seconds = UCB0RXBUF;
                    getMins = 1;
                }
            }
            break;
        case 0x18: //Vector 24 TXIFG0
            if(mode == 1 || mode == 2) {
                UCB0TXBUF = 0x00;
            } else {
                if(position == (sizeof(message) - 1)) {
                    UCB0TXBUF = message[position];
                    position = 0;
                } else {
                    UCB0TXBUF = message[position];
                    position++;
                }
            }
            break;
        default:
            break;
    }
}

//UART
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_RX_ISR(void) {
    input = UCA0RXBUF;
    data_ready = 1;
}

//PORT2
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
