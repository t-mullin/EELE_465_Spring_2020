//Lizzy Hamaoka & Tristan Mullin
//3/31/2020
//Lab 5: Temperature readings from LM19 and MSP430 being displayed on a LCD
#include <msp430.h> 

unsigned int position = 0;
int messagePosition = 0;
int data_ready = 0;
char message[11];
char enterMessage[] = "Enter n:";
char LM19Message[]  = " T,K:kkk T,C:cc ";
char MSP430Message[]  = " T,K:kkk T,C:cc ";

//simple for loop based delay function
void delay(unsigned int i) {
    unsigned int j;
    //1700 is ~16 ms
    //500 is ~4.8 ms
    //10 is ~130 us
    //2 is ~53.2 us
    for(j = 0; j < i; j=j+1) {}
}

//Writes the nibble to the lcd
void writeToLCD(int nibble, unsigned int delayNum) {
    P1OUT = nibble;
    P2OUT |= BIT0;
    P2OUT &= ~BIT0;
    delay(delayNum);
}

//software initialization of the LCD
void init_LCD() {
    //after flashing, remove the debug RX and TX pins
    //they were causing the lcd to not display correctly
    delay(1700);           // Start up delay
    writeToLCD(0x30, 500); //function set 8-bit, delay > 4.1ms
    writeToLCD(0x30, 10);  //function set 8-bit, delay > 150us
    writeToLCD(0x30, 2);   //function set, delay 8-bit, > 40us

    writeToLCD(0x20, 2);   //function set 4-bit, delay > 40us
    writeToLCD(0x20, 2);   //function set 4-bit
    writeToLCD(0x80, 2);   //NF**, N = # of lines, F = Char font

    writeToLCD(0x00, 2);   //Display on
    writeToLCD(0xF0, 2);   //1DCB, D = toggle display on/off, C = cursor, B = blink

    writeToLCD(0x00, 172);   //Clear display
    writeToLCD(0x10, 172);

    writeToLCD(0x00, 2);   //Entry Mode Set
    writeToLCD(0x60, 2);   //01I/DS, I/D = increment/decrement, S = shifts the display when 1
}

//sets the rs and rw depending on what needs to be written or read
void setRS_RW(unsigned int rs, unsigned int rw) {
    if (rs == 1) {
        P2OUT |= BIT6;
    } else {
        P2OUT &= ~BIT6;
    }
    if(rw == 1) {
        P1OUT |= BIT0;
    } else {
        P1OUT &= ~BIT0;
    }
}

//checks to see if the cursor has reached the end of the screen then wraps or clears the screen
void checkEndOfScreen() {
    setRS_RW(0, 0);
    if(position == 16) {
        writeToLCD(0xC0, 2);
        writeToLCD(0x00, 2);
    } else if (position == 33) {
        writeToLCD(0x00, 2);
        writeToLCD(0x10, 2);
        position = 0;
    }
}

//Writes the character to the LCD screen
void writeChar(char input) {
    setRS_RW(1, 1);
    writeToLCD(input, 2);
    setRS_RW(1, 0);
    input = input << 4;
    writeToLCD(input, 2);
    setRS_RW(0, 0);
    position++;
}

//writes the "Enter n:" message to the LCD
void writeDefault() {
    int i;
    for(i = 0; i < sizeof(enterMessage)-1; i++) {
        writeChar(enterMessage[i]);
    }
}

//sets the temperature data into the correct spots in the temperature messages
void setTemperature() {
    LM19Message[5] = message[1];
    LM19Message[6] = message[2];
    LM19Message[7] = message[3];
    LM19Message[13] = message[4];
    LM19Message[14] = message[5];

    MSP430Message[5] = message[6];
    MSP430Message[6] = message[7];
    MSP430Message[7] = message[8];
    MSP430Message[13] = message[9];
    MSP430Message[14] = message[10];
}

//writes the temperature messages to the LCD
void writeTemperature() {
    int i;
    setTemperature();
    for(i = 0; i < sizeof(LM19Message)-1; i++) {
        writeChar(LM19Message[i]);
    }
    checkEndOfScreen();
    for(i = 0; i < sizeof(MSP430Message)-1; i++) {
        writeChar(MSP430Message[i]);
    }
}

//clears the LCD screen
void clearScreen() {
    setRS_RW(0, 0);
    writeToLCD(0x00, 172);
    writeToLCD(0x10, 172);
    position = 0;
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
    P1DIR |= BIT7|BIT6|BIT5|BIT4|BIT0;
    P2DIR |= BIT6|BIT0;
    P1OUT &= ~BIT7|~BIT6|~BIT5|~BIT4|~BIT0;
    // r/w 1.0, rs 2.6, en 2.0
    P2OUT &= ~BIT6|~BIT0; //Clear R/W, RS, and enable;
    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM
    //-- 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;  //UCSWRST=1 for eUSCI_B0 in SW reset
    //-- 5. Enable Interrupts
    UCB0IE |= UCRXIE0;
    __enable_interrupt();

    init_LCD();

    writeDefault();

    while(1) {
        //checks to see if there has been any data has been received
        if(data_ready == 1) {
            switch(message[0]) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    writeChar(message[0]);
                    delay(50000);
                    clearScreen();
                    writeTemperature();
                    data_ready = 0;
                    break;
                case '*':
                    clearScreen();
                    writeDefault();
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
