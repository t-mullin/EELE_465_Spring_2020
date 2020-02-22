;Lizzy Hamaoka & Tristan Mullin
;1/23/2020
;Lab 0: Heatbeat LED
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
;-------------------------------------------------------------------------------
; Part 1 - Fastloop
;-------------------------------------------------------------------------------
;init:
;		bic.w	#0001h, &PM5CTL0	;Disable Digital I/O low-power default
;		bis.b	#BIT0, &P1DIR		;Sets P1 bit 0 as an output
;main:
;		xor.b	#BIT0, &P1OUT		;xors bit 0 of P1 to toggle LED1
;		mov.w	#0FFFFh, R4			;Sets R4 to a large value for the delay loop
;		mov.w	#5, R5				;Sets R5 to a small value for the delay1 loop
;delay:
;		dec.w	R4					;Decrements the value in R4
;		jnz 	delay				;Jump back to delay until Zero bit is reset
;		jz		delay1				;Jump to delay1 if Zero bit is set
;delay1:
;		dec		R5					;Decrements the value in R5
;		jz		main				;Jump back to main if the Zero bit is set
;		jnz		delay				;Jump back to delay if Zero bit is reset
;		NOP

;-------------------------------------------------------------------------------
; Part 2 - Prescaler
;-------------------------------------------------------------------------------
;init:
;		bic.w	#0001h, &PM5CTL0	;Disable Digital I/O low-power default
;		bis.b	#BIT0, &P1DIR		;Sets P1 bit 0 as an output
;		bic.b	#BIT0, &P1OUT		;Clears bit 0 of P1OUT

		;--Setup timer B0
;		bis.w	#TBCLR, &TB0CTL				;Clears the timer
;		bis.w	#TBSSEL__ACLK, &TB0CTL		;Sets the clock to ACLK (32.768 kHz)
;		bis.w	#MC__CONTINUOUS, &TB0CTL	;Set the clock to continuous mode
;		bis.w	#CNTL_1, &TB0CTL			;Sets the counter length (12 bit)
;		bis.w	#ID__8, &TB0CTL				;Sets the clock divider (Divide by 8)
;		bis.w	#TBIE, &TB0CTL				;Enables interupts
;		bic.w	#TBIFG, &TB0CTL				;Clears the interupt flag
;		NOP
;		bis.w	#GIE, SR					;Enables maskable interupts
;		NOP
;main:
;		jmp main		;Jump back to main
;-------------------------------------------------------------------------------
; Interrupt Service Routine
;-------------------------------------------------------------------------------
;ISR_TB0_Overflow:
;			bic.w	#TBIFG, &TB0CTL		;Clears the Timer_B interupt flag
;			xor.b	#BIT0, &P1OUT		;xors bit 0 of P1 to toggle LED1
;			reti						;Returns from interupt

;-------------------------------------------------------------------------------
; Part 3 - Interupt
;-------------------------------------------------------------------------------
init:
		bic.w	#0001h, &PM5CTL0	;Disable Digital I/O low-power default
		bis.b	#BIT0, &P1DIR		;Sets P1 bit 0 as an output
		bic.b	#BIT0, &P1OUT		;Clears bit 0 of P1OUT

		;--Setup timer B0
		bis.w	#TBCLR, &TB0CTL				;Clears the timer
		bis.w	#TBSSEL__ACLK, &TB0CTL		;Sets the clock to ACLK (32.768 kHz)
		bis.w	#MC__CONTINUOUS, &TB0CTL	;Set the clock to continuous mode
		bis.w	#CNTL_1, &TB0CTL			;Sets the counter length (12 bit)
		bis.w	#ID__1, &TB0CTL				;Sets the clock divider (Divide by 1)
		bis.w	#TBIE, &TB0CTL				;Enables interupts
		bic.w	#TBIFG, &TB0CTL				;Clears the interupt flag
		NOP
		bis.w	#GIE, SR					;Enables maskable interupts
		NOP

		mov.w	#9, R4		;Sets R4 to a small value to create a delay
main:
		jmp 	main		;Jumps back to main
LED_toggle:
		xor.b	#BIT0, &P1OUT	;xors bit 0 of P1 to toggle LED1
		mov.w 	#9, R4			;Sets R4 to a small value to create a delay

;-------------------------------------------------------------------------------
; Interrupt Service Routine
;-------------------------------------------------------------------------------
ISR_TB0_Overflow:
			bic.w	#TBIFG, &TB0CTL		;Clears the Timer_B interupt flag
			dec		R4					;Decrements the value in R4
			cmp		#0, R4				;Compares the 0 to the value in R4
			jz		LED_toggle			;Jump to LED_toggle if the Zero bit is set
			reti						;Returns from interupt

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
            
            .sect 	".int42"
            .short	ISR_TB0_Overflow
