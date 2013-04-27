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

#ifndef _TREEDECRYPTER_H_
#define _TREEDECRYPTER_H_

#include "OCObject.h"
#include "OCBase.h"

/*
 * The TreeDecrypter implements the "tree decryption" algorithm
 * Use it in the following order of calls
 *	1) TreeDecrypterCreate			(create the object)
 *	2) TreeDecrypterSetLeaf			(set leaf data (== ciphertexts to decrypt))
 *	3) TreeDecrypterPopulateTree		(do preprocessing in front of  algorithm)
 *	4) TreeDecrypterDoAlgorithm		(do the actual algorithm and decryption)
 */

typedef void* TDItemPtr;
struct TDDelegate{
	OCObjectRef delegateObject;
	
	TDItemPtr (*createEncryptedNeutral)(OCObjectRef delegate);
	TDItemPtr (*createDecryptedNeutral)(OCObjectRef delegate);
	OCBool (*isDecryptedNeutral)(OCObjectRef delegate, TDItemPtr);

	void (*combine)(OCObjectRef delegate, TDItemPtr to, TDItemPtr a, TDItemPtr b);
	void (*uncombine)(OCObjectRef delegate, TDItemPtr to, TDItemPtr a, TDItemPtr b);
	void (*decrypt)(OCObjectRef delegate, TDItemPtr to, TDItemPtr from);
	
	void (*setItem)(OCObjectRef delegate, TDItemPtr to, TDItemPtr from);
	void (*deleteItem)(OCObjectRef delegate, TDItemPtr item);
};

typedef struct TreeDecrypter* TreeDecrypterRef;

TreeDecrypterRef TreeDecrypterCreate(int nLeaves, struct TDDelegate delegate);

void TreeDecrypterSetLeaf(TreeDecrypterRef td, int iLeaf, TDItemPtr c);
TDItemPtr TreeDecrypterGetLeaf(TreeDecrypterRef td, int iLeaf);
OCBool TreeDecrypterIsLeafZero(TreeDecrypterRef td, int iLeaf);

void TreeDecrypterPopulateTree(TreeDecrypterRef td); 
int TreeDecrypterDoAlgorithm(TreeDecrypterRef td);

void TreeDecrypterPrintAll(TreeDecrypterRef td);


#endif
