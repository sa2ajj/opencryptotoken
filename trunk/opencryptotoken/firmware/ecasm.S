#   define __SFR_OFFSET 0
#   define _VECTOR(N)   __vector_ ## N   /* io.h does not define this for asm */
#include <avr/io.h>

#include "ec.h"

	.global ec_mul
	.type   ec_mul, @function

	.global ec_on_curve
	.type   ec_on_curve, @function

	.global ec_normalize
	.type   ec_normalize, @function

	.global bin_mul
	.type 	bin_mul, @function

	.global bin_sub_2N
	.type 	bin_sub_2N, @function

	.global bin_add
	.type 	bin_add, @function

	.global bin_sub
	.type 	bin_sub, @function

	.global bin_shiftr_2N
	.type 	bin_shiftr_2N, @function

	.global bin_shiftl
	.type 	bin_shiftl, @function

	.global inv_mod
	.type 	inv_mod, @function

#define acc0 r2
#define acc1  r3
#define acc2  r4
#define zero  r15

#define acc  r5 

#define i  r16
#define j  r17

#define pal r18
#define pah r19

#define pbl r20
#define pbh r21

#define pcl r22
#define pch r23

#define ptrl  r24
#define ptrh  r25


memcpy:
	ld acc,X+
	st Y+,acc
	dec i
	brne memcpy 
	ret
/*
;r2,r3 - a
;r4,r5 - b
;r6,r7,r8,r9 - r

tmul:
	clr zero
	clr r8
	clr r9

	mul r2,r4
	mov r6,r0
	mov r7,r1

	mul r3,r4
	add r7,r0
	adc r8,r1
	adc r9,zero

	mul r2,r5
	add r7,r0
	adc r8,r1
	adc r9,zero

	mul r3,r5
	add r8,r0
	adc r9,r1
	ret
*/

psqr:
	movw YL,XL
pmul:	push ptrl
	push ptrh

	push pal
	push pah
	push pbl
	push pbh

	in ptrl,SPL
	in ptrh,SPH
	sbiw ptrl,2*N
	sbiw ptrl,N
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1

	push ZL
	push ZH

	movw ZL,ptrl
	rcall bin_mul_

	pop ZH
	pop ZL

	rjmp p192reduction

	adiw ptrl,N
	adiw ptrl,2*N-1
	out SPL,ptrl
	out SPH,ptrh
	pop ptrh
	pop ptrl
	ret

bin_sqr_:	
	movw YL,XL

bin_mul_:
;	ldi ZL,lo8(result)
;	ldi ZH,hi8(result)

	movw pal,XL
	movw pbl,YL

	clr r6
	clr r7
	clr r8
	clr r9
	clr r10

	clr j
	clr zero
nextcol:
	clr i
	movw YL,pbl

	movw XL,pal
	sec
	adc XL,j
	adc XH,zero
	sec
	adc XL,j
	adc XH,zero

nexti:
	ld r2,Y+
	ld r3,Y+
	ld r5,-X
	ld r4,-X

;	mul r0,r1

;r2,r3 - a
;r4,r5 - b
;r6,r7,r8,r9 - r

	mul r2,r4
	add r6,r0
	adc r7,r1
	adc r8,zero
	adc r9,zero
	adc r10,zero

	mul r3,r4
	add r7,r0
	adc r8,r1
	adc r9,zero
	adc r10,zero

	mul r2,r5
	add r7,r0
	adc r8,r1
	adc r9,zero
	adc r10,zero

	mul r3,r5
	add r8,r0
	adc r9,r1
	adc r10,zero

	inc	i
	cp i,j
	brlo nexti
	breq nexti

	st Z+,r6
	st Z+,r7
	movw r6,r8
	mov r8,r10
	clr r9
	clr r10

	inc j
	cpi j,N/2
	brlo nextcol


	subi pal,lo8(-N)
	sbci pah,hi8(-N)
;	adiw pal,N

nextcol2:
	movw XL,pal
	
	movw YL,pbl
	adiw YL,2
	movw pbl,YL

	ldi i,N-1
	sub i,j

nexti2:
	ld r2,Y+
	ld r3,Y+
	ld r5,-X
	ld r4,-X

;	mul r0,r1

