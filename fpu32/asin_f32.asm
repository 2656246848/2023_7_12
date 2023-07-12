; asin_f32.asm - assembly implementation of arcsin on FPU32 with TMU
;
; Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
; ALL RIGHTS RESERVED 

;-----------------------------------------------------------------------------

; C prototype: extern float asinf(float theta)
; argument 1 = theta : [R0H] input [-1 : +1]
; return = [R0H] angle in radians [-pi : +pi]

		.if __TI_EABI__
		.asg	asinf, _asin
		.endif
		.global _asin

	   .if (__TI_EABI__ = 0)
       ; For TI COFF double and float are the same size, so atanf() and atan()
       ; should be the same function
		.global _asinf

		.align	2
_asinf:
	   .endif
_asin:
		.asmfunc
		.if (.TMS320C2800_TMU0 = 1)
		MOV32   		*SP++, R7H				; push R7H
		ABSF32			R1H, R0H				; phi = |theta|
		MPYF32			R2H, R0H, R0H			; R2H = phi^2
		MOVIZF32		R7H, #1.0				; R7H = 1.0f
		SUBF32			R2H, R7H, R2H			; R2H = 1 - phi^2
		NOP
		CMPF32			R1H, R7H				; test for (|theta| > 1)
		MOVST0			NF, ZF					; copy flags
		SB				asin_nan, GT			; NaN
		SQRTF32			R2H, R2H				; R2H = sqrt(1 = phi^2)
		CMPF32			R0H, #0.0				; test sgn(theta)
		MOV32			*SP++, STF				; push flags
		NOP
		NOP
		DIVF32			R2H, R1H, R2H			; R2H = asin_v = phi / sqrt(1 = phi^2)
		NOP
		NOP
		NOP
		NOP
		DIVF32			R3H, R7H, R2H			; R3H = 1 / asin_v
		MOVF32			R1H, #1.570796327		; R1H = pi / 2
		NOP
		NOP
		NOP
		ATANPUF32		R0H, R2H				; R0H = atan(asin_v)
		ATANPUF32		R3H, R3H				; R3H = atan(1 / asin_v)
		NOP
		NOP
		NOP
		MPY2PIF32		R0H, R0H				; R0H = 2*pi*atan(asin_v)
		MPY2PIF32		R3H, R3H				; R3H = 2*pi*atan(1 / asin_v)
		NOP
		NOP
		SUBF32			R3H, R1H, R3H			; R3H = (pi/2) - 2*pi*atan(1 / asin_v)
		CMPF32			R2H, R7H				; set flags on asin_v
		MOV32			R0H, R3H, GT			; select asin
		MOV32			STF, *--SP				; pop flags
		NEGF32			R0H, R0H, LT			; set sign
		MOV32   		R7H, *--SP, UNC			; pop R7H
		LRETR
asin_nan:
		MOVIZ			R0H, #0x7FC0			; R0H = NaN
		MOV32   		R7H, *--SP, UNC			; pop R7H
		LRETR
		.else
		.wmsg			"WARNING - TMU0 NOT PRESENT: _asinf NOT BUILT"
		LRETR
		.endif
		.endasmfunc

;-----------------------------------------------------------------------------

		.end

; end of file
