/* This file is part of AdLeaks.
 * Copyright (C) 2012-2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * AdLeaks is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * AdLeaks is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AdLeaks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ALToolsConfig.h"
#include "DJ.h"
#include "OCBase.h"
#include "OCTime.h"
#include "CryptoRandom.h"
#include <stdio.h>
#include <gmp.h>

// fix a warning of gmp:
size_t __gmpz_out_str (FILE *, int, mpz_srcptr);

#define N 2

void doBenchmark(DJRef dj){
	OCTimeInterval encTimes[N];
	OCTimeInterval decTimes[N];
	OCTimeInterval multTimes[N];
	
	DJPlaintextRef plains[N];
	DJPlaintextRef plain;
	DJCipherTextRef ciphers[N];
	DJCipherTextRef cipher;
	
	// init
	int i;
	for(i = 0; i < N; i++)
	{
		mpz_init(plains[i]);
		mpz_init(ciphers[i]);
	}
	mpz_init(plain);
	mpz_init(cipher);

	int len = DJGetS(dj) * (DJGetModulusBits(dj)/8);
	unsigned char* buf = OCAllocate(ocDefaultAllocator, len * sizeof(char));

	// create plaintexts
	for(i = 0; i < N; i++)
	{
		CryptoRandomRand(buf, len, CryptoRandomSourceDevURandom);
		mpz_import(plains[i], len, 1, 1, 0, 0, buf);
		mpz_mod(plains[i], plains[i], *DJGetPlaintextModulus(dj));
//		mpz_set_ui(plains[i], 2);
	}
	free(buf); 
	buf = NULL;

	// encryption
	for(i = 0; i < N; i++)
	{
		OCTimeInterval t1 = OCTimeIntervalSinceReferenceDate();
		DJEncrypt(dj, ciphers[i], plains[i], NULL);
		OCTimeInterval t2 = OCTimeIntervalSinceReferenceDate();
		encTimes[i] = t2-t1;
	}
	
	// decryption
	for(i = 0; i < N; i++)
	{
		OCTimeInterval t1 = OCTimeIntervalSinceReferenceDate();
		DJDecrypt(dj, plain, ciphers[i]);
		OCTimeInterval t2 = OCTimeIntervalSinceReferenceDate();
		if(mpz_cmp(plain, plains[i]) != 0)
		{
			OCLog("error enc/dec:");			   
			mpz_out_str(stderr, 10, plain);
			OCLog("");
			fflush(stderr);
		}

		decTimes[i] = t2-t1;
	}
	
	// mult
	for(i = 0; i < N; i++)
	{
		OCTimeInterval t1 = OCTimeIntervalSinceReferenceDate();
		DJMult(dj, cipher, ciphers[i], ciphers[(i+1) % N]);
		OCTimeInterval t2 = OCTimeIntervalSinceReferenceDate();
		multTimes[i] = t2-t1;
	}

	OCLog("s=%d, modulus=%d bits", DJGetS(dj), DJGetModulusBits(dj));
	OCLog("enc\t\t\tdec\t\t\tmult");
	// output
	for(i = 0; i < N; i++)
		OCLog("%f\t%f\t%f", encTimes[i], decTimes[i], multTimes[i]);
	
	
	// cleanup
	for(i = 0; i < N; i++)
	{
		mpz_clear(plains[i]);
		mpz_clear(ciphers[i]);
	}
	mpz_clear(plain);
	mpz_clear(cipher);
}


void test(DJRef dj){
	
	DJPlaintextRef plain;
	DJCipherTextRef cipher;
	
	mpz_init_set_ui(plain, 0);
	mpz_init(cipher);
	
	DJEncrypt(dj, cipher, plain, NULL);
	mpz_out_str(stdout, 16, cipher);
	fflush(stdout);
	fprintf(stdout, "\n");
	mpz_powm(cipher, cipher, *DJGetModulus(dj), *DJGetCipherModulus(dj));
	mpz_out_str(stdout, 16, cipher);
	fflush(stdout);
	fprintf(stdout, "\n");
	mpz_powm(cipher, cipher, *DJGetModulus(dj), *DJGetCipherModulus(dj));
	mpz_out_str(stdout, 16, cipher);
	fflush(stdout);
	fprintf(stdout, "\n");
	DJDecrypt(dj, plain, cipher);
	mpz_out_str(stdout, 16, plain);
	fflush(stdout);
	fprintf(stdout, "\n");
	exit(0);
}

int main(int argc, char** argv)
{
	VAR_UNUSED(argc);
	VAR_UNUSED(argv);
	DJPublicKeyRef pub;
	DJPrivateKeyRef prv;
	DJKeyGen(2048, &pub, &prv);
	OCLog("started here");

//	DJRef dj = DJCreate(1, pub, prv);
//	test(dj);
	

	int s = 1;
	for(s = 1; s <= 9; s++)
	{
		DJRef _dj = DJCreate(s, pub, prv, NULL);

		doBenchmark(_dj);
		
		OCObjectRelease((OCObjectRef*)&_dj);		
	}
	return 0;
}
