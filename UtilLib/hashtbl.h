/*
HashTable support sort and auto delete.
Written By: Qinghai Gou @2014
*/
#ifndef __HASHTBL_H__
#define __HASHTBL_H__
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

/////////////////////////////////////////////////////////////////////////////
// Global Function
#ifndef _FREE
#define _FREE(p) { if ((p)) { free((p)); (p) = NULL; } }
#endif
#ifndef _DELETE
#define _DELETE(p) if( (p) ) { delete (p); (p) = NULL; }
#endif
#ifndef _DELETEM
#define _DELETEM(p) if( (p) ) { delete[] (p); (p) = NULL; }
#endif

#define HASHTBL_SIZE		512

#define	ASC		0			// ascending order
#define DESC	1			// descending order

// FOR loop
struct _HASHTBLPOS {
	hash_size	index;		// HASH table index
	hash_size	offset;		// list number
	bool		bUseList;
};
typedef _HASHTBLPOS* PPOSITION;

// hashnode_s
template <class K, class T>
struct hashnode_s {
	T data;
	K key;

	hashnode_s* next;
};

// hashtable
template <class K, class T>
struct hashtable {
	hash_size			size;		// table size
	hash_size			used;		// used elements number

	_HASHTBLPOS			pos;		// for search
	hashnode_s<K, T>	**nodes;	// host
	size_t (*hashfunc)(K);			// hash code generator function
};

// ------------------------------------------------------------------
// function for generateKey
// ------------------------------------------------------------------
// WCHAR*
static size_t genHashKeyStrW(wchar_t* key) {
	size_t hash = 0;
	unsigned char* p = (unsigned char*)key;
	while(*p)
		hash = 31 * hash + (unsigned char) *p++;

	return hash;
};

// CHAR*
static size_t genHashKeyStrA(char* key) {
	size_t hash = 0;
	unsigned char* p = (unsigned char*)key;
	while(*p)
		hash = 31 * hash + (unsigned char) *p++;

	return hash;
};

static size_t genHashKeyDouble(double key) {
	char buff[1079] = {0};
	sprintf_s(buff, 1079, "%lf", key);

	return genHashKeyStrA(buff);
}

template <typename K>
size_t genHashKey(K key) {
	return (size_t)key;
};

// ------------------------------------------------------------------
// default function for free node
// ------------------------------------------------------------------
template<class K, class T>
struct _freeNull {
	void operator()(K key, T data) {
	}
};

struct _freePtr {
	void operator()(void* key, void* data) {
		if( key )
			free(key);
		if( data )
			free(data);
	}
};

struct _deletePtr {
	void operator()(void* key, void* data) {
		if( key )
			delete(key);
		if( data )
			delete(data);
	}
};

// ------------------------------------------------------------------
// default function for duplcate key
// ------------------------------------------------------------------
struct _dupKeyStrW {
	wchar_t* operator()(const wchar_t* key) const {
		return _wcsdup(key);
	}
};

struct _dupKeyStrA {
	char* operator()(const char* key) const {
		return _strdup(key);
	}
};

template <class K>
struct _dupKey {
	K operator()(K key) const {
		return key;
	}
};

// ------------------------------------------------------------------
// default function for compare key
// ------------------------------------------------------------------
struct _compareKeyStrW {
	int operator()(const wchar_t* k1, const wchar_t* k2, short order = ASC) const {
		int result = wcscmp(k1, k2);

		if( order == DESC )
			return -result;

		return result;
	}
};

struct _compareKeyStrA {
	int operator()(const char* k1, const char* k2, short order = ASC) const {
		int result = strcmp(k1, k2);

		if( order == DESC )
			return -result;

		return result;
	}
};

#ifdef _UNICODE
#define _compareKeyStr	_compareKeyStrW
#define _dupKeyStr		_dupKeyStrW
#define genHashKeyStr	genHashKeyStrW
#else
#define _compareKeyStr	_compareKeyStrA
#define _dupKeyStr		_dupKeyStrA
#define genHashKeyStr	genHashKeyStrA
#endif

// è∏èá
template<class K>
struct _compareKey {
	int operator()(const K& k1, const K& k2, short order = ASC) const {
		if( k1 == k2 )
			return 0;
		if( k1 > k2 )
			return (order == ASC) ? 1 : -1;
		if( k1 < k2 )
			return (order == ASC) ? -1 : 1;

		return 0;
	}
};

// ----------------------------------------------------------------------------
// template class HashTbl
// 
// ----------------------------------------------------------------------------
template <class K, class T, 
          class Clear = _freeNull<K, T>, 
          class Compare = _compareKey<K>,
		  class Key = _dupKey<K> >
