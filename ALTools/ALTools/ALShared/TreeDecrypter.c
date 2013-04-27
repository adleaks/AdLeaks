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

#include "TreeDecrypter.h"

#include <math.h>
#include "OCObject.h"


struct TreeDecrypter{
	struct OCObject _obj;

	TDItemPtr* tree;
	TDItemPtr* dtree;

	struct TDDelegate delegate;
	
	int treeSize;
	int nLeaves;
	int firstLeaf;
	int lastLeaf;
	int depth;
};

TDItemPtr TreeDecrypterPopulateTreeFromNode(TreeDecrypterRef td, int startNode);
int TreeDecrypterDoAlgorithmFromNode(TreeDecrypterRef me, int startNode, TDItemPtr startNodeVal);

void TreeDecrypterDealloc(void* ptr){
	TreeDecrypterRef me = (TreeDecrypterRef) ptr;
	
	int i = 0;
	for(i=0; i < me->treeSize; i++)
	{
		me->delegate.deleteItem(me->delegate.delegateObject, me->tree[i]);
		me->delegate.deleteItem(me->delegate.delegateObject, me->dtree[i]);
	}
	OCObjectRelease(&me->delegate.delegateObject);

	OCDeallocate(ocDefaultAllocator, me->tree);
	OCDeallocate(ocDefaultAllocator, me->dtree);
}

TreeDecrypterRef TreeDecrypterCreate(int nLeaves, struct TDDelegate delegate){
	OCOBJECT_ALLOCINIT(TreeDecrypter);
	me->nLeaves = nLeaves;
	me->treeSize = 2*nLeaves-1; //nLeaves/2 + nLeaves/2^2 + ... + 1
	me->firstLeaf = nLeaves-1;
	me->lastLeaf = me->treeSize-1;
	me->depth = log2(me->treeSize);
	me->delegate = delegate;
	OCObjectRetain(me->delegate.delegateObject);
	
	me->tree = (TDItemPtr*) OCAllocate(ocDefaultAllocator, sizeof(TDItemPtr)*me->treeSize);
	me->dtree = (TDItemPtr*) OCAllocate(ocDefaultAllocator, sizeof(TDItemPtr)*me->treeSize);
	int i = 0;
	for(i = 0; i < me->treeSize; i++)
	{
		me->tree[i] = me->delegate.createEncryptedNeutral(me->delegate.delegateObject); 
		me->dtree[i] = me->delegate.createDecryptedNeutral(me->delegate.delegateObject);
	}
	
	return me;	
}

void TreeDecrypterSetLeaf(TreeDecrypterRef me, int iLeaf, TDItemPtr c){
	if(iLeaf >= me->nLeaves)
		return;
	
	TDItemPtr l = me->tree[me->firstLeaf+iLeaf];
	me->delegate.setItem(me->delegate.delegateObject, l, c);
}

TDItemPtr TreeDecrypterGetLeaf(TreeDecrypterRef me, int iLeaf){
	int index = me->firstLeaf+iLeaf;
	return me->dtree[index];
}

OCBool TreeDecrypterIsLeafZero(TreeDecrypterRef me, int iLeaf){
	int index = me->firstLeaf+iLeaf;
	if(index< me->firstLeaf || index > me->lastLeaf)
		return NO;

	TDItemPtr chunk = me->dtree[index];
	return me->delegate.isDecryptedNeutral(me->delegate.delegateObject, chunk);
}

void TreeDecrypterPopulateTree(TreeDecrypterRef td){
	TreeDecrypterPopulateTreeFromNode(td, 0);
}

TDItemPtr TreeDecrypterPopulateTreeFromNode(TreeDecrypterRef me, int startNode){
	int left = 2*startNode + 1;
	int right = left + 1;
	
	if(left < me->lastLeaf)
	{
		TDItemPtr leftNode = TreeDecrypterPopulateTreeFromNode(me, left);
		TDItemPtr rightNode = TreeDecrypterPopulateTreeFromNode(me, right);
		
		me->delegate.combine(me->delegate.delegateObject, me->tree[startNode], leftNode, rightNode);
	}
	
	return me->tree[startNode];
}

int TreeDecrypterDoAlgorithm(TreeDecrypterRef td){
	if(td == NULL)
		return 0;
	
	return TreeDecrypterDoAlgorithmFromNode(td, 0, NULL);
}

int TreeDecrypterDoAlgorithmFromNode(TreeDecrypterRef me, int startNode, TDItemPtr startNodeVal){
	int left = 2*startNode+1;
	int right = left+1;
	int count = 0;

	TDItemPtr value = me->dtree[startNode];
	
	if(startNodeVal != NULL)
		me->delegate.setItem(me->delegate.delegateObject, value, startNodeVal);
	else
	{
		me->delegate.decrypt(me->delegate.delegateObject, value, me->tree[startNode]);
		count++;
	}
	// note: assignment to dtree[...] is done above
	
	if(me->delegate.isDecryptedNeutral(me->delegate.delegateObject, value))
		return count;

	if(left < me->lastLeaf)
	{
		TDItemPtr tmp = me->delegate.createDecryptedNeutral(me->delegate.delegateObject);
		int lcount = TreeDecrypterDoAlgorithmFromNode(me, left, NULL);
		me->delegate.uncombine(me->delegate.delegateObject, tmp, value, me->dtree[left]);
		int rcount = TreeDecrypterDoAlgorithmFromNode(me, right, tmp);
		
		count += lcount + rcount;	
		me->delegate.deleteItem(me->delegate.delegateObject, tmp);
	}
	
	return count;
}


void TreeDecrypterPrintAll(TreeDecrypterRef td){
	VAR_UNUSED(td);
}

