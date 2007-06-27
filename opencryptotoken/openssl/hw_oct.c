/* crypto/engine/hw_oct.c */
 /* Written by Fred Donnat (frederic.donnat@oct.com) for "oct"
 * engine integration in order to redirect crypto computing on a crypto
 * hardware accelerator zenssl32  ;-)
 *
 * Date : 25 jun 2002
 * Revision : 17 Ju7 2002
 * Version : oct_engine-0.9.7
 */

/* ====================================================================
 * Copyright (c) 1999-2001 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */


/* ENGINE general include */
#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/dso.h>
#include <openssl/engine.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

#include "ecs_locl.h"

#include <stdlib.h>
#include <string.h>
#include <usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */
#include "../firmware/ec.h"

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */



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


/* OpenCryptoToken uses the free shared default VID/PID. If you want to see an
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
















#ifndef OPENSSL_NO_HW
#ifndef OPENSSL_NO_HW_OCT

#ifdef FLAT_INC
#  include "hw_oct.h"
#else
#  include "vendor_defns/hw_oct.h"
#endif

#define OCT_LIB_NAME "OpenCryptoToken engine"
#include "hw_oct_err.c"

#define FAIL_TO_SOFTWARE		-15

#if 0
#  define PERROR(s)	perror(s)
#  define CHEESE()	fputs("## [ZenEngine] ## " __FUNCTION__ "\n", stderr)
#else
#  define PERROR(s)
#  define CHEESE()
#endif


/* Sorry ;) */
#ifndef WIN32
static inline void esrever ( unsigned char *d, int l )
{
	for(;--l>0;--l,d++){*d^=*(d+l);*(d+l)^=*d;*d^=*(d+l);}
}

static inline void ypcmem ( unsigned char *d, const unsigned char *s, int l )
{
	for(d+=l;l--;)*--d=*s++;
}
#else
static __inline void esrever ( unsigned char *d, int l )
{
	for(;--l>0;--l,d++){*d^=*(d+l);*(d+l)^=*d;*d^=*(d+l);}
}

static __inline void ypcmem ( unsigned char *d, const unsigned char *s, int l )
{
	for(d+=l;l--;)*--d=*s++;
}
#endif



/* Function for ENGINE detection and control */
static int oct_destroy ( ENGINE *e ) ;
static int oct_init ( ENGINE *e ) ;
static int oct_finish ( ENGINE *e ) ;
static int oct_ctrl ( ENGINE *e, int cmd, long i, void *p, void (*f) () ) ;

/* BIGNUM stuff */
static int oct_bn_mod_exp ( BIGNUM *r, const BIGNUM *a, const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx ) ;

/* Rand stuff */
static void RAND_oct_seed ( const void *buf, int num ) ;
static int RAND_oct_rand_bytes ( unsigned char *buf, int num ) ;
static int RAND_oct_rand_status ( void ) ;

/* Digest Stuff */
static int engine_digests ( ENGINE *e, const EVP_MD **digest, const int **nids, int nid ) ;

/* Cipher Stuff */
static int engine_ciphers ( ENGINE *e, const EVP_CIPHER **cipher, const int **nids, int nid ) ;

/* KM stuff */
static EVP_PKEY *hwcrhk_load_privkey(ENGINE *eng, const char *key_id,
	UI_METHOD *ui_method, void *callback_data);
static EVP_PKEY *hwcrhk_load_pubkey(ENGINE *eng, const char *key_id,
	UI_METHOD *ui_method, void *callback_data);



#define OCT_CMD_SO_PATH			ENGINE_CMD_BASE
static const ENGINE_CMD_DEFN oct_cmd_defns [ ] =
{
	{ OCT_CMD_SO_PATH,
	  "SO_PATH",
	  "Specifies the path to the 'zenbridge' shared library",
	  ENGINE_CMD_FLAG_STRING},
	{ 0, NULL, NULL, 0 }
} ;


static ECDSA_SIG *ecdsa_do_sign(const unsigned char *dgst, int dlen, 
		const BIGNUM *bn1, const BIGNUM *bn2, EC_KEY *eckey) {
			fprintf(stderr,"ecdsa_do_sign\n");
//			return 0;

    usb_dev_handle *handle = NULL;
    ECDSA_SIG *sig=ECDSA_SIG_new();
    int nBytes;

    usb_init();
    if(usbOpenDevice(&handle, USBDEV_SHARED_VENDOR, "its.sed.pl", USBDEV_SHARED_PRODUCT, "OpenCryptoToken") != 0){
        fprintf(stderr, "Could not find USB device \"OpenCryptoToken\" with vid=0x%x pid=0x%x\n", USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT);
        goto sig_ex;
    }
    nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE, AVRCMD_SIGN, 0, 0, (char *)dgst, dlen, 5000);
    int n=0;
    unsigned char buf[256];
    memset(buf,0,256);
    ecdsa_sig_t avrsig,mirrored;
    while((nBytes=usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, AVRCMD_GETRESULT, 0, 0, (char *)&avrsig, sizeof(avrsig), 5000))==0 && n++<25)
      usleep(200000);
    if(nBytes==sizeof(avrsig)) {
    	int i;
    	for(i=0; i<N; i++) {
    		mirrored.R.value[i]=avrsig.R.value[N-1-i];
    		mirrored.S.value[i]=avrsig.S.value[N-1-i];
    		BN_bin2bn(mirrored.R.value,N,sig->r);
    		BN_bin2bn(mirrored.S.value,N,sig->s);
    	}
/*      fprintf(stdout,"R:"); hex_print(stdout,sig.R.value,N);
      fprintf(stdout,"\nS:"); hex_print(stdout,sig.S.value,N);
      fprintf(stdout,"\n");*/
      
    } else {
      fprintf(stderr,"avrcryptotoken timeout\n");
    }