;r2,r3 - a
;r4,r5 - b
;r6,r7,r8,r9 - r

	mul r2,r4
	add r6,r0
	adc r7,r1
	adc r8,zero
	adc r9,zero
	adc r10,zero

	mul r3,r4
	add r7,r0
	adc r8,r1
	adc r9,zero
	adc r10,zero

	mul r2,r5
	add r7,r0
	adc r8,r1
	adc r9,zero
	adc r10,zero

	mul r3,r5
	add r8,r0
	adc r9,r1
	adc r10,zero

	dec	i
	brne nexti2

	st Z+,r6
	st Z+,r7
	movw r6,r8
	mov r8,r10
	clr r9
	clr r10

	inc j
	cpi j,N-1
	brlo nextcol2

	st Z+,r6
	st Z+,r7
	ret


p192reduction:
	movw pal,ptrl
;	ldi pal,lo8(result)
;	ldi pah,hi8(result)
;	pop ZH
;	pop ZL
;rezultat mno¿enia w pal
;wynik redukcji w Z

	movw XL,pal

	movw YL,ptrl
	adiw YL,2*N ;tmp
;	ldi YL,lo8(redtmp)
;	ldi YH,hi8(redtmp)
	
	adiw XL,N
	ldi i,8
red1:
	ld acc,X+
	std Y+16,zero
	std Y+8,acc
	st Y+,acc
	dec i
	brne red1

	sbiw YL,8
	movw XL,pal
	rcall padd		;S1+S2

	movw YL,ptrl
	adiw YL,2*N
;	ldi YL,lo8(redtmp)
;	ldi YH,hi8(redtmp)

	movw XL,ptrl
;	ldi XL,lo8(result)
;	ldi YH,hi8(result)
	adiw XL,N+8
	ldi i,8
red2:
	ld acc,X+
	std Y+8,acc
	std Y+16,acc
	st Y+,zero
	dec i
	brne red2

	sbiw YL,8
	movw XL,ZL ;  Z+=S3
	rcall padd

	movw YL,ptrl
	adiw YL,2*N
;	ldi YL,lo8(redtmp)
;	ldi YH,hi8(redtmp)

	movw XL,ptrl
;	ldi XL,lo8(result)
;	ldi YH,hi8(result)
	adiw XL,N+16
	ldi i,8
red3:
	ld acc,X+
	std Y+8,acc
	std Y+16,acc
	st Y+,acc
	dec i
	brne red3

	sbiw YL,8
	movw XL,ZL
	rcall padd

	adiw ptrl,N
	adiw ptrl,2*N-1
	out SPL,ptrl
	out SPH,ptrh

	pop pbh
	pop pbl
	pop pah
	pop pal
	pop ptrh
	pop ptrl
	ret

bin_add:
	push YL
	push YH
	movw XL,r24
	movw YL,r22
	movw ZL,r20
	ldi r20,N
	clc
binadd1:
	ld r18,X+
	ld r19,Y+
	adc r18,r19
	st Z+,r18
	dec r20
	brne binadd1
	clr r24
	rol r24
	pop YH
	pop YL
	ret

bin_sub_2N:
	push YL
	push YH
	movw ZL,r20
	ldi r20,2*N
bin_sub_:
	movw XL,r24
	movw YL,r22
	clc
binsub1:
	ld r18,X+
	ld r19,Y+
	sbc r18,r19
	st Z+,r18
	dec r20
	brne binsub1
	clr r24
	rol r24
	pop YH
	pop YL
	ret

bin_sub:
	push YL
	push YH
	movw ZL,r20
	ldi r20,N
	rjmp bin_sub_

bin_shiftr_2N:
	movw XL,r24
	movw ZL,r22
	ldi r20,2*N
	clc
	adiw XL,2*N
	clc
	adiw ZL,2*N
	clc
bshr1:	
	ld r18,-X
	ror r18
	st -Z,r18
	dec r20
	brne bshr1
	clr r24
	rol r24
	ret

bin_shiftl:
	movw XL,r24
	movw ZL,r22
	ldi r20,N
	clc
bshl1:
	ld r18,X+
	rol r18
	st Z+,r18
	dec r20
	brne bshl1
	clr r24
	rol r24
	ret
	
