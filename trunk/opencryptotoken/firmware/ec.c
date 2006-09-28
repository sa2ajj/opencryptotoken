#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "ec.h"
#include <avr/eeprom.h>

bignum_t field_b,field_a,field_prime,field_order;
ec_point_t G;
bignum_t private_key;

//uzywany w pinv 
//bignum_t *prime_ptr=&field_prime;

//ec_point_t public_key;

unsigned int P_field_b[] PROGMEM = {
	0xb9b1,0xc146,0xdeec,0xfeb8,0x3049,0x7224,
	0xe9ab,0x0fa7,0x80e7,0xe59c,0x0519,0x6421
	};

//a=-3
unsigned int P_field_a[] PROGMEM = {
	0xfffc,0xffff,0xffff,0xffff,0xfffe,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
	};

//prime
unsigned int P_field_prime[] PROGMEM  = {
	0xffff,0xffff,0xffff,0xffff,0xfffe,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
	};
//order
unsigned int P_field_order[] PROGMEM  = {
	0x2831,0xb4d2,0xc9b1,0x146b,0xf836,0x99de,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
	};
/*FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831 order from openssl*/
unsigned int P_G[] PROGMEM = {
	0x1012,0x82ff,0x0afd,0xf4ff,0x8800,0x43a1,
	0x20eb,0x7cbf,0x90f6,0xb030,0xa80e,0x188d,

	0x4811,0x1e79,0x77a1,0x73f9,0xcdd5,0x6b24,
	0x11ed,0x6310,0xda78,0xffc8,0x2b95,0x0719,

	0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

unsigned char P_private_key[] PROGMEM = {
0xb8,0x93,0x92,0x5c,0x97,0x0f,0x93,0x8d,0xe7,0xec,0xf1,0xea,
0x9b,0xec,0xef,0x3b,0xb5,0x22,0xf3,0xb2,0x6a,0xc2,0xdf,0xbb
};

void ec_init() {
	memcpy_P(field_prime.value,P_field_prime,N);
	memcpy_P(field_order.value,P_field_order,N);
	memcpy_P(field_a.value,P_field_a,N);
	memcpy_P(field_b.value,P_field_b,N);
	memcpy_P(G.X.value,P_G,3*N);
	memcpy_P(&private_key,P_private_key,N);

//	eeprom_read_block(private_key.value,(void *)0,N);
//	eeprom_read_block(public_key.X.value,(void *)N,2*N);
//	memset(public_key.Z.value,0,N);
//	public_key.Z.value[0]=1;
}

void div_mod(bigbignum_t *a, bignum_t *mod) {
  bigbignum_t bb;
  bigbignum_t tmp;
  int i;
  memset(bb.lo.value,0,N);
  bb.hi=*mod;
//  memcpy(bb+N,b,N);

  i=N*8;
  while(!(bb.hi.value[N-1]&0x80)) {
    bin_shiftl(&bb.hi,&bb.hi); i++; 
  }  

  for(;i>=0; i--) {
//    tmp=*a;
    int lz=bin_sub_2N(a,&bb,&tmp);
    if(!lz) *a=tmp;
    bin_shiftr_2N(&bb,&bb);    
  }
}

void add_mod(bignum_t *a, bignum_t *b, bignum_t *c, bignum_t *mod) {
	unsigned char carry=bin_add(a,b,c);
//	if(carry) bin_sub(c,mod,c);
}

void mul_mod(bignum_t *a, bignum_t *b, bignum_t *c, bignum_t *mod) {
	bigbignum_t bn;
	bin_mul(a,b,&bn);
	div_mod(&bn,mod);
	*c=bn.lo;
}

int ecdsa_sign_setup(bignum_t *r, bignum_t *kinv) {
	
} 

int ecdsa_sign(unsigned char *dgst,int dgst_len, ecdsa_sig_t *ecsig) {
	bignum_t k,kinv,t1,t2,dg;
	bigbignum_t r;
	
	ec_point_t ptmp;
	int i;
// mala szansa (bardzo mala :) ze k bedzie 0 albo wieksze od field_prime
// na razie olewamy sprawdzanie
	if(dgst_len>N)
		return 0;
	memcpy(&k,entrophy,N);
	memset(dg.value,0,N);
//	memset(k.value,0,N);
//	k.value[0]=2;
	for(i=0; i<dgst_len; i++) 
		dg.value[i]=dgst[dgst_len-1-i];

// tmp = k*G
	ec_mul(&G,&k,&ptmp);
	ec_normalize(&ptmp);

// kolejne operacje modulo field_order
	memset(&r,0,sizeof(r));
	r.lo=ptmp.X;
	div_mod(&r,&field_order);
	ecsig->R=r.lo;	
	inv_mod(&k,&kinv,&field_order);
	mul_mod(&private_key,&ecsig->R,&t1,&field_order);
//	ecsig->S=t1;
	add_mod(&t1,&dg,&t2,&field_order);
	mul_mod(&kinv,&t2,&(ecsig->S),&field_order);
//	ecsig->S=kinv;
//ecsig->s jest (?) z wysokim prawdopodobienstwem rozne od zera, 
//olewamy sprawdzanie
	return 1;
}

int ec_test() {
//	ec_normalize((unsigned char *)gx);
	ec_point_t a;
	bignum_t k;
	memset(k.value,0x55,N);
//	for(i=0; i<2; i++) k[i]=0x55;
	
	if(!ec_on_curve(&G)) return 0;
	ec_mul(&G,&k,&a);
	ec_normalize(&a);
	return ec_on_curve(&a);
//	return 1;
}