class HashTbl {
public:
	typedef Compare		key_compare;
	typedef Key			key_dup;
	typedef Clear		key_free;

public:
	HashTbl(const key_compare& _keycomp = key_compare(), 
            const key_dup& _keydup = key_dup(),
            const key_free& _keyfree = key_free())
	{
		this->free_fn		= _keyfree;
		this->dup_fn		= _keydup;
		this->compare_fn	= _keycomp;

		this->freex_fn = NULL;

		this->hashtbl = NULL;
		this->keylist = NULL;
		this->orderBy = ASC;		// ascending order
	}

	~HashTbl() {
		destroy();
	}

	hashtable<K, T>* create(size_t (*hashfunc)(K) = genHashKey, bool bEnableSort = false, hash_size size = HASHTBL_SIZE) {
		hashtbl = (hashtable<K, T>*)malloc(sizeof(struct hashtable<K, T>));
		if( hashtbl == NULL )
			return NULL;
		memset(hashtbl, 0x00, sizeof(struct hashtable<K, T>));

		hashtbl->nodes = (hashnode_s<K, T>**)calloc(size, sizeof(struct hashnode_s<K, T>*));
		if( hashtbl->nodes == NULL ) {
			free(hashtbl);
			return NULL;
		}
		hashtbl->size = size;
		hashtbl->used = 0;

		if( hashfunc )
			hashtbl->hashfunc = hashfunc;
		else
			hashtbl->hashfunc = NULL;

		this->bEnableSort = bEnableSort;
		if( bEnableSort ) {
			keylist = new LinkedList<K>();
			sorted = false;
		}

		return hashtbl;
	}

	void setfree(void(*freefn)(K, T)) {
		this->freex_fn = freefn;
	}

	void orderby(short order) {
		orderBy = order;
	}

	short orderby() {
		return orderBy;
	}

	void removeall() {
		hash_size n = 0;
		struct hashnode_s<K, T> *node, *oldnode;

		if( hashtbl == NULL )
			return;

		for(n = 0; n < hashtbl->size; ++n) {
			node = hashtbl->nodes[n];
			while(node != NULL) {
				if (freex_fn != NULL)
					freex_fn(node->key, node->data);
				else 
					free_fn(node->key, node->data);

				oldnode = node;
				node = node->next;
				free(oldnode);
			}
		}
		// clear
		memset(hashtbl->nodes, 0x00,  hashtbl->size * sizeof(struct hashnode_s<K, T>*));
		memset(&hashtbl->pos, 0x00, sizeof(_HASHTBLPOS));
		hashtbl->used = 0;

		if( bEnableSort ) {
			keylist->clear();
			sorted = false;
		}
	}

	void destroy()
	{
		hash_size n = 0;
		struct hashnode_s<K, T> *node, *oldnode;

		if( hashtbl == NULL )
			return;

		for(n = 0; n < hashtbl->size; ++n) {
			node = hashtbl->nodes[n];
			while(node != NULL) {
				if (freex_fn != NULL)
					freex_fn(node->key, node->data);
				else 
					free_fn(node->key, node->data);

				oldnode = node;
				node = node->next;
				free(oldnode);
			}
		}
		free(hashtbl->nodes);
		free(hashtbl);
		hashtbl = NULL;

		if( bEnableSort ) {
			delete(keylist);
			keylist = NULL;
			sorted = false;
		}
	}

	void swapKey(hash_size i, hash_size j) {
		if( keylist ) {
			keylist->swap(i, j);
		}
	}

	int insert(K key, T data, hash_size idx = -1, short inspos = INSERT_BEFORE) {
		return insert(hashtbl, key, data, idx, inspos);
	}

	int add(K key, T data) {
		return insert(hashtbl, key, data, -1, INSERT_BEFORE);
	}

	int remove(K key) {
		return remove(hashtbl, key);
	}

	bool haskey(K key) {
		if( hashtbl == NULL )
			return false;

		hashnode_s<K, T>* node;
		hash_size hash = hashtbl->hashfunc(key) % hashtbl->size;

		node = hashtbl->nodes[hash];

		while( node ) {
			if( compare_fn(node->key, key) == 0 ) {
				return true;
			}

			node = node->next;
		}

		return false;
	}

	bool get(K key, T& value)
	{
		if( hashtbl == NULL )
			return false;

		hashnode_s<K, T>* node;
		hash_size hash = hashtbl->hashfunc(key) % hashtbl->size;

		node = hashtbl->nodes[hash];

		while( node ) {
			if( compare_fn(node->key, key) == 0 ) {
				value = node->data;
				return true;
			}

			node = node->next;
		}

		return false;
	}