pshiftl:
	movw YL,XL	
padd:
	push ZL
	push ZH

	ldi i,N
	clc
padd1:
	ld acc0,X+
	ld acc1,Y+
	adc acc0,acc1
	st Z+,acc0
	dec i
	brne padd1

	pop ZH
	pop ZL
	brcc padd2
	rjmp padd_sub
padd2:
	ldi YL,lo8(field_prime)
	ldi YH,hi8(field_prime)
	movw XL,ZL
	rcall bcmp
;	ldi i,N
;	clc
;padd3:
;	ld acc0,X+
;	ld acc1,Y+
;	cpc acc0,acc1
;	dec i
;	brne padd3
	brcs padd5
padd_sub:
	ldi XL,lo8(field_prime)
	ldi XH,hi8(field_prime)
	ldi i,N
	clc
padd4:
	ld acc0,Z
	ld acc1,X+
	sbc acc0,acc1
	st Z+,acc0
	dec i
	brne padd4
	sbiw ZL,N
padd5:
	ret

psub:
	ldi i,N
	clc
psub1:
	ld acc0,X+
	ld acc1,Y+
	sbc acc0,acc1
	st Z+,acc0
	dec i
	brne psub1
	brcc psub2
	sbiw ZL,N
		
	ldi XL,lo8(field_prime)
	ldi XH,hi8(field_prime)
	ldi i,N
	clc
psub4:
	ld acc0,Z
	ld acc1,X+
	adc acc0,acc1
	st Z+,acc0
	dec i
	brne psub4
psub2:
	ret

psub_:
	ldi i,N
	clc
psub1_:
	ld acc0,X
	ld acc1,Y+
	sbc acc0,acc1
	st X+,acc0
	dec i
	brne psub1_
	brcc psub2_
	sbiw XL,N
		
	ldi i,N
	clc
psub4_:
	ld acc0,X
	ld acc1,Z+
	adc acc0,acc1
	st X+,acc0
	dec i
	brne psub4_
psub2_:
	ret

; X - wskaŸnik na argument
; Z - wskaŸnik na docelo8y

#define u 0
#define v N
#define A 2*N
#define C 3*N
	
pinv:
	ldi  YL,lo8(field_prime)
	ldi  YH,hi8(field_prime)
inv_:
	push ptrl
	push ptrh
	push YH
	push YL
	in ptrl,SPL
	in ptrh,SPH

	subi ptrl,lo8(4*N)
	sbci ptrh,hi8(4*N)
;	sbiw ptrl,2*N
;	sbiw ptrl,2*N

	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1
	push ZL
	push ZH
	movw YL,ptrl
	adiw YL,u
	ldi i,N
	rcall memcpy
;	ld  XL,Y
;	ld  XH,Y+1
	ror r0
	movw YL,ptrl
	adiw YL,2*N
	adiw YL,2*N
	ld XL,Y+
	ld XH,Y+
	rol r0	
;	ldi XL,lo8(field_prime)
;	ldi XH,hi8(field_prime)
	movw YL,ptrl
	adiw YL,v
	ldi i,N
	rcall memcpy

	ldi i,N
	movw XL,ptrl
	adiw XL,A

	movw YL,ptrl
	subi YL,lo8(-C)
	sbci YH,hi8(-C)
	clr zero
	ldi i,N
pinv0:
	st X+,zero
	st Y+,zero
	dec i
	brne pinv0
	sbiw XL,N
	sbiw YL,N
	
	ldi i,1
	st X,i
;koniec inicjalizacji, g³ówna pêtelka
pinv1:
	
	movw XL,ptrl
	adiw XL,u
	ld i,X
	andi i,1
	brne pinv2
	clc
	rcall brshift
	movw XL,ptrl
	adiw XL,A
	ld i,X
	andi i,1
	brne pinv3
	clc
	rcall brshift
	rjmp pinv1
pinv3:
	ror r0
	push XL
	push XH
	movw XL,ptrl
	adiw XL,2*N
	adiw XL,2*N
	ld YL,X+
	ld YH,X+
	pop XH
	pop XL
	rol r0
