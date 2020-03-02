//Lizzy Hamaoka & Tristan Mullin
//3/3/2020
//Lab 3: Setting up an LCD to display characters from keypad input
#include <msp430.h> 
/**
 * main.c
 */

char data_in = 'x';
unsigned int position = 0;

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
    delay(1700); // Start up delay
    //function set 8-bit, delay > 4.1ms
    writeToLCD(0x30, 500);
    //function set 8-bit, delay > 150us
    writeToLCD(0x30, 10);
    //function set, delay 8-bit, > 40us
    writeToLCD(0x30, 2);

    //function set 4-bit, delay > 40us
    writeToLCD(0x20, 2);
    //function set 4-bit
    writeToLCD(0x20, 2);
    writeToLCD(0x80, 2); //NF**, N = # of lines, F = Char font

    //Display on
    writeToLCD(0x00, 2);
    writeToLCD(0xF0, 2); //1DCB, D = toggle display on/off, C = cursor, B = blink

    //Clear display
    writeToLCD(0x00, 2);
    delay(170);
    writeToLCD(0x10, 2);
    delay(170);

    //Entry Mode Set
    writeToLCD(0x00, 2);
    writeToLCD(0x60, 2); //01I/DS, I/D = increment/decrement, S = shifts the display when 1
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

    while(1) {
        //need to check if near the end of a line and/or if the display is full
        switch(data_in) {
            case '0':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x00, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '1':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x10, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '2':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x20, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '3':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x30, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '4':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x40, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '5':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x50, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '6':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x60, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '7':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x70, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '8':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x80, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '9':
                setRS_RW(1, 1);
                writeToLCD(0x30, 2);
                setRS_RW(1, 0);
                writeToLCD(0x90, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case 'A':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x10, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case 'B':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x20, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case 'C':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x30, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case 'D':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x40, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '*':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x50, 2);
                setRS_RW(0, 0);
                position++;
                break;
            case '#':
                setRS_RW(1, 1);
                writeToLCD(0x40, 2);
                setRS_RW(1, 0);
                writeToLCD(0x60, 2);
                setRS_RW(0, 0);
                position++;
                break;
            default:
                break;
        }
        data_in = 'x';
        checkEndOfScreen();
    }
    return 0;
}
//------- Interrupt Service Routines ---------------------------
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