	bool find(K key) {
		if (hashtbl == NULL)
			return false;

		hashnode_s<K, T>* node;
		hash_size hash = hashtbl->hashfunc(key) % hashtbl->size;

		node = hashtbl->nodes[hash];

		while (node) {
			if (compare_fn(node->key, key) == 0)
				return true;

			node = node->next;
		}

		return false;
	}

	T get(const K key)
	{
		if( hashtbl == NULL )
			return NULL;

		hashnode_s<K, T>* node;
		hash_size hash = hashtbl->hashfunc(key) % hashtbl->size;

		node = hashtbl->nodes[hash];

		while( node ) {
			if( compare_fn(node->key, key) == 0 )
				return node->data;

			node = node->next;
		}

		return NULL;
	}

	LinkedList<K>* getkeylist() {
		return keylist;
	}

	// bUseList		èáî‘ï€éù
	// bSortFirst	É\Å[Ég
	PPOSITION first(bool bUseList = false, bool bSortFirst = false) {
		if( hashtbl == NULL )
			return NULL;

		// reset
		hashtbl->pos.bUseList = false;

		if( bUseList && bEnableSort ) {
			if( !keylist || keylist->size() == 0 )
				return NULL;

			if( bSortFirst && sorted == false ) {
				hash_size n = keylist->size() - 1;
				quicksort(keylist, 0, n, orderBy);

				sorted = true;
			}

			hashtbl->pos.index = 0;
			hashtbl->pos.bUseList = bUseList;

			return &hashtbl->pos;
		}

		hash_size n;
		hashnode_s<K, T>* node;
		for(n = 0; n < hashtbl->size; ++n) {
			node = hashtbl->nodes[n];
			if( node != NULL ) {
				hashtbl->pos.index = n;
				hashtbl->pos.offset = 0;

				hashtbl->pos.bUseList = bUseList;

				return &hashtbl->pos;
			}
		}

		return NULL;
	}

	void next(PPOSITION& pos, K& key, T& data) {
		if( hashtbl == NULL || pos == NULL )
			return;

		if( pos->bUseList && bEnableSort ) {
			if( keylist == NULL || ((pos->index + 1) > keylist->size()) ) {
				pos = NULL;

				return;
			}

			key = keylist->at(pos->index);
			data = get(key);

			pos->index++;
			if( pos->index >= keylist->size()) {
				pos = NULL;

				return;
			}

			return;
		}

		hash_size index, n, off;
		hashnode_s<K, T>* node;

		index = pos->index;

		off = 0;
		node = hashtbl->nodes[index];
		while( node != NULL ) {
			if( off == pos->offset ) {
				key = node->key;
				data = node->data;

				if( node->next ) {
					pos->offset++;

					return;
				}
			}
			off++;

			node = node->next;
		}

		// pos
		for(n = index + 1; n < hashtbl->size; ++n) {
			node = hashtbl->nodes[n];
			if( node != NULL ) {
				pos->index = n;
				pos->offset = 0;

				return;
			}
		}
	
		pos = NULL;
	}

	hash_size size() {
		if( hashtbl ) {
			if( hashtbl->used < 0 )
				return 0;

			return hashtbl->used;
		}

		return 0;
	}

	void dump(void (*dump_fn)(K, T) = NULL) {
		if( hashtbl == NULL ||
			dump_fn == NULL )
			return;

		hash_size n = 0;
		struct hashnode_s<K, T> *node;

		for(n = 0; n < hashtbl->size; ++n) {
			node = hashtbl->nodes[n];
			while(node != NULL) {
				// Dump
				dump_fn(node->key, node->data);

				// Next
				node = node->next;
			}
		}
	}

protected:
	int insert(hashtable<K, T>* tbl, K key, T data, hash_size idx, short inspos) {
		if( tbl == NULL )
			return 0;

		struct hashnode_s<K, T>* node;
		hash_size hash = tbl->hashfunc(key) % tbl->size;

		node = tbl->nodes[hash];
		while( node ) {
			if( compare_fn(key, node->key) == 0 ) {
				node->data = data;

				return 1;
			}
			node = node->next;
		}

		// insert new
		if( !(node = (hashnode_s<K, T>*)malloc(sizeof(struct hashnode_s<K, T>))))
			return 0;
		memset(node, 0x00, sizeof(struct hashnode_s<K, T>));

		// set key
		node->key = dup_fn(key);

		node->data = data;
		node->next = tbl->nodes[hash];
		tbl->nodes[hash] = node;
		// Count up
		tbl->used++;

		if( bEnableSort && keylist ) {
			if( idx >= 0 )
				keylist->insert(node->key, idx, inspos);
			else
				keylist->add(node->key);

			sorted = false;
		}

		return 1;
	}

