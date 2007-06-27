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

//prime192v1 (fast reduction algorithm only for this curve)

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

//unsigned char P_private_key[] PROGMEM = {
//0xb8,0x93,0x92,0x5c,0x97,0x0f,0x93,0x8d,0xe7,0xec,0xf1,0xea,
//0x9b,0xec,0xef,0x3b,0xb5,0x22,0xf3,0xb2,0x6a,0xc2,0xdf,0xbb
//};

// new test priv key
unsigned char P_private_key[] PROGMEM = {
0x0c,0x41,0x60,0x93,0x45,0x41,0x5c,0xe1,0x83,0x47,0xb2,0xe1,
0x00,0xca,0x0d,0x3b,0x4f,0x2d,0xce,0x48,0xef,0x18,0x95,0x79
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

void add_mod(bignum_t *c, bignum_t *a, bignum_t *b, bignum_t *mod) {
	unsigned char carry=bin_add(a,b,c);
	if(carry) bin_sub(c,mod,c);
}

void mul_mod(bignum_t *c, bignum_t *a, bignum_t *b, bignum_t *mod) {
	bigbignum_t bn;
	bin_mul(a,b,&bn);
	div_mod(&bn,mod);
	*c=bn.lo;
}


typedef unsigned char uchar;

uchar is_zero(bignum_t *k) {
	uchar *ptr=k->value;
	uchar *ptre=ptr+N;
	while(ptr<ptre && !*ptr) ptr++;
	return (ptr==ptre);
}

uchar ecisinf(ec_point_t *point) {
	return is_zero(&point->Y);
}

/*
void bin_shiftr(bignum_t *k) {
	uchar carry=0;
	uchar *ptr=k->value+N-1;
	while(ptr>=k->value) {
		uchar nc=(*ptr)&1;
		*ptr=(*ptr>>1)|carry;
		carry=nc?0x80:0;
		ptr--;
	}
}
*/

void field_add(bignum_t *r,bignum_t *a, bignum_t *b) {
	unsigned char carry=bin_add(a,b,r);
	if(carry) bin_sub(r,&field_prime,r);
}

void field_sub(bignum_t *r,bignum_t *a, bignum_t *b) {
	unsigned char carry=bin_sub(a,b,r);
	if(carry) bin_add(r,&field_prime,r);
}

void fast192reduction(bignum_t *result, bigbignum_t *bn) {
	bignum_t tmp;
	int i;
	for(i=0; i<8; i++) {
		tmp.value[i]=(tmp.value+8)[i]=bn->hi.value[i];
		(tmp.value+16)[i]=0;
	}
	field_add(result,&tmp,&bn->lo);
	for(i=0; i<8; i++) {
		tmp.value[i]=0;
		(tmp.value+8)[i]=(tmp.value+16)[i]=(bn->hi.value+8)[i];
	}
	field_add(result,result,&tmp);
	for(i=0; i<8; i++) {
		tmp.value[i]=(tmp.value+8)[i]=(tmp.value+16)[i]=(bn->hi.value+16)[i];
	}
	field_add(result,result,&tmp);
}


void field_mul(bignum_t *r,bignum_t *a, bignum_t *b) {
	bigbignum_t bn;
	bin_mul(a,b,&bn);
	fast192reduction(r,&bn);
}


void field_sqr(bignum_t *r,bignum_t *a) {
	field_mul(r,a,a);
}

//#define field_sqr(r,a) field_mul(r,a,a)

void ec_normalize(ec_point_t *point) {
	bignum_t n0,n1;
	inv_mod(&n0,&point->Z,&field_prime);	// n0=Z^-1
	field_sqr(&n1,&n0);			// n1=Z^-2
	field_mul(&point->X,&point->X,&n1);	// X*=n1
	field_mul(&n0,&n0,&n1);			// n0=Z^-3	
	field_mul(&point->Y,&point->Y,&n0);
	memset(&point->Z,0,sizeof(point->Z));
	point->Z.value[0]=1;
}

// http://en.wikibooks.org/wiki/Prime_Curve_Jacobian_Coordinates


#define AISMINUS3 1

// a*=2
void ec_dbl(ec_point_t *a) {
	bignum_t S,M,YY,ZZ,T;
	if(ecisinf(a)) return;
	field_sqr(&YY,&a->Y);
	field_sqr(&ZZ,&a->Z);

#ifdef AISMINUS3
	field_add(&T,&a->X,&ZZ);
	field_sub(&M,&a->X,&ZZ);
	field_mul(&M,&M,&T);
	field_add(&T,&M,&M);
	field_add(&M,&T,&M);	//M=3*(X-Z^2)*(X+X^2)
#else
	field_sqr(&T,&ZZ);
	field_mul(&T,&field_a,&T); //T=a*Z^4
	field_sqr(&M,&X);
	field_add(&S,&M,&M);
	field_add(&M,&S,&M);	
	field_add(&M,&M,&T);
#endif

	field_mul(&S,&a->X,&YY);
	field_add(&S,&S,&S);
	field_add(&S,&S,&S); // S=4*X*Y^2

	field_sqr(&T,&M);	
	field_add(&a->X,&S,&S);
	field_sub(&a->X,&T,&a->X);    // X'=M^2-2*S

	field_mul(&a->Z,&a->Y,&a->Z);
	field_add(&a->Z,&a->Z,&a->Z); // Z'=2*Y*Z

	field_sqr(&T,&YY);
	field_add(&T,&T,&T);
	field_add(&T,&T,&T);
	field_add(&T,&T,&T);

	field_sub(&a->Y,&S,&a->X);
	field_mul(&a->Y,&M,&a->Y);
	field_sub(&a->Y,&a->Y,&T);   //Y'=M*(S-X') - 8*Y^4
}


//a+=b;
void ec_add(ec_point_t *a,ec_point_t *b) {
	bignum_t u1,u2,s1,s2,t1,t2,t3;
	if(ecisinf(b)) return;
	if(ecisinf(a)) {*a=*b; return;}
	
	field_sqr(&t1,&b->Z);
	field_mul(&u1,&a->X,&t1);	//u1=X1*Z2^2
	
	field_sqr(&t2,&a->Z);
	field_mul(&u2,&b->X,&t2);	//u2=X2*Z1^2

	field_mul(&t1,&t1,&b->Z);
	field_mul(&s1,&a->Y,&t1);	//s1=Y1*Z2^3
	
	field_mul(&t2,&t2,&a->Z);
	field_mul(&s2,&b->Y,&t2);	//s2=Y2*Z1^3

	field_sub(&u2,&u2,&u1);
	field_sub(&s2,&s2,&s1);

	if(is_zero(&u2)) {
		if(is_zero(&s2)) {
			ec_dbl(a);
			return;
		} else {
			//inf
			memset(a,0,sizeof(*a));
			return;
		}
	}

#define	H u2
#define R s2

	field_sqr(&t1,&H);	//t1 = H^2
	field_mul(&t2,&H,&t1);  //t2 = H^3
	field_mul(&t3,&u1,&t1);	//t3=u1*h^2
	
	field_sqr(&a->X,&R);
	field_sub(&a->X,&a->X,&t2);

	field_sub(&a->X,&a->X,&t3);  
	field_sub(&a->X,&a->X,&t3);  //X3=R^2 - H^3 - 2*U1*H^2

	field_sub(&a->Y,&t3,&a->X);
	field_mul(&a->Y,&a->Y,&R);

	field_mul(&t3,&s1,&t2);
	field_sub(&a->Y,&a->Y,&t3);

	field_mul(&a->Z,&a->Z,&b->Z);
	field_mul(&a->Z,&a->Z,&H);
}
#undef H
#undef R

void ec_mul(ec_point_t *result, ec_point_t *point,bignum_t *ak) {
	ec_point_t tmp=*point;
	bignum_t k=*ak;
	memset(result,0,sizeof(*result));	//result=inf
	while(!is_zero(&k)) {
		if(k.value[0]&1) ec_add(result,&tmp);
		ec_dbl(&tmp);
		bin_shiftr(&k,&k);
	}
	
}

int ecdsa_sign_setup(bignum_t *r, bignum_t *kinv) {
	return 0;
} 

int ecdsa_sign(ecdsa_sig_t *ecsig,unsigned char *dgst,int dgst_len) {
	bignum_t k,dg;
	bigbignum_t r;
	
	ec_point_t ptmp;
	int i;
// mala szansa (bardzo mala :) ze k bedzie 0 albo wieksze od field_prime
// na razie olewamy sprawdzanie
	if(dgst_len>N)
		return 0;
	memcpy(&k,entrophy,N);
	memset(&dg,0,sizeof(dg));
//	memset(k.value,0x55,N);
//	k.value[0]=2;
	for(i=0; i<dgst_len; i++) 
		dg.value[i]=dgst[dgst_len-1-i];

	ec_mul(&ptmp,&G,&k);
	ec_normalize(&ptmp);

// kolejne operacje modulo field_order
	memset(&r.hi,0,sizeof(r.hi));
	r.lo=ptmp.X;
	div_mod(&r,&field_order);
	ecsig->R=r.lo;	
	inv_mod(&k,&k,&field_order);
	mul_mod(&ecsig->S,&private_key,&ecsig->R,&field_order);

	add_mod(&ecsig->S,&dg,&ecsig->S,&field_order);
	mul_mod(&ecsig->S,&k,&ecsig->S,&field_order);

//ecsig->s jest (?) z wysokim prawdopodobienstwem rozne od zera, 
//olewamy sprawdzanie
	return 1;
}
/*
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
*/