;	ldi YL,lo8(field_prime)
;	ldi YH,hi8(field_prime)
	rcall badd
	ror acc
	movw XL,ptrl
	adiw XL,A
	rol acc
	rcall brshift
	rjmp pinv1
pinv2:
	movw XL,ptrl
	adiw XL,v
	ld i,X
	andi i,1
	brne pinv4
	clc
	rcall brshift
	movw XL,ptrl
	subi XL,lo8(-C)
	sbci XH,hi8(-C)
	ld i,X
	andi i,1
	brne pinv5
	clc
	rcall brshift
	rjmp pinv2
pinv5:
	ror r0
	push XL
	push XH
	movw XL,ptrl
	adiw XL,2*N
	adiw XL,2*N
	ld YL,X+
	ld YH,X+
	pop XH
	pop XL
	rol r0
;	ldi YL,lo8(field_prime)
;	ldi YH,hi8(field_prime)
	rcall badd
	ror acc
	movw XL,ptrl
	subi XL,lo8(-C)
	sbci XH,hi8(-C)
	rol acc
	rcall brshift
	rjmp pinv2
pinv4:
	movw XL,ptrl
	adiw XL,u
	movw YL,ptrl
	adiw YL,v
	rcall bcmp
skip:
	brcc pinv7 ;iu>=iv

	movw XL,ptrl
	adiw XL,v
	movw YL,ptrl
	adiw YL,u
	rcall bsub
	movw XL,ptrl
	subi XL,lo8(-C)
	sbci XH,hi8(-C)

	movw YL,ptrl
	adiw YL,2*N
	adiw YL,2*N
	ld ZL,Y+
	ld ZH,Y+


	movw YL,ptrl
	adiw YL,A

;	movw ZL,XL

	rcall psub_
pinv8:
	rjmp pinv1
pinv7:
	in acc,SREG
	push acc
	movw XL,ptrl
	adiw XL,u
	movw YL,ptrl

	adiw YL,v
	rcall bsub
	movw XL,ptrl
	adiw XL,A

	movw YL,ptrl
	adiw YL,2*N
	adiw YL,2*N
	ld ZL,Y+
	ld ZH,Y+

	movw YL,ptrl
	subi YL,lo8(-C)
	sbci YH,hi8(-C)

;	movw ZL,XL

	rcall psub_
	pop acc
	clz
	out SREG,acc
	brne pinv8

; C - wynik
	pop YH
	pop YL
	movw XL,ptrl
	subi XL,lo8(-C)
	sbci XH,hi8(-C)
	ldi i,N
	rcall memcpy

	subi ptrl,lo8(-4*N+1)
	sbci ptrh,hi8(-4*N+1)
;	adiw ptrl,2*N-1
;	adiw ptrl,2*N
	out SPL,ptrl
	out SPH,ptrh
	pop YL
	pop YH
	pop ptrh
	pop ptrl
	ret

brshift:
	push i
	in i,SREG
	adiw XL,N
	out SREG,i
	ldi i,N
rsh1:
	ld acc,-X
	ror acc
	st X,acc
	dec i
	brne rsh1
	pop i
	ret

bcmp:
	ldi i,N
	clt
	clc
bcmp1:
	ld r0,X+
	ld r1,Y+
	sez
	cpc r0,r1
	breq bcmp2
	set
bcmp2:
	dec i
	brne bcmp1
	brtc bcmp3
	clz
bcmp3:
	ret

badd:
	push i
	ldi i,N
	clc
badd1:
	ld r0,X
	ld r1,Y+
	adc r0,r1
	st X+,r0
	dec i
	brne badd1
	pop i
	ret		

bsub:
	push i
	ldi i,N
	clc
bsub1:
	ld r0,X
	ld r1,Y+
	sbc r0,r1
	st X+,r0
	dec i
	brne bsub1
	pop i
	ret		

;X: 1 punkt
;ret: Z=1 - ok

;y^2=x^3-3x+b

ec_on_curve:
	movw XL,r24

	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push YL
	push YH