//			return ECDSA_OpenSSL()->ecdsa_do_sign(dgst,dlen,bn1,bn2,eckey);
sig_ex:
    if(handle)  usb_close(handle);
    return sig;
};
static int ecdsa_sign_setup(EC_KEY *eckey, BN_CTX *ctx_in, BIGNUM **kinvp, 
		BIGNUM **rp) {
			fprintf(stderr,"ecdsa_sign_setup\n");
			return ECDSA_OpenSSL()->ecdsa_sign_setup(eckey,ctx_in,kinvp,rp);
		};
static int ecdsa_do_verify(const unsigned char *dgst, int dgst_len, 
		const ECDSA_SIG *sig, EC_KEY *eckey) {
			fprintf(stderr,"ecdsa_do_verify\n");
			return ECDSA_OpenSSL()->ecdsa_do_verify(dgst,dgst_len,sig,eckey);
		};

static ECDSA_METHOD avr_ecdsa_meth = {
	"OpenCryptoToken ECDSA method",
	ecdsa_do_sign,
	ecdsa_sign_setup,
	ecdsa_do_verify,
#if 0
	NULL, /* init     */
	NULL, /* finish   */
#endif
	0,    /* flags    */
	NULL  /* app_data */
};

/* Our internal RAND_meth specific to oct ZNGINE providing pointers to  our function */
static RAND_METHOD oct_rand =
{
	RAND_oct_seed,
	RAND_oct_rand_bytes,
	NULL,
	NULL,
	RAND_oct_rand_bytes,
	RAND_oct_rand_status
} ;


/* Constants used when creating the ENGINE */
static const char *engine_oct_id = "oct";
static const char *engine_oct_name = "OpenCryptoToken";


/* This internal function is used by ENGINE_oct () and possibly by the
 * "dynamic" ENGINE support too   ;-)
 */
static int bind_helper ( ENGINE *e )
{
	const RAND_METHOD *meth_rand ;

	if ( !ENGINE_set_id ( e, engine_oct_id ) ||
			!ENGINE_set_name ( e, engine_oct_name ) ||
			!ENGINE_set_ECDSA (e, &avr_ecdsa_meth) ||
			!ENGINE_set_destroy_function ( e, oct_destroy ) ||
			!ENGINE_set_init_function ( e, oct_init ) ||
			!ENGINE_set_finish_function ( e, oct_finish ) ||
			!ENGINE_set_ctrl_function ( e, oct_ctrl ) ||
			!ENGINE_set_load_privkey_function(e, hwcrhk_load_privkey) ||
//			!ENGINE_set_load_pubkey_function(e, hwcrhk_load_pubkey) ||
			!ENGINE_set_cmd_defns ( e, oct_cmd_defns ) 
			) {
		return 0 ;
	}

	/* We use OpenSSL (SSLeay) meth to supply what we don't provide ;-*)
	 */
	meth_rand = RAND_SSLeay () ;

	/* meth_rand->seed ; */
	/* oct_rand.seed = meth_rand->seed ; */
	/* meth_rand->bytes ; */
	/* oct_rand.bytes = meth_rand->bytes ; */
	oct_rand.cleanup = meth_rand->cleanup ;
	oct_rand.add = meth_rand->add ;
	/* meth_rand->pseudorand ; */
	/* oct_rand.pseudorand = meth_rand->pseudorand ; */
	/* oct_rand.status = meth_rand->status ; */
	/* meth_rand->status ; */

	/* Ensure the oct error handling is set up */
	ERR_load_OCT_strings () ;
	return 1 ;
}


/* As this is only ever called once, there's no need for locking
 * (indeed - the lock will already be held by our caller!!!)
 */
static ENGINE *ENGINE_oct ( void )
{

	ENGINE *eng = ENGINE_new () ;

	if ( !eng ) {
		return NULL ;
	}
	if ( !bind_helper ( eng ) ) {
		ENGINE_free ( eng ) ;
		return NULL ;
	}

	return eng ;
}


#ifdef ENGINE_DYNAMIC_SUPPORT
static
#endif
void ENGINE_load_oct ( void )
{
	/* Copied from eng_[openssl|dyn].c */
	ENGINE *toadd = ENGINE_oct ( ) ;
	if ( !toadd ) return ;
	ENGINE_add ( toadd ) ;
	ENGINE_free ( toadd ) ;
	ERR_clear_error ( ) ;
}

/* Destructor (complements the "ENGINE_oct ()" constructor)
 */
