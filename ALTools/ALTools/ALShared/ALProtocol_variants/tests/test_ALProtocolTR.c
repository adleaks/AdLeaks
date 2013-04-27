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

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "ALProtocolTR.h"
#include "CryptoRandom.h"
#include "ALChunkTR_private.h"

#define VAR_UNUSED(x) (void)(x)

ALProtocolTRRef alp = NULL;

void setup(void){	
	alp = ALProtocolTRCreateNewInstance(2048, 1);
	fail_if(alp == NULL, "could not create protocol");
}

void teardown(void){	
}

START_TEST(encNeutral)
{
	ALChunkTRRef chunk = ALProtocolTREncNeutral(alp);
	OCStatus ok = ALProtocolTRDecVrfy(alp, chunk);	
	fail_unless(ok, "neutral decryption failed");
}
END_TEST

START_TEST(encData)
{
	OCByte* bytes;
	size_t nBytes;
	ALChunkTRRef chunk;
	
	mpz_t m;
	mpz_t m_;
	mpz_init(m);
	mpz_init(m_);
	mpz_t t;
	mpz_init(t);
	
	DJRef djM = ALProtocolTRGetDJM(alp);
	size_t len = DJGetS(djM) * DJGetModulusBits(djM)/8 - DJGetS(djM);
	len -= 4*32;
	OCByte* buf = OCAllocate(ocDefaultAllocator, len);
	
	nBytes = len;
	bytes = OCAllocate(ocDefaultAllocator, len);
	CryptoRandomRand(bytes, len, CryptoRandomSourceDevURandom);
	mpz_import(m, len, 1, 1, 1, 0, buf);
	mpz_mod(m_, m, *DJGetPlaintextModulus(djM));
	int cmpVal = mpz_cmp(m_, m);
	fail_unless(cmpVal == 0, "creation of data failed");
	
	chunk = ALProtocolTREncData(alp, bytes, nBytes);
	
	OCStatus ok = ALProtocolTRDecVrfy(alp, chunk);	
	fail_unless(ok, "decryption failed");
	
}
END_TEST


int main(int argc, char** argv){
	int number_failed;

	Suite* suite = suite_create("ALProtocolTR (TechReport) Suite");
	TCase* tcase = tcase_create("creation, encryption and decryption");	
	tcase_add_checked_fixture(tcase, setup, teardown);
	tcase_add_test(tcase, encNeutral);
	tcase_add_test(tcase, encData);
	tcase_set_timeout(tcase, 100);
	suite_add_tcase(suite, tcase);

	SRunner* runner = srunner_create(suite);
	srunner_run_all(runner, CK_VERBOSE);
	number_failed = srunner_ntests_failed(runner);
	
	srunner_free(runner);
  
	VAR_UNUSED(argc);
	VAR_UNUSED(argv);

	return number_failed;
}
