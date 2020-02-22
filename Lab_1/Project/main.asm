;Lizzy Hamaoka & Tristan Mullin
;1/30/2020
;Lab 1: Manual I2C using bit banging
;
;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
init:
		bis.b		#BIT6, &P4DIR			;SDA PIN 4.6
		bis.b		#BIT7, &P4DIR			;SCL PIN 4.7

		bis.b		#BIT6, &P4OUT
		bis.b		#BIT7, &P4OUT

		bic.b		#LOCKLPM5, &PM5CTL0

		;Setting the time registers in the DS3221
		call		#StartBit		;Starting the I2C data stream
		mov.w		#0D000h, R4		;Slave Address
		call		#TxByte			;Write to Slave device
		mov.w		#00000h, R4		;First Data register in the Slave device
		call		#TxByte			;Write to Slave device
		mov.w		#00000h, R4		;0 Seconds
		call		#TxByte			;Write to Slave device
		mov.w		#06100h, R4		;31 Minutes
		call		#TxByte			;Write to Slave device
		mov.w		#02200h, R4		;2PM
		call		#TxByte			;Write to Slave device
		call 		#StopBit		;Stops the I2C Data stream
		call		#BitDelay
main:

		call		#StartBit

		mov.w		#0D000h, R4		;Slave Address (Write)
		call		#TxByte			;Write to Slave device

		mov.w		#00000h, R4		;First Data register in the Slave device
		call		#TxByte			;Write to Slave device

		bis.b		#BIT6, &P4OUT
		bis.b		#BIT7, &P4OUT

		call 		#StartBit		;Repeated Start

		mov.w		#000D0h, R4		;Slave Address
		inc			R4				;Increments Address for Reading
		swpb		R4				;Slave Address (Read)
		call		#TxByte			;Write to Slave device

		call 		#RxByte			;Reading from Slave device
		call		#AckMe			;Master Ack

		call 		#RxByte			;Reading from Slave device
		call		#AckMe			;Master Ack

		call		#RxByte			;Reading from Slave device
		call		#NackMe			;Master Nack

		call		#StopBit

		jmp			main
;-------------------------------------------------------------------------------
; Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Counting Subroutines
;-------------------------------------------------------------------------------
Count:
		mov.w		#0, R6
Loop:
		mov.w		R6, R4
		swpb		R4
		call 		#TxByte
		inc			R6
		cmp			#10, R6
		jnz			Loop
		ret
;-------------------------------------------------------------------------------
; End of Counting Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Receive Subroutines
;-------------------------------------------------------------------------------
RxByte:
		bic.b		#BIT6, &P4DIR	;sets p4 bit 6 as an input
		mov.w		#8, R5			;bit counter
RxNextBit:							;gets the next bit from the slave
		rlc			R4
		jnc			RxSendLow
RxSendHigh:							;input from slave = 1
		bis.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
		jmp			RxCont
RxSendLow:							;input from slave = 0
		bic.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
RxCont:								;clears clk and decrements the bit counter
		bic.b		#BIT7, &P4OUT
		dec			R5
		cmp			#0, R5
		jnz			RxNextBit		;jumps if there is still more to take in
		ret
AckMe:								;sends the slave device an ack
		bis.b		#BIT6, &P4DIR
		bic.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
		bic.b		#BIT7, &P4OUT
		bis.b		#BIT6, &P4DIR
		ret
NackMe:								;sends the slave device and nack
		bis.b		#BIT6, &P4DIR
		bis.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
		bic.b		#BIT7, &P4OUT
		bis.b		#BIT6, &P4DIR
		ret
;-------------------------------------------------------------------------------
; End of Receive Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Transmit Subroutines
;-------------------------------------------------------------------------------
TxByte:
		mov.w		#8, R5			;Counter for sending bits
NextBit:
		rlc			R4				;rolls the msb into the carry bit
		jnc			SendLow			;carry = 0
SendHigh:							;Sends a 1 and generates a clock cycle
		bis.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
		jmp			TxCont
SendLow:							;Sends a 0 and generates a clock cycle
		bic.b		#BIT6, &P4OUT
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
TxCont:								;clears the clock and decrements the counter
		bic.b		#BIT7, &P4OUT
		dec			R5
		cmp			#0, R5
		jz			AckPoll			;checks for ack after counter is 0
		jmp			NextBit
AckPoll:							;checks for ack from slave device
		bic.b		#BIT6, &P4DIR
		call		#SetupDelay
		bis.b		#BIT7, &P4OUT
		call		#BitDelay
		mov.w		&P4IN, R7
		xor.b		#0BFh, R7
		cmp			#191, R7		;might need to be set to 191, 64
		jz			NoAck
		bic.b		#BIT7, &P4OUT
		bis.b		#BIT6, &P4DIR
		ret

		;sending a ack without a device on other end
;		bic.b		#BIT6, &P4OUT
;		call		#SetupDelay
;		bis.b		#BIT7, &P4OUT
;		call		#BitDelay
;		bic.b		#BIT7, &P4OUT
;		ret
NoAck:								;clears clk and sets p4 data direction back to an output
		bic.b		#BIT7, &P4OUT
		bis.b		#BIT6, &P4DIR
		ret
;-------------------------------------------------------------------------------
; End of Transmit Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Start/Stop Subroutines
;-------------------------------------------------------------------------------
StartBit:							;Generates a I2C start condition
		bic.b		#BIT6, &P4OUT
		call		#BitDelay
		bic.b		#BIT7, &P4OUT
		ret

StopBit:							;Generates a I2C stop condition
		bic.b		#BIT6, &P4OUT
		bis.b		#BIT7, &P4OUT
		bis.b		#BIT6, &P4OUT
		call		#BitDelay
		ret
;-------------------------------------------------------------------------------
; End of Start/Stop Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Delay Subroutines
;-------------------------------------------------------------------------------
SetupDelay:
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		ret

BitDelay:
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		ret
;-------------------------------------------------------------------------------
; End of Delay Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; End of Subroutines
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