static int oct_destroy (ENGINE *e )
{

	ERR_unload_OCT_strings () ;

	return 1 ;
}


/* (de)initialisation functions. Control Function
 */
static int oct_init ( ENGINE *e )
{
	return 1 ;
}


static int oct_finish ( ENGINE *e )
{

	CHEESE () ;

	return 1 ;
}


static int oct_ctrl ( ENGINE *e, int cmd, long i, void *p, void (*f) () )
{

	int initialised = 1;

	CHEESE () ;

	/*
	 * We Should add some tests for non NULL parameters or bad value !!
	 * Stuff to be done ...
	 */
	switch ( cmd ) {
	case OCT_CMD_SO_PATH :
/*
		if ( p == NULL ) {
			OCTerr ( OCT_F_OCT_CTRL, ERR_R_PASSED_NULL_PARAMETER ) ;
			return 0 ;
		}
		if ( initialised ) {
			OCTerr ( OCT_F_OCT_CTRL, OCT_R_ALREADY_LOADED ) ;
			return 0 ;
		}
		OCT_LIBNAME = (const char *) p ;
*/
		return 1 ;
	default :
		break ;
	}

	OCTerr ( OCT_F_OCT_CTRL, OCT_R_CTRL_COMMAND_NOT_IMPLEMENTED ) ;

	return 0 ;
}


/* RAND stuff Functions
 */
static void RAND_oct_seed ( const void *buf, int num )
{
	/* Nothing to do cause our crypto accelerator provide a true random generator */
}


static int RAND_oct_rand_bytes ( unsigned char *buf, int num )
{
/*
	zen_nb_t r;

	CHEESE();

	if ( !oct_dso ) {
		ENGINEerr(OCT_F_OCT_RAND, OCT_R_NOT_LOADED);
		return 0;
	}

	ptr_oct_init_number ( &r, num * 8, buf ) ;

	if ( ptr_oct_rand_bytes ( &r, ZENBRIDGE_RNG_DIRECT ) < 0 ) {
		PERROR("zenbridge_rand_bytes");
		ENGINEerr(OCT_F_OCT_RAND, OCT_R_REQUEST_FAILED);
		return 0;
	}
*/
	return 0;
}


static int RAND_oct_rand_status ( void )
{
//	CHEESE () ;

	return 1;
}


/* This stuff is needed if this ENGINE is being compiled into a self-contained
 * shared-library.
 */
#ifdef ENGINE_DYNAMIC_SUPPORT
static int bind_fn ( ENGINE *e, const char *id )
{

	if ( id && ( strcmp ( id, engine_oct_id ) != 0 ) ) {
		return 0 ;
	}
	if ( !bind_helper ( e ) )  {
		return 0 ;
	}

	return 1 ;
}

IMPLEMENT_DYNAMIC_CHECK_FN ()
IMPLEMENT_DYNAMIC_BIND_FN ( bind_fn )
#endif /* ENGINE_DYNAMIC_SUPPORT */




static EVP_PKEY *hwcrhk_load_privkey(ENGINE *eng, const char *key_id,
	UI_METHOD *ui_method, void *callback_data)
	{
	char *hexpub=
                    "04ec13cccae129c34b511de7c4531b"
                    "d2c63081aa49a58f51d4ccde964479"
                    "822a03b2ef7745ae4ea5074cbb58f216bacfe7"
	;                                                            
	fprintf(stderr,"start loading key\n");
	EVP_PKEY *res = NULL;
	EC_KEY *eckey=EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);
	EC_POINT *pub_key=EC_POINT_hex2point(EC_KEY_get0_group(eckey),hexpub,0,0);
	if(EC_KEY_generate_key(eckey)) 
		fprintf(stderr,"new key generated\n");
	
	EC_KEY_set_public_key(eckey,pub_key);
	res = EVP_PKEY_new();
	EVP_PKEY_assign_EC_KEY(res, eckey);
	fprintf(stderr,"key loaded\n");
/*
        if (!res)
                HWCRHKerr(HWCRHK_F_HWCRHK_LOAD_PRIVKEY,
                        HWCRHK_R_PRIVATE_KEY_ALGORITHMS_DISABLED);
*/
	return res;
 err:
	if (res)
		EVP_PKEY_free(res);
	return NULL;
	}

static EVP_PKEY *hwcrhk_load_pubkey(ENGINE *eng, const char *key_id,
	UI_METHOD *ui_method, void *callback_data)
	{
	EVP_PKEY *res = NULL;

        res = hwcrhk_load_privkey(eng, key_id,
                ui_method, callback_data);

	if (res)
		switch(res->type)
			{
		case EVP_PKEY_EC:
			{
			}
			break;
		default:
/*
			HWCRHKerr(HWCRHK_F_HWCRHK_LOAD_PUBKEY,
				HWCRHK_R_CTRL_COMMAND_NOT_IMPLEMENTED);*/
			goto err;
			}

	return res;
 err:
	if (res)
		EVP_PKEY_free(res);
	return NULL;
	}



#endif /* !OPENSSL_NO_HW_OCT */
#endif /* !OPENSSL_NO_HW */