_ec_on_curve:
	in ptrl,SPL
	in ptrh,SPH
	sbiw ptrl,2*N
	sbiw ptrl,N
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1
	
	movw YL,ptrl
	ldi i,2*N
	rcall memcpy

	movw XL,ptrl 
	movw YL,ptrl
	movw ZL,ptrl   
	adiw ZL,2*N		;t=x1*x1
	rcall pmul
	movw XL,ptrl	
	movw YL,ptrl
	adiw YL,2*N
	movw ZL,YL		;t=t*X1=X1^3
	rcall pmul
	movw XL,ptrl
	ldi YL,lo8(field_a)
	ldi YH,hi8(field_a)
	movw ZL,ptrl
	rcall pmul		;X1=a*X1
	movw XL,ptrl
	movw YL,ptrl
	adiw YL,2*N
	movw ZL,ptrl
	rcall padd		;X1=t+a*X1=X^3+a*X1
	movw XL,ptrl
	ldi YL,lo8(field_b)
	ldi YH,hi8(field_b)
	movw ZL,ptrl
	rcall padd

	movw XL,ptrl
	adiw XL,N
	movw YL,XL
	movw ZL,XL
	rcall pmul 		;Y1=Y1^2

	movw XL,ptrl
	movw YL,ptrl
	adiw YL,N
	rcall bcmp

	in r0,SREG					
	adiw ptrl,2*N-1
	adiw ptrl,N
	out SPL,ptrl
	out SPH,ptrh
	clr r24
	out SREG,r0
	nop
	brne not_on_curve
	inc r24	
not_on_curve:
	pop YH
	pop YL
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	clr r1 			//for C
	ret		
		
; X - punkt we wspó³rzêdnych jakobianowych
ec_is_inf:
	adiw XL,2*N
p_is_zero:
	ldi i,N
ecisinf1:
	ld acc,X+
	or acc,acc
	brne ecisinf2
	dec i
	brne ecisinf1
ecisinf2:
	ret

; X - arg

p_zero:
	ldi i,N
	clr zero
pzero1:
	st X+,zero
	dec i
	brne pzero1
	ret

; X - src
; Z - dst

ec_point_copy:
	movw YL,XL
	subi YL,lo8(-3*N)
	sbci YH,hi8(-3*N)

eccp:
	ld acc,X+
	st Z+,acc
	cp XL,YL
	brne eccp	
	cp XH,YH
	brne eccp
	ret

; X - arg1
; Y - arg2
; Z - rezultat
	

ec_add:
	push ptrl
	push ptrh
	push pal
	push pah
	push pbl
	push pbh
	push pcl
	push pch

	in ptrl,SPL
	in ptrh,SPH
	subi ptrl,lo8(7*N)
	sbci ptrh,hi8(7*N)
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1

	movw pal,XL
	movw pbl,YL
	movw pcl,ZL

	rcall ec_is_inf
	brne ecadd1

	movw XL,pbl
	movw ZL,pcl
	rcall ec_point_copy
	rjmp ecaddex
		
ecadd1:
	movw XL,pbl
	rcall ec_is_inf
	brne ecadd2
	movw XL,pal
	movw ZL,pcl
	rcall ec_point_copy
	rjmp ecaddex
ecadd2:

	movw XL,pbl
	adiw XL,2*N
	movw ZL,ptrl
	rcall psqr		;n0=Zb^2

	movw XL,ptrl
	movw YL,pal
	movw ZL,ptrl
	adiw ZL,N
	rcall pmul		;n1=n0*Xa

	movw XL,pbl
	adiw XL,2*N
	movw YL,ptrl
	movw ZL,ptrl
	rcall pmul		;n0=n0*Zb
	movw XL,ptrl
	movw YL,pal
	adiw YL,N
	movw ZL,ptrl
	adiw ZL,2*N
	rcall pmul		;n2=n0*Ya=Ya*Zb^3

	
	movw XL,pal
	adiw XL,2*N
	movw ZL,ptrl
	rcall psqr		;n0=Za^2

	movw XL,ptrl
	movw YL,pbl

	movw ZL,ptrl
	subi ZL,lo8(-3*N)
	sbci ZH,hi8(-3*N)
	rcall pmul		;n3=n0*Xb


	movw XL,pal
	adiw XL,2*N
	movw YL,ptrl
	movw ZL,ptrl
	rcall pmul		;n0=n0*Za

	movw XL,ptrl
	movw YL,pbl
	adiw YL,N
	movw ZL,ptrl
	subi ZL,lo8(-4*N)
	sbci ZH,hi8(-4*N)
	rcall pmul		;n2=n0*Yb=Yb*Za^3

	movw XL,ptrl
	adiw XL,N

	movw YL,ptrl
	subi YL,lo8(-3*N)
	sbci YH,hi8(-3*N)

	movw ZL,ptrl
	subi ZL,lo8(-5*N)
	sbci ZH,hi8(-5*N)
	rcall psub		;n5=n1-n3

	movw XL,ptrl
	adiw XL,2*N

	movw YL,ptrl
	subi YL,lo8(-4*N)
	sbci YH,hi8(-4*N)

	movw ZL,ptrl
	subi ZL,lo8(-6*N)
	sbci ZH,hi8(-6*N)
	rcall psub		;n6=n2-n4

	movw XL,ptrl
	subi XL,lo8(-5*N)
	sbci XH,hi8(-5*N)
	rcall p_is_zero
	brne ecadd3

	movw XL,ptrl
	subi XL,lo8(-6*N)
	sbci XH,hi8(-6*N)
	rcall p_is_zero
	brne ecadd4
