//Lizzy Hamaoka & Tristan Mullin
//3/31/2020
//Lab 5: Temperature readings from LM19 and MSP430 being displayed on a LCD
#include <msp430.h> 

//-- Define Constants
#define CALADC12_1_5V_30C *((unsigned int *)0x1A1A)
//-- End of Define Constants

//-- Global Variables
char input;
int data_ready = 0;
int position = 0;
char message[11]; // ie 12302323023
float LM19TempK, LM19TempC;
float MSP430TempK, MSP430TempC;
unsigned int V_temp;
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

void init_ADC() {
    //-- 1.C Initialize ADC & Temp Sensor
    PMMCTL2 |= TSENSOREN;   //enable internal temp sensor
    ADCCTL0 &= ~ADCSHT;     //Clear ADCSHT from def. of ADCSHT=01
    ADCCTL0 |= ADCSHT_2;    //Conversion Cycles = 16 (ADCSHT=10)
    ADCCTL0 |= ADCON;       //Turn ADC on
    ADCCTL1 |= ADCSSEL_2;   //ADC Clock Source = SMCLK
    ADCCTL1 |= ADCSHP;      //Sample signal source = sampling timer
    ADCCTL2 &= ~ADCRES;     //Clear ADCRES from def. of ADCRES=01
    ADCCTL2 |= ADCRES_2;    //Resolution = 12-bit (ADCRES=10)
    ADCMCTL0 |= ADCINCH_11; //ADC Input Channel = A11 (P5.3)
    ADCIE |= ADCIE0;        //Enable ADC Conv Complete IRQ
    //-- 2.C Configure Pins
    P5SEL1 |= BIT3;         //Sets P5.3 to be an ADC input
    P5SEL0 |= BIT3;
}
//-- End of Initialization Functions

//simple for loop based delay function
void delay(int delayNum) {
    int i;
    for(i = 0; i < delayNum; i++) {}
}

//starts the I2C and UART transmissions.
void sendDataToSlave() {
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

//function to get the temperature from the LM19 sensor
void getLM19Temperature() {
    int i;
    ADCCTL0 &= ~ADCENC;
    ADCMCTL0 = 0x000B;  //sets the ADC channel to 11 (pin 40)
    for(i = 0; i < (int)input-48; i++) {
        ADCCTL0 |= ADCENC | ADCSC;
        while((ADCIFG & ADCIFG0) == 0); //waits until conversion is done
        LM19TempC += (float)(1.8641 - ((float)V_temp*0.00080859))/0.01171; //formula from datasheet for -10C to 65C
    }
    LM19TempC = LM19TempC / (input-48); //averaging the temperature
    LM19TempK = LM19TempC + 273.15; //getting the temperature in kelvin
}

//function to get the temperature from the MSP430 temperature sensor
void getMSP430Temperature() {
    int i;
    ADCCTL0 &= ~ADCENC;
    ADCMCTL0 = 0x000C;  //sets the ADC channel to 12 (internal temp sensor)
    for(i = 0; i < (int)input-48; i++) {
        ADCCTL0 |= ADCENC | ADCSC;
        while((ADCIFG & ADCIFG0) == 0); //waits until conversion is done
        MSP430TempC += (float)((long)V_temp - CALADC12_1_5V_30C) * 0.00355 + 30; //formula from datasheet
    }
    MSP430TempC = MSP430TempC / (input-48); //averaging the temperature
    MSP430TempK = MSP430TempC + 273.15; //getting the temperature in kelvin
}

//calls the functions to get the temperature data from the sensors
void getTemperature() {
    getLM19Temperature();
    getMSP430Temperature();
}

//clears the temperature data
void clearTemperature() {
    LM19TempC = 0;
    LM19TempK = 0;
    MSP430TempC = 0;
    MSP430TempK = 0;
}

//packages the data into a char array
void generateMessage() {
    int i;
    int temp;
    int temp1;
    message[0] = input;

    temp = (int)LM19TempK;
    temp1 = (int)MSP430TempK;
    for(i = 0; i < 3; i++) {
        message[3-i] = (temp % 10) + '0';
        message[8-i] = (temp1 % 10) + '0';
        temp /= 10;
        temp1 /= 10;
    }
    temp = (int)LM19TempC;
    temp1 = (int)MSP430TempC;
    for(i = 0; i < 2; i++) {
        message[5-i] = (temp % 10) + '0';
        message[10-i] = (temp1 % 10) + '0';
        temp /= 10;
        temp1 /= 10;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init_I2C();     //initialize I2C module
    init_UART();    //initialize UART module
    init_ADC();     //initialize ADC module
    init_Keypad();  //initialize Keypad

    PM5CTL0 &= ~LOCKLPM5;   //Disable LPM

    __enable_interrupt();   //Enable Maskable IRQs

    while(1){
        if(data_ready == 1) {
            switch(input) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    getTemperature();
                    generateMessage();
                    sendDataToSlave();
                    clearTemperature();
                    break;
                case '*':
                    message[0] = input;
                    sendDataToSlave();
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
    switch (UCB0IV) {
        case 0x18: //Vector 24 TXIFG0
            if(position == (sizeof(message) - 1)) {
                UCB0TXBUF = message[position];
                position = 0;
            } else {
                UCB0TXBUF = message[position];
                position++;
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

//ADC
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void) {
    V_temp = ADCMEM0;
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
