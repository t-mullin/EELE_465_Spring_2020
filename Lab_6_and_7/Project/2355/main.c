//Lizzy Hamaoka & Tristan Mullin
//5/4/2020
//Lab 6 & 7: Temperature readings from LM92, Time readings DS1337 RTC being displayed on a LCD, and
//Thermoelectric cooler state controller.
#include <msp430.h> 
#include <math.h>

//-- Global Variables
char input = '0';
int data_ready = 0;
int position = 0;
int mode = 0;
int resetClock = 0;
int getLow = 0;
int tempLM92;
float tempTemp = 0;
int getMins = 0;
int tempSeconds;
int seconds;
int secHigh;
int secLow;
int minutes;
int minHigh;
int minLow;
char message[7]; // ie 1230232
//-- End of Global Variables

//-- Initialization Functions
void init_I2C() {
    //-- 1. Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;              //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 2. Configure eUSCI_B0
    UCB0CTLW0 |= UCSSEL_3;             //Choose BRCLK=SMCLK=1MHz
    UCB0BRW = 10;                      //Divide BRCLK by 10 for SCL=100kHz
    UCB0CTLW0 |= UCMODE_3;             //Put into I2C mode
    UCB0CTLW0 |= UCMST;                //Put into master mode
    UCB0CTLW0 |= UCTR;                 //Put into Tx mode
    UCB0I2CSA = 0x0042;                //Initial Slave address = 0x42
    UCB0CTLW1 |= UCASTP_2;             //Auto STOP when UCB0TBCNT reached
    UCB0TBCNT = sizeof(message);       //Send 7 bytes of data
    //-- 3. Configure Pins
    P1SEL1 &= ~BIT3;                   //P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;                   //P1.2 = SDA
    P1SEL0 |= BIT2;
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;             //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCTXIE0;                 //Enable I2C Tx0 IRQ
    UCB0IE |= UCRXIE0;                 //Enable I2C Rx0 IRQ
}

void init_UART() {
    //-- 1. Put eUSCI_A0 into software reset
    UCA0CTLW0 |= UCSWRST;              //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 2. Configure eUSCI_A0
    UCA0CTLW0 |= UCSSEL__ACLK;         //Choose BRCLK=ACLK=32.768kHz
    UCA0BRW = 3;                       //Pre-scaler = 3
    UCA0MCTLW |= 0x9200;               //Modulation
    //-- 3.
    P1SEL1 &= ~BIT6;                   //P1.6 = RX
    P1SEL0 |= BIT6;
    P1SEL1 &= ~BIT7;                   //P1.7 = TX
    P1SEL0 |= BIT7;
    //-- 4. Take eUSCI_A0 out of SW reset
    UCA0CTLW0 &= ~UCSWRST;             //UCSWRST=1 for eUSCI_A0 in SW reset
    //-- 5. Enable Interrupts
    UCA0IE |= UCRXIE;                  //Enable UART RX IRQ
}

void init_Keypad() {
    P2DIR = 0xF0;                      //Upper 4 bits set as outputs, Lower 4 bits set as inputs
    P2OUT = 0x0F;
    P2REN = 0x0F;
    //-- 5. Enable Interrupts
    P2IFG = 0x00;
    P2IES = 0x0F;
    P2IE = 0x0F;

}

void init_TimerB() {
    TB0CTL |= TBCLR;                   //Clears timer and dividers
    TB0CTL |= TBSSEL__ACLK;            //Clock Source: ACLK=32.768kHz
    TB0CTL |= MC__CONTINUOUS;          //Clock Mode: Continuous
    TB0CTL |= CNTL_1;                  //Clock Bit Length: 12-bit
    TB0CTL |= ID__8;                   //Clock Divider: 8
    //-- 5. Enable Interrupts
    TB0CTL |= TBIE;                    //Enable Timer B0 IRQ
    TB0CTL &= ~TBIFG;                  //Clears Timer B0 Flag

}

void init_ControlLines() {
    P4DIR |= BIT1|BIT0;
    P4OUT &= ~BIT1|~BIT0;
}
//-- End of Initialization Functions

//simple for loop based delay function
void delay(int delayNum) {
    int i;
    for(i = 0; i < delayNum; i++) {}
}

//changes the number of bytes that are sent by I2C
void setI2CByteNum(int bytes) {
    //-- 1. Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;              //UCSWRST=1 for eUSCI_B0 in SW reset
    if(bytes == 7) {
        UCB0TBCNT = sizeof(message);   //Send 7 bytes of data
    } else if (bytes == 1) {
        UCB0TBCNT = 0x01;              //Send 1 byte of data
    } else if (bytes == 2) {
        UCB0TBCNT = 0x02;              //Send 2 bytes of data
    } else if (bytes == 3) {
        UCB0TBCNT = 0x03;              //Send 3 bytes of data
    }
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;             //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCTXIE0;                 //Enable I2C Tx0 IRQ
    UCB0IE |= UCRXIE0;                 //Enable I2C Rx0 IRQ
}