;a is the same as b

	movw XL,pal
	movw ZL,pcl
	rcall ec_dbl
	rjmp ecaddex	

ecadd4:	
;a is inverse of b
	movw ZL,pcl
	adiw ZL,2*N
	rcall p_zero
	rjmp ecaddex

ecadd3:
	movw XL,ptrl
	adiw XL,N
	movw YL,ptrl
	subi YL,lo8(-3*N)	
	sbci YH,hi8(-3*N)
	movw ZL,XL
	rcall padd		;n1-=n3

	movw XL,ptrl
	adiw XL,2*N
	movw YL,ptrl
	subi YL,lo8(-4*N)	
	sbci YH,hi8(-4*N)
	movw ZL,XL
	rcall padd		;n2-=n4


	movw XL,pal
	adiw XL,2*N
	movw YL,pbl
	adiw YL,2*N
	movw ZL,ptrl
	rcall pmul		;n0=Za*Zb

	movw XL,ptrl
	subi XL,lo8(-5*N)
	sbci XH,hi8(-5*N)
	movw YL,ptrl
	movw ZL,pcl
	adiw ZL,2*N	
	rcall pmul		;Zc=n5*Za*Zb

	movw XL,ptrl
	subi XL,lo8(-6*N)
	sbci XH,hi8(-6*N)
	movw ZL,ptrl
	rcall psqr		;n0=n6^2
	
	movw XL,ptrl
	subi XL,lo8(-5*N)
	sbci XH,hi8(-5*N)
	movw ZL,ptrl
	subi ZL,lo8(-4*N)
	sbci ZH,hi8(-4*N)
	rcall psqr		;n4=n5^2
	
	movw XL,ptrl
	adiw XL,N
	movw YL,ptrl
	subi YL,lo8(-4*N)
	sbci YH,hi8(-4*N)
	movw ZL,ptrl
	subi ZL,lo8(-3*N)
	sbci ZH,hi8(-3*N)
	rcall pmul		;n3=n1*n4

	movw XL,ptrl
	movw YL,ptrl
	subi YL,lo8(-3*N)
	sbci YH,hi8(-3*N)
	movw ZL,pcl
	rcall psub

	movw XL,pcl
	movw ZL,ptrl
	rcall pshiftl

	movw XL,ptrl
	subi XL,lo8(-3*N)
	sbci XH,hi8(-3*N)
	movw YL,ptrl
	movw ZL,ptrl
	rcall psub		

	movw XL,ptrl
	movw YL,ptrl
	subi YL,lo8(-6*N)
	sbci YH,hi8(-6*N)
	movw ZL,XL	
	rcall pmul

	movw XL,ptrl
	subi XL,lo8(-4*N)
	sbci XH,hi8(-4*N)
	movw YL,ptrl
	subi YL,lo8(-5*N)
	sbci YH,hi8(-5*N)
	movw ZL,YL
	rcall pmul

	movw XL,ptrl
	adiw XL,2*N
	movw YL,ptrl
	subi YL,lo8(-5*N)
	sbci YH,hi8(-5*N)
	movw ZL,ptrl
	adiw ZL,N
	rcall pmul		;n1=n2*n5

	movw XL,ptrl
	movw YL,ptrl
	adiw YL,N
	movw ZL,pcl
	adiw ZL,N
	rcall psub

	movw XL,pcl
	adiw XL,N
	ld i,X
	andi i,1
	clc
	breq ecadd5
	
	ldi YL,lo8(field_prime)
	ldi YH,hi8(field_prime)
	rcall badd

