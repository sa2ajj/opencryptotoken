	.global usbAppPreInit
	.global usbAppInit
	.global usbAppIdle
	.global usbAppFunctionSetup
	.global	usbAppFunctionWrite

	.global state

usbAppPreInit:
	eor r30,r30
vcall:	
	eor r31,r31
	lds r27,state
	and r27,r27
	brne vcall_ex
	ijmp
vcall_ex:
	ret
usbAppInit:
	ldi r30,1
	rjmp vcall
usbAppIdle:
	ldi r30,2
	rjmp vcall
usbAppFunctionSetup:
	ldi r30,3
	rjmp vcall
usbAppFunctionWrite:
	ldi r30,4
	rjmp vcall
