typedef void (*usbAppPreInit_t)();
typedef void (*usbAppInit_t)(void (*ptr)());
typedef void (*usbAppIdle_t)();
typedef uchar (*usbAppFunctionSetup_t)(uchar[8],uchar **);
typedef uchar (*usbAppFunctionWrite_t)(uchar *,uchar);

/*

#define usbAppPreInit() ({		\
    __asm__ __volatile__                    \
    (                                       \
        "ldi r30,0\n\t"                     \
        "eor r31,r31\n\t"                   \
        "icall\n\t"                         \
        :	\
    );                                	\
})
*/

void usbAppPreInit();
void usbAppInit(void (*)());
void usbAppIdle();
uchar usbAppFunctionSetup(uchar[8],uchar **);
uchar usbAppFunctionWrite(uchar *,uchar);