ecadd5:
	in i,SREG
	movw XL,pcl
	adiw XL,N
	out SREG,i
	rcall brshift

ecaddex:
	subi ptrl,lo8(-7*N+1)
	sbci ptrh,hi8(-7*N+1)
	out SPL,ptrl
	out SPH,ptrh

	pop pch
	pop pcl
	pop pbh
	pop pbl
	pop pah
	pop pal
	pop ptrh
	pop ptrl

	ret
; X - argument
; Z - resultat

ec_dbl:
	push ptrl
	push ptrh
	push pal
	push pah
	push pbl
	push pbh

	in ptrl,SPL
	in ptrh,SPH
	sbiw ptrl,2*N
	sbiw ptrl,2*N
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1

	movw pal,XL
	movw pbl,ZL

	rcall ec_is_inf
	brne ecdbl1
	
	movw XL,pal
	adiw XL,2*N
	movw YL,pbl
	adiw YL,2*N
	rcall memcpy	;Zr=Za=0

	rjmp ecdbl2

ecdbl1:
	movw XL,pal
	adiw XL,2*N
	movw ZL,ptrl
	rcall psqr		;n0=Za^2

	movw XL,pal
	movw YL,ptrl
	movw ZL,ptrl
	adiw ZL,N
	rcall padd		;n1=Xa+n0

	movw XL,pal
	movw YL,ptrl
	movw ZL,ptrl
	adiw ZL,2*N	;n2=Xa-n0
	rcall psub
	
	movw XL,ptrl
	adiw XL,N
	movw YL,ptrl
	adiw YL,2*N
	movw ZL,ptrl
	rcall pmul		;n0=n1*n2
	
	movw XL,ptrl
	movw ZL,ptrl
	adiw ZL,N
	rcall pshiftl	;n1=2n0

	movw XL,ptrl
	movw YL,ptrl
	adiw YL,N
	movw ZL,YL
	rcall padd		;n1=n1+n0

	movw XL,pal
	adiw XL,N
	movw YL,pal
	adiw YL,2*N
	movw ZL,ptrl
	rcall pmul		;n0=Ya*Za
	movw XL,ptrl
	movw ZL,pbl
	adiw ZL,2*N	
	rcall pshiftl	;Zr=2*n0

	movw XL,pal
	adiw XL,N
	movw ZL,ptrl
	rcall pshiftl	;n0=Ya*2
	movw XL,ptrl
	movw ZL,ptrl
	rcall psqr		;n0=n0^2 = 4Ya^2
	movw XL,pal
	movw YL,ptrl
	movw ZL,ptrl
	adiw ZL,2*N		;n2=n0*Xa = 4XaYa^2
	rcall pmul

	movw XL,ptrl
	adiw XL,N		;n1
	movw ZL,ptrl
	rcall psqr		;n0=n1^2
	movw XL,ptrl
	movw YL,ptrl
	adiw YL,2*N
	movw ZL,ptrl
	rcall psub		;n0=n0-n2=n1^2-n2

	movw XL,ptrl
	movw YL,ptrl
	adiw YL,2*N
	movw ZL,pbl
	rcall psub		;Xr=n0-n2=n1^2-2*n2

	movw XL,pal
	adiw XL,N		
	movw ZL,ptrl	
	rcall psqr		;n0=Ya^2
	movw XL,ptrl
	movw ZL,ptrl
	rcall pshiftl	;n0*=2=2Ya^2
	movw XL,ptrl
	movw ZL,ptrl
	rcall psqr		;n0=n0^2=4Ya^4
	movw XL,ptrl
	movw ZL,ptrl
	adiw ZL,N
	adiw ZL,2*N		;n3
	rcall pshiftl		
	
	movw XL,ptrl
	adiw XL,2*N
	movw YL,pbl
	movw ZL,ptrl
	rcall psub		;n0=n2-Xr
	movw XL,ptrl
	movw YL,ptrl
	adiw YL,N
	movw ZL,ptrl
	rcall pmul		;n0=n0*n1
	
	movw XL,ptrl
	movw YL,ptrl
	adiw YL,2*N
	adiw YL,N
	movw ZL,pbl
	adiw ZL,N
	rcall psub		;Yr=n0-n3		