	int remove(hashtable<K, T>* tbl, K key) {
		if( tbl == NULL )
			return 0;

		hashnode_s<K, T>* node, *prevnode = NULL;
		hash_size hash = tbl->hashfunc(key) % tbl->size;

		node = tbl->nodes[hash];
		while(node) {
			if( compare_fn(node->key, key) == 0 ) {
				if (freex_fn != NULL)
					freex_fn(node->key, node->data);
				else
					free_fn(node->key, node->data);

				if( prevnode )
					prevnode->next = node->next;
				else
					tbl->nodes[hash] = node->next;

				free(node);

				if( bEnableSort ) {
					remove_key(keylist, key);
				}
				tbl->used--;
				if( tbl->used < 0 )
					tbl->used = 0;

				return 1;
			}

			prevnode = node;
			node = node->next;
		}

		return 0;
	}

	int resize(hashtable<K, T>* tbl, hash_size size) {
		if( tbl == NULL )
			return 0;

		hash_size n;
		hashnode_s<K, T> *node, *next;

		hashtable<K, T> newtbl;
		newtbl.size = size;
		newtbl.hashfunc = tbl->hashfunc;

		newtbl.nodes = (hashnode_s<K, T>**)calloc(size, sizeof(struct hashnode_s<K, T>*));
		if( newtbl.nodes == NULL )
			return 0;

		for(n = 0; n < tbl->size; ++n) {
			for(node = tbl->nodes[n]; node; node = next) {
				next = node->next;

				insert(&newtbl, node->key, node->data, false);
				remove(tbl, node->key, false);
			}
		}
		free(tbl->nodes);
		tbl->size = newtbl.size;
		tbl->nodes = newtbl.nodes;

		return 1;
	}

	void remove_key(LinkedList<K>* list, K key) {
		if( list == NULL )
			return;

		if( list->first() ) {
			do {
				K key0 = list->get();

				if( compare_fn(key0, key) == 0 ) {
					list->remove();

					return;
				}
			} while( list->next() );
		}
	}

	void quicksort(LinkedList<K>* list, hash_size left, hash_size right, short order) {
		if( list == NULL )
			return;

		hash_size i = left;
		hash_size j = right;

		// pick a mid point for the pivot
		K pivot = list->at((left + right) / 2);

		while( i <= j ) {
			while( compare_fn(list->at(i), pivot, order) < 0 )
				i++;

			while( compare_fn(list->at(j), pivot, order) > 0 )
				j--;

			if( i <= j ) {
				list->swap(i, j);

				i++;
				j--;
			}
		}

		// sort the left half using the same function recurisively
		if( left < j)
			quicksort(list, left, j, order);
		// sort the right half using the same function recursively
		if( i < right )
			quicksort(list, i, right, order);
	}

	void debug() {
		if( hashtbl == NULL )
			return;

		printf("hashtbl\n");
		long len = hashtbl->size * sizeof(struct hashnode_s<K, T>*);
		printf("hashtbl (count:%d) memory %lu\n", hashtbl->size, len);
	}

private:
	key_dup				dup_fn;
	key_compare			compare_fn;
	key_free			free_fn;

	void(*freex_fn)(K, T);

	hashtable<K, T>*	hashtbl;
	LinkedList<K>*		keylist;
	bool				bEnableSort;
	bool				sorted;

	// order
	short				orderBy;
};

struct _freeStrWP
{
	void operator()(WCHAR* pKey, WCHAR* pVal) const
	{
		if( pKey )
			free(pKey);
		if( pVal )
			free(pVal);
	}
};
typedef HashTbl<WCHAR*, WCHAR*, _freeStrWP, _compareKeyStrW, _dupKeyStrW>	CHashTblStringToStringW;
struct _freeStrAP
{
	void operator()(CHAR* pKey, CHAR* pVal) const
	{
		if( pKey )
			free(pKey);
		if( pVal )
			free(pVal);
	}
};
typedef HashTbl<CHAR*, CHAR*, _freeStrAP, _compareKeyStrA, _dupKeyStrA>		CHashTblStringToStringA;
#ifdef _UNICODE
#define CHashTblStringToString CHashTblStringToStringW
#else
#define CHashTblStringToString CHashTblStringToStringA
#endif


#endif
