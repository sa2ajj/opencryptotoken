#define N 24
#define AVRCMD_SELECT_FILE 4
#define AVRCMD_SIGN 5
#define AVRCMD_MULT 6
#define AVRCMD_READ_BINARY 0x81
#define AVRCMD_GETRESULT 0x82
#define AVRCMD_GETRND 0x83

#define AVR_FILE_BUF_SIZE 128

#ifndef __ASSEMBLER__

typedef struct {
  unsigned char value[N];
} bignum_t;

typedef struct {
  bignum_t lo;
  bignum_t hi;
} bigbignum_t;

typedef struct {
  bignum_t X;
  bignum_t Y;
  bignum_t Z;
} ec_point_t;

typedef struct {
  bignum_t R;
  bignum_t S;
} ecdsa_sig_t;

extern void ec_mul(ec_point_t *x, ec_point_t *b, bignum_t *k);
extern unsigned char ec_on_curve(ec_point_t *x);
extern void ec_normalize(ec_point_t *x);
extern int ec_test();
extern void ec_init();

extern void bn_mod_add(bignum_t *a, bignum_t *b, bignum_t *c);
extern void bn_mod_mul(bignum_t *a, bignum_t *b, bignum_t *c);
extern void bn_mod_inv(bignum_t *a, bignum_t *b);

extern void inv_mod(bignum_t *a, bignum_t *z, bignum_t *mod);

extern unsigned char bin_sub_2N(bigbignum_t *x, bigbignum_t *y, bigbignum_t *z);
extern unsigned char bin_shiftr_2N(bigbignum_t *x, bigbignum_t *z);
extern unsigned char bin_shiftl(bignum_t *x, bignum_t *z);
extern unsigned char bin_shiftr(bignum_t *x, bignum_t *z);

extern void bin_mul(bignum_t *a,bignum_t *b,bigbignum_t *z);
extern unsigned char bin_add(bignum_t *a,bignum_t *b,bignum_t *c);
extern unsigned char bin_sub(bignum_t *a,bignum_t *b,bignum_t *c);

extern bignum_t field_b;
extern bignum_t field_a;
extern bignum_t field_prime;
extern bignum_t field_order;
extern ec_point_t G;
extern unsigned char entrophy[32];
extern bignum_t private_key;
extern int ecdsa_sign(ecdsa_sig_t *ecsig,unsigned char *dgst,int dgst_len);

extern void usbAppIdle();
extern void usbAppInit();
#endif