ecdbl2:
	adiw ptrl,2*N-1
	adiw ptrl,2*N
	out SPL,ptrl
	out SPH,ptrh
	pop pbh
	pop pbl
	pop pah
	pop pal
	pop ptrh
	pop ptrl
	ret		

ec_normalize:

	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push YL
	push YH

	movw XL,r24

	in ptrl,SPL
	in ptrh,SPH
	sbiw ptrl,2*N
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1
	
	movw pal,XL

	movw XL,pal
	adiw XL,2*N		;z
	movw ZL,ptrl
	rcall pinv		;n0=z_1

	movw XL,ptrl
	movw ZL,ptrl
	adiw ZL,N
	rcall psqr		;n1=z_2

	movw XL,pal
	movw YL,ptrl
	adiw YL,N
	movw ZL,pal
	rcall pmul		;X=n1*X

	movw XL,ptrl
	adiw XL,N
	movw YL,ptrl
	movw ZL,ptrl
	rcall pmul		;n0=n1*n0=z_3

	movw XL,ptrl
	movw YL,pal
	adiw YL,N
	movw ZL,YL
	rcall pmul		;Y=n0*Y

	movw XL,pal
	adiw XL,2*N
	ldi i,1
	st X+,i
	clr zero
	ldi i,N-1
nrm:
	st X+,zero
	dec i
	brne nrm

	adiw ptrl,2*N-1
	out SPL,ptrl
	out SPH,ptrh

	pop YH
	pop YL
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	clr r1

	ret

ec_mul:
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push YL
	push YH

	movw XL,r24
	movw YL,r22
	movw ZL,r20


;X - punkt (wsp. Jakobinowe)
;Y - k
;Z - resultat

_ec_mul:

	in ptrl,SPL
	in ptrh,SPH
	subi ptrl,lo8(4*N)
	sbci ptrh,hi8(4*N)
	out SPL,ptrl
	out SPH,ptrh
	adiw ptrl,1


	movw pal,XL
	movw pbl,YL
	movw pcl,ZL
	
	movw XL,pbl
	movw YL,ptrl
	ldi i,N
	rcall memcpy		;n0=k

	movw XL,pal
	movw ZL,ptrl
	adiw ZL,N			;n1=P
	rcall ec_point_copy

	movw XL,pcl
	adiw XL,2*N
	rcall p_zero		;result=inf

ecmul1:

	push ptrl
	push ptrh
	push pcl
	push pch
	clr r1
	wdr
	rcall usbPoll
	pop pch
	pop pcl
	pop ptrh
	pop ptrl

	movw XL,ptrl
	rcall p_is_zero	
	breq ecmulex

	movw XL,ptrl
	ld i,X
	andi i,1
	breq ecmulskip

	movw XL,ptrl
	adiw XL,N
	movw YL,pcl
	movw ZL,pcl
	rcall ec_add

ecmulskip:
	movw XL,ptrl
	adiw XL,N
	movw ZL,XL
	rcall ec_dbl
			
	movw XL,ptrl
	clc
	rcall brshift
	rjmp ecmul1

ecmulex:

	subi ptrl,lo8(-4*N+1)
	sbci ptrh,hi8(-4*N+1)
	out SPL,ptrl
	out SPH,ptrh

popregs:
	pop YH
	pop YL

	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2

	clr r1
	ret

bin_mul:
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push YL
	push YH

	movw XL,r24
	movw YL,r22
	movw ZL,r20
	rcall bin_mul_
	rjmp popregs

inv_mod:
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push YL
	push YH

	movw XL,r24
	movw ZL,r22
	movw YL,r20
	rcall inv_
	rjmp popregs

