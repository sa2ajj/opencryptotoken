	.global usbAppInit
	.global usbAppIdle
	.global usbAppFunctionSetup
	.global usbAppFunctionWrite

	.section .appvect,"ax",@progbits
usbAppPreInit_:
	ret
usbAppInit_:
	ret
usbAppIdle_:
	ret
usbAppFunctionSetup_:
	ret
usbAppFunctionWrite_:
	ret
