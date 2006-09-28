/* Name: ectl.c
 * Project: OpenCryptoToken based on powerSwitch
 * Author: mrk
 * Creation Date: 2006-09-16
 * Copyright: (c) 2006 by ITS
 */

/*
General Description:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */
#include "../firmware/ec.h"

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */

#define PSCMD_ECHO  0
#define PSCMD_GET   1
#define PSCMD_ON    2
#define PSCMD_OFF   3
/* These are the vendor specific SETUP commands implemented by our USB device */

static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s test\n", name);
    fprintf(stderr, "  %s mult n\n", name);
    fprintf(stderr, "  %s sign n\n", name);
    fprintf(stderr, "  %s rnd\n", name);
}


static int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING)
        return 0;
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for(i=1;i<rval;i++){
        if(i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}


/* AvrCryptoToken uses the free shared default VID/PID. If you want to see an
 * example device lookup where an individually reserved PID is used, see our
 * RemoteSensor reference implementation.
 */

#define USB_ERROR_NOTFOUND  1
#define USB_ERROR_ACCESS    2
#define USB_ERROR_IO        3

static int usbOpenDevice(usb_dev_handle **device, int vendor, char *vendorName, int product, char *productName)
{
struct usb_bus      *bus;
struct usb_device   *dev;
usb_dev_handle      *handle = NULL;
int                 errorCode = USB_ERROR_NOTFOUND;
static int          didUsbInit = 0;

    if(!didUsbInit){
        didUsbInit = 1;
        usb_init();
    }
    usb_find_busses();
    usb_find_devices();
    for(bus=usb_get_busses(); bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product){
                char    string[256];
                int     len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    errorCode = USB_ERROR_ACCESS;
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }
                if(vendorName == NULL && productName == NULL){  /* name does not matter */
                    break;
                }
                /* now check whether the names match: */
                len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
                if(len < 0){
                    errorCode = USB_ERROR_IO;
                    fprintf(stderr, "Warning: cannot query manufacturer for device: %s\n", usb_strerror());
                }else{
                    errorCode = USB_ERROR_NOTFOUND;
                    /* fprintf(stderr, "seen device from vendor ->%s<-\n", string); */
                    if(strcmp(string, vendorName) == 0){
                        len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                        if(len < 0){
                            errorCode = USB_ERROR_IO;
                            fprintf(stderr, "Warning: cannot query product for device: %s\n", usb_strerror());
                        }else{
                            errorCode = USB_ERROR_NOTFOUND;
                            /* fprintf(stderr, "seen product ->%s<-\n", string); */
                            if(strcmp(string, productName) == 0)
                                break;
                        }
                    }
                }
                usb_close(handle);
                handle = NULL;
            }
        }
        if(handle)
            break;
    }
    if(handle != NULL){
        errorCode = 0;
        *device = handle;
    }
    return errorCode;
}

void hex_print(FILE *f,unsigned char *buf, int len) {
  int i;
   for(i=0; i<len; i++) {
     fprintf(f,"%02x",buf[len-i-1]);
   }
   fflush(f); 
}

int hex2bin(char *arg,unsigned char *buf) {
  int n=0;
  char *p=arg;
  p+=strlen(p)-1;
  
  while(p>=arg) {
    if(*p>='A' && *p<='F') *p+='a'-'A';
    int b=((*p>='0' && *p<='9') ? (*p-'0') : (*p>='a' && *p<='f') ? *p-'a'+10 : 0)<<((n&1)*4);
    buf[n++/2]|=b;
    p--;
  }
  return (n+1)/2;
}

int main(int argc, char **argv)
{
usb_dev_handle      *handle = NULL;
unsigned char       buffer[8];
int                 nBytes;

    if(argc < 2){
        usage(argv[0]);
        exit(1);
    }
    usb_init();
    if(usbOpenDevice(&handle, USBDEV_SHARED_VENDOR, "its.sed.pl", USBDEV_SHARED_PRODUCT, "AvrCryptoToken") != 0){
        fprintf(stderr, "Could not find USB device \"AvrCryptoToken\" with vid=0x%x pid=0x%x\n", USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT);
        exit(1);
    }
/* We have searched all devices on all busses for our USB device above. Now
 * try to open it and perform the vendor specific control operations for the
 * function requested by the user.
 */
    if(strcmp(argv[1], "mult") == 0){
        unsigned char buf[256];
        memset(buf,0,256);
        int n=hex2bin(argv[2],buf);
        
        int nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE, AVRCMD_MULT, 0, 0, (char *)buf, n, 5000);
        n=0;
        while((nBytes=usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, AVRCMD_GETRESULT, 0, 0, (char *)buf, 256, 5000))==0 && n++<25)
          usleep(200000);
        if(nBytes==3*N) {
          fprintf(stdout,"X:"); hex_print(stdout,buf,N);
          fprintf(stdout,"\nY:"); hex_print(stdout,buf+N,N);
          fprintf(stdout,"\nZ:"); hex_print(stdout,buf+2*N,N);
          fprintf(stdout,"\n");
        } else {
          fprintf(stderr,"mult timeout\n");
        }
    } else if(strcmp(argv[1], "sign") == 0) {
        nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE, AVRCMD_SIGN, 0, 0, argv[2], strlen(argv[2]), 5000);
        int n=0;
        unsigned char buf[256];
        memset(buf,0,256);
        ecdsa_sig_t sig;
        while((nBytes=usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, AVRCMD_GETRESULT, 0, 0, (char *)&sig, sizeof(ecdsa_sig_t), 5000))==0 && n++<25)
          usleep(200000);
        if(nBytes==sizeof(ecdsa_sig_t)) {
          fprintf(stdout,"R:"); hex_print(stdout,sig.R.value,N);
          fprintf(stdout,"\nS:"); hex_print(stdout,sig.S.value,N);
          fprintf(stdout,"\n");
        } else {
          fprintf(stderr,"sign timeout\n");
        }
    } else if(strcmp(argv[1], "rnd") == 0){
      unsigned char buf[32];
      nBytes=usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, AVRCMD_GETRND, 0, 0, (char *)buf, 32, 5000);
      hex_print(stdout,buf,nBytes);
      fprintf(stdout,"\n");
    } else if(strcmp(argv[1], "test") == 0){
        int i, v, r;
/* The test consists of writing 1000 random numbers to the device and checking
 * the echo. This should discover systematic bit errors (e.g. in bit stuffing).
 */
        for(i=0;i<1000;i++){
            v = rand() & 0xffff;
            nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, PSCMD_ECHO, v, 0, (char *)buffer, sizeof(buffer), 5000);
            if(nBytes < 2){
                if(nBytes < 0)
                    fprintf(stderr, "USB error: %s\n", usb_strerror());
                fprintf(stderr, "only %d bytes received in iteration %d\n", nBytes, i);
                exit(1);
            }
            r = buffer[0] | (buffer[1] << 8);
            if(r != v){
                fprintf(stderr, "data error: received 0x%x instead of 0x%x in iteration %d\n", r, v, i);
                exit(1);
            }
        }
        printf("test succeeded\n");
    }
    usb_close(handle);
    return 0;
}
