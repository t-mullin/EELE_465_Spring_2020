;Tristan Mullin, Ali Almutrafi and Connor Overcast with assistance from Shane Gill and Kevin Williamson
;10/28/16
;Lab 8 read and write various VALUEs held within the ds3231
		INCLUDE 'derivative.inc'
		XDEF _Startup, main
		XREF __SEG_END_SSTACK
;Assembler Equates
RAMSPACE EQU $0060 ;RAM start address
ROMSPACE EQU $E000 ;EPROM start address


; Emulated I2C lines on Port A pins
; Need a clock (SCL) and data (SDA)
SCL EQU 7 ;Serial clock
SDA EQU 6 ;Serial data
SlAddr EQU $68 ;Slave address of DAC

; RAM Variables
			ORG RAMSPACE

BitCounter RMB 1 ;Used to count bits in a Tx
VALUE RMB 1 ;Used to store data VALUE
Direction RMB 1 ;Indicates increment or decrement
temp DS.B 1 
			ORG ROMSPACE 
main:
_Startup:

			;LDA #%01010010 ;putting the watchdog down :(
			;STA SOPT1

;Initialize variables
			CLR VALUE ;Clear all RAM variables
			CLR BitCounter
			CLR Direction
;Setup parallel ports

	BSET 6, PTBDD	;SDA
	BSET 7, PTBDD	;SCL


; Send the I2C transmission, including START, address,
; data, and STOP

			;Data write
			JSR StartBit
			
			LDA #SlAddr ;Slave device address
			CLC ;write to slave
			ROLA
			JSR TxByte ;Send the eight bits

			LDA #$00
			STA PTBD
			JSR TxByte
			
			;15 secs
			LDA #$45
			JSR TxByte
			
			;30 minutes
			LDA #$59
			JSR TxByte
			
			;09 AM
			LDA #$09
			JSR TxByte
			
			;stop condition
			JSR StopBit 		;Give STOP condition
			JSR BitDelay 	;Wait a bit
			
			

mainLoop:

		feed_watchdog ;he'll get hungry if you dont
		
		
;START condition to set pointer
			JSR StartBit 	;Give START condition

;data read seconds --write pointer, then read

;ADDRESS byte, consists of 7-bit address + 0 as LSbit

			LDA #SlAddr 		;Slave device address
			CLC 				;write to slave
			ROLA
			JSR TxByte 			;Send the eight bits


			LDA #$00
			STA PTBD
			JSR TxByte

			BSET SDA,PTBD
			BSET SCL,PTBD


;repeated start to begin reading
			JSR StartBit
			
			
;ADDRESS byte, consists of 7-bit address + 0 as LSbit
			LDA #SlAddr 		;Slave device address
			SEC 				;read from slave (set carry to 1)
			ROLA 				;Need this to align address
			JSR TxByte 			;Send the eight bits

			JSR RxByte	;seconds
			JSR AckMe
			
			
			JSR RxByte	;minutes
			JSR AckMe
			
			
			JSR RxByte	;hours
			JSR NackMe

;stop condition
			JSR StopBit 		;Give STOP condition
			JSR BitDelay 		;Wait a bit
			BRA mainLoop 		;Repeat

RxByte:
;Initialize variable

			BCLR SDA,PTBDD
			LDX #$08
			STX BitCounter

RxNextBit:

			ROLA 			;Shift MSbit into Carry
			BCC RxSendLow 	;Send low bit or high bit
RxSendHigh:
			BSET SDA,PTBD 	;Set the data bit VALUE
			JSR SetupDelay 	;Give some time for data setup
			BSET SCL,PTBD 	;Clock it in
			JSR BitDelay 	;Wait a bit
			BRA RxCont 		;Continue
RxSendLow:
			BCLR SDA,PTBD
			JSR SetupDelay
			BSET SCL,PTBD
			JSR BitDelay
RxCont:
			BCLR SCL,PTBD 	;Restore clock to low state
			DEC BitCounter 	;Decrement the bit counter
			BNE RxNextBit
			RTS
			
			
AckMe:			
			;send an acknowledge unless last bit
			BSET SDA,PTBDD
			BCLR SDA,PTBD
			JSR SetupDelay
			BSET SCL,PTBD ;Clock the line to get ACK
			JSR BitDelay
			BCLR SCL,PTBD ;Restore clock line
			BSET SDA,PTBDD ;SDA back as output
			RTS

NackMe:
			;send nack after last byte of data
			BSET SDA,PTBDD
			BSET SDA,PTBD
			JSR SetupDelay
			BSET SCL,PTBD ;Clock the line to get ACK
			JSR BitDelay
			BCLR SCL,PTBD ;Restore clock line
			BSET SDA,PTBDD ;SDA back as output
			RTS

; Transmit the byte in Acc to the SDA pin
; (Acc will not be restored on return)
; Must be careful to change SDA VALUEs only while SCL is low,
; otherwise a STOP or START could be implied 


TxByte:
;Initialize variable
			LDX #$08
			STX BitCounter

NextBit:
			ROLA 		;Shift MSbit into Carry
			BCC SendLow ;Send low bit or high bit
SendHigh:
			BSET SDA,PTBD 	;Set the data bit VALUE
			JSR SetupDelay 	;Give some time for data setup
			BSET SCL,PTBD 	;Clock it in
			JSR BitDelay 	;Wait a bit
			BRA TxCont 		;Continue
SendLow:
			BCLR SDA,PTBD
			JSR SetupDelay
			BSET SCL,PTBD
			JSR BitDelay
TxCont:
			BCLR SCL,PTBD 	;Restore clock to low state
			DEC BitCounter 	;Decrement the bit counter
			BEQ AckPoll 	;Last bit?
			BRA NextBit
AckPoll:
			BSET SDA,PTBD
			BCLR SDA,PTBD
			BCLR SDA,PTBDD 	;Set SDA as input
			JSR SetupDelay	;wait a beat
			BSET SCL,PTBD 	;Clock the line to get ACK
			JSR BitDelay	;wait a beat
			BRSET SDA,PTBD,NoAck ;Look for ACK from slave device 
			BCLR SCL,PTBD ;Restore clock line
			BSET SDA,PTBDD ;SDA back as output
			RTS

;No acknowledgment received from slave device
;Some error action can be performed here
;For now, just restore the bus
NoAck:
			BCLR SCL,PTBD
			BSET SDA,PTBDD
			RTS

; Start Condition
StartBit:
			BCLR SDA,PTBD
			JSR BitDelay
			BCLR SCL,PTBD
			RTS

; Stop Condition
StopBit:
			BCLR SDA,PTBD
			BSET SCL,PTBD
			BSET SDA,PTBD
			JSR BitDelay
			RTS

; Provide some data setup time to allow
; SDA to stabilize in slave device
; Completely arbitrary delay (10 cycles)
SetupDelay:
			NOP
			NOP
			RTS

; Bit delay to provide (approximately) the desired
; SCL frequency
; Again, this is arbitrary (16 cycles)
BitDelay:
			NOP
			NOP
			NOP
			NOP
			NOP
			RTS