//gets the temperature from LM92 temperature sensor
void getTemperature() {
    setI2CByteNum(1);
    mode = 1;
    UCB0CTLW0 |= UCTR;                 //Put into Tx mode
    UCB0I2CSA = 0x0048;                //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    setI2CByteNum(2);
    UCB0CTLW0 &= ~UCTR;                //Put into Rx mode
    UCB0I2CSA = 0x0048;                //Slave address = 0x48 LM92
    UCB0CTLW0 |= UCTXSTT;
    delay(100);

    int i;
    for(i = 0; i < 3; i++) {
        message[3-i] = (tempLM92 % 10) + '0';
        tempLM92 /= 10;
    }
}

//clears the DS1337 RTCs first 2 registers (seconds & minutes)
void clearClock() {
    TB0CTL &= ~TBIE;                    //Enable Timer B0 IRQ

    setI2CByteNum(3);
    mode = 2;
    UCB0CTLW0 |= UCTR;                 //Put into Tx mode
    UCB0I2CSA = 0x0068;                //Slave address = 0x68 DS1337 RTC
    UCB0CTLW0 |= UCTXSTT;
    delay(100);

    init_TimerB();
}

//gets the time from the DS1337 RTCs first 2 registers (seconds & minutes)
void getTime() {
    //resets the clock if there has been any new input
    if(resetClock == 1) {
        clearClock();
        resetClock = 0;
    }

    setI2CByteNum(1);
    mode = 2;
    UCB0CTLW0 |= UCTR;                 //Put into Tx mode
    UCB0I2CSA = 0x0068;                //Slave address = 0x68 DS1337 RTC
    UCB0CTLW0 |= UCTXSTT;
    delay(100);
    setI2CByteNum(2);
    UCB0CTLW0 &= ~UCTR;                //Put into Rx mode
    UCB0I2CSA = 0x0068;                //Slave address = 0x68 DS1337 RTC
    UCB0CTLW0 |= UCTXSTT;
    delay(100);

    tempSeconds = (((minHigh*10) + minLow)*60) + ((secHigh*10) + secLow);

    //16mins & 59secs
    if(tempSeconds > 999) {
        clearClock();
    }

    seconds = tempSeconds;

    int i;
    for(i = 0; i < 3; i++) {
        if(message[0] == '0') {
            message[6-i] = '0';
        } else {
            message[6-i] = (tempSeconds % 10) + '0';
            tempSeconds /= 10;
        }
    }
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
    data_ready = 0;
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_I2C();             //initialize I2C module
    init_UART();            //initialize UART module
    init_Keypad();          //initialize Keypad
    init_TimerB();          //initialize TimerB
    init_ControlLines();    //initialize Control Lines for TEC

    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM

    __enable_interrupt();   //Enable Maskable IRQs

    clearClock();

    while(1){
        if(data_ready == 1) {
            switch(input) {
                case '0':
                    message[0] = '0';
                    getTime();
                    if(seconds % 2 == 0) {
                        getTemperature();
                    }
                    sendDataToSlave();
                    P4OUT &= ~BIT0;
                    P4OUT &= ~BIT1;
                    break;
                case '1':
                    message[0] = '1';
                    getTime();
                    if(seconds % 2 == 0) {
                        getTemperature();
                    }
                    sendDataToSlave();
                    P4OUT |= BIT1;
                    P4OUT &= ~BIT0;
                    break;
                case '2':
                    message[0] = '2';
                    getTime();
                    if(seconds % 2 == 0) {
                        getTemperature();
                    }
                    sendDataToSlave();
                    P4OUT |= BIT0;
                    P4OUT &= ~BIT1;
                    break;
            }
        }
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
//TIMER B
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TB0_Overflow_ISR(void) {
    data_ready = 1;
    TB0CTL &= ~TBIFG;
}

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
                    minLow = minutes & 0x000F;
                    minHigh = minutes >> 4;
                    getMins = 0;
                } else {
                    seconds = UCB0RXBUF;
                    secLow = seconds & 0x000F;
                    secHigh = seconds >> 4;
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
    UCA0TXBUF = input;
    resetClock = 1;
    data_ready = 1;
}

//PORT2
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
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
    resetClock = 1;
    //resets the direction, input mask, and resistors
    P2DIR = 0xF0;           //Upper 4 bits set as outputs, Lower 4 bits set as inputs
    P2OUT = 0x0F;
    P2REN = 0x0F;
    P2IFG = 0x00;   //clear the interrupt flag
}
