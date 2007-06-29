	.global usbAppIdle
	.global usbAppFunctionWrite
	.global usbAppFunctionSetup
	.global __vector_default
	.section .appvect,"ax",@progbits

__vector_default: 	/*needed by linker*/
	rjmp __init
	rjmp usbAppInit
	rjmp usbAppIdle
	rjmp usbAppFunctionSetup
	rjmp usbAppFunctionWrite

/*ret before jmp to main */
	.section .init8,"ax",@progbits
	ret

	.section .init1,"ax",@progbits
	eor r1,r1

