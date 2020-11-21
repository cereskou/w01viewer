/*
Written By: Qinghai Gou @2014
*/
#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#pragma once

#include <memory.h>

#ifdef _WIN64
typedef __int64		hash_size;
#else
typedef long		hash_size;
#endif

#define INSERT_BEFORE		0
#define INSERT_AFTER		1

#define DEFSIZE		64

// -----------------------------------------------------------------------
// free function
// -----------------------------------------------------------------------
template <typename T>
void _freeptr(T data)
{
	// for calloc()
	free(data);
}

template <typename T>
void _deleteptr(T data)
{
	// for new[]
	delete data;
}

// -----------------------------------------------------------------------
// ListNode
// -----------------------------------------------------------------------
template <typename T>
class LinkedList;

template <typename T>
class ListNode
{
	friend class LinkedList<T>;

public:
	ListNode(T);

	hash_size	index;
	T		data;
};

template <typename T>
ListNode<T>::ListNode(T in)
{
	data = in;
}

// -----------------------------------------------------------------------
// LinkedList
// -----------------------------------------------------------------------
template <typename T>
class LinkedList
{
public:
	LinkedList();
	LinkedList(bool del, void (*free_fn)(T) = _deleteptr);
	~LinkedList();

	void set_free(void (*free_fn)(T));
	void set_compare(int(*compare_fn)(const T&, const T&, short sort));

	void clear();
	bool empty();

	hash_size append(LinkedList<T>* list);

	hash_size add(T);
	hash_size insert(T in, hash_size i, short pos = INSERT_BEFORE);

	bool get_delete();
	void set_delete(void(*free_fn)(T));

	bool move(hash_size i);
	bool next();
	bool prev();
	bool first();
	void last();
	
	void swap(hash_size i, hash_size j);
	void sort(short order = 0);
	void quicksort(LinkedList<T>* list, hash_size left, hash_size right, short order);

	hash_size index();

	T remove();
	T remove(hash_size i);

	T get();
	T operator[](hash_size i);
	T at(hash_size i);
	void set(hash_size i, T in);

	hash_size size();

	unsigned long get_tag();
	void set_tag(unsigned long val);

	void* get_ext();
	void set_ext(void* val);

	void dump();
	void debug();
private:
	void (*free_fn)(T);
	int(*compare_fn)(const T&, const T&, short sort);

	bool deleteNode;
	ListNode<T> **pArray;

	// for extended
	unsigned long tag;
	void* ext;

	hash_size lstsize;
	hash_size lstpos;
	hash_size maxsize;
};

template <typename T>
LinkedList<T>::LinkedList()
{
	lstsize = 0;
	lstpos = 0;
	tag = 0;
	ext = NULL;

	deleteNode = false;
	free_fn = NULL;

	maxsize = DEFSIZE;

	pArray = (ListNode<T> **)calloc(maxsize, sizeof(ListNode<T> *));
}

template <typename T>
LinkedList<T>::LinkedList(bool del, void (*free_fn)(T))
{
	lstsize = 0;
	lstpos = 0;
	tag = 0;
	ext = NULL;

	this->free_fn = NULL;
	deleteNode = del;

	if( deleteNode )
	{
		if( free_fn != NULL )
			deleteNode = true;
		else
			deleteNode = false;

		this->free_fn = free_fn;
	}

	maxsize = DEFSIZE;

	pArray = (ListNode<T> **)calloc(maxsize, sizeof(ListNode<T> *));
}

template <typename T>
LinkedList<T>::~LinkedList()
{
	clear();

	// free extended memory block
	if (ext != NULL)
	{
		free(ext);
	}
}

template<typename T>
T LinkedList<T>::get()
{
	if( lstpos < lstsize )
	{
		ListNode<T> *ptr = pArray[lstpos];

		return ptr->data;
	}

	return NULL;
}

template<typename T>
T LinkedList<T>::operator[](hash_size i)
{
	return at(i);
}

template<typename T>
T LinkedList<T>::at(hash_size i)
{
	if( i >= 0 && i < lstsize )
	{
		lstpos = i;

		ListNode<T> *ptr = pArray[lstpos];

		return ptr->data;
	}

	return NULL;
}

template<typename T>
void LinkedList<T>::set(hash_size i, T in)
{
	if( i < lstsize )
	{
		lstpos = i;

		ListNode<T> *ptr = pArray[lstpos];

		ptr->data = in;
	}
}

template<typename T>
hash_size LinkedList<T>::index()
{
	return lstpos;
}

template<typename T>
bool LinkedList<T>::get_delete()
{
	return deleteNode;
}

template<typename T>
void LinkedList<T>::set_delete(void(*free_fn)(T))
{
	if( free_fn != NULL )
	{
		deleteNode = true;
	}
	else
	{
		deleteNode = false;
	}
	this->free_fn = free_fn;
}

template<typename T>
unsigned long LinkedList<T>::get_tag()
{
	return tag;
}

template<typename T>
void LinkedList<T>::set_tag(unsigned long val)
{
	tag = val;
}

template<typename T>
void* LinkedList<T>::get_ext()
{
	return ext;
}

template<typename T>
void LinkedList<T>::set_ext(void* val)
{
	ext = val;
}

template<typename T>
T LinkedList<T>::remove()
{
	T data = 0;

	ListNode<T> *ptr = pArray[lstpos];
	data = ptr->data;
	pArray[lstpos] = NULL;

	delete ptr;
	
	hash_size n = lstsize - lstpos - 1;
	if( n > 0 )
	{
		memmove(&pArray[lstpos], &pArray[lstpos+1], n * sizeof(ListNode<T> *));
		pArray[lstsize-1] = NULL;
	}
	lstsize--;

	// realloc
	if( lstsize < maxsize - DEFSIZE )
	{
		maxsize -= DEFSIZE;

		// realloc
		pArray = (ListNode<T> **)realloc(pArray, maxsize * sizeof(ListNode<T> *));
		if( !pArray )
			return NULL;

		size_t refill = maxsize - lstsize;
		memset(&pArray[lstsize], 0x00, sizeof(ListNode<T> *) * refill);
	}

	if( lstpos > lstsize - 1)
	{
		lstpos--;
	}

	if( lstpos < 0 )
		lstpos = 0;

	return data;
}

template<typename T>
T LinkedList<T>::remove(hash_size i)
{
	 if( move(i) )
	 {
		 return remove();
	 }

	 return 0;
}

template<typename T>
hash_size LinkedList<T>::size()
{
	return lstsize;
}

template<typename T>
void LinkedList<T>::clear()
{
	if( !empty() )
	{
		for(hash_size i = 0; i < lstsize; i++)
		{
			if( deleteNode && free_fn )
			{
				free_fn(pArray[i]->data);
			}

			delete pArray[i];

			pArray[i] = NULL;
		}
	}
	if( pArray != NULL )
	{
		free(pArray);
		pArray = NULL;

		maxsize = 0;
	}
	lstpos = 0;
	lstsize = 0;
}

template <typename T>
bool LinkedList<T>::empty()
{
	if( lstsize > 0 )
		return false;

	return true;
}

template <typename T>
hash_size LinkedList<T>::insert(T in, hash_size i, short pos)
{
	hash_size x = i;
	if( pos == INSERT_BEFORE )
		x = i;
	else
		x = i + 1;

	// Insert
	if( x < 0 )
		x = 0;

	// ’Ç‰Á
	if( x > lstsize )
	{
		return add(in);
	}

	// Block size check
	hash_size max = lstsize + 1;
	// realloc?
	if( max > maxsize )
	{
		//part++;
		maxsize += DEFSIZE;

		// realloc
		pArray = (ListNode<T> **)realloc(pArray, maxsize * sizeof(ListNode<T> *));
		if( pArray == NULL )
			return 0;

		size_t refill = maxsize - max;
		memset(&pArray[max], 0x00, sizeof(ListNode<T> *) * refill);
	}
	
	hash_size n = lstsize - x;
	// move
	if( lstsize > 0 )
	{
		memmove(&pArray[x + 1], &pArray[x], n * sizeof(ListNode<T> *));
	}

	ListNode<T> *newPtr = new ListNode<T>(in);
	if( newPtr == NULL )
		return 0;

	pArray[x] = newPtr;
	lstsize++;

	return x;
}

template <typename T>
hash_size LinkedList<T>::append(LinkedList<T>* list)
{
	if (list->first())
	{
		do
			add(list->get());
		while (list->next());
	}

	return lstsize;
}

template <typename T>
hash_size LinkedList<T>::add(T in)
{
	hash_size max = lstsize + 1;

	// realloc?
	if( max > maxsize )
	{
		//part++;
		maxsize += DEFSIZE;

		// realloc
		pArray = (ListNode<T> **)realloc(pArray, maxsize * sizeof(ListNode<T> *));
		if( pArray == NULL )
			return 0;

		size_t refill = maxsize - max;
		memset(&pArray[max], 0x00, sizeof(ListNode<T> *) * refill);
	}

	ListNode<T> *newPtr = new ListNode<T>(in);
	if( newPtr == NULL )
		return 0;

	pArray[lstsize] = newPtr;
	lstsize++;

	return lstsize;
}

template <typename T>
bool LinkedList<T>::move(hash_size i)
{
	if( i >= 0 && i < lstsize )
	{
		lstpos = i;

		return true;
	}

	return false;
}

template <typename T>
bool LinkedList<T>::next()
{
	hash_size pos = lstpos + 1;
	if( pos < lstsize )
	{
		lstpos = pos;

		return true;
	}
	lstpos = lstsize;

	return false;
}

template <typename T>
bool LinkedList<T>::prev()
{
	if (lstpos == 0)
		return false;

	lstpos--;

	return true;
}

template <typename T>
bool LinkedList<T>::first()
{
	if( lstsize > 0 )
	{
		lstpos = 0;

		return true;
	}

	return false;
}

template <typename T>
void LinkedList<T>::last()
{
	lstpos = lstsize - 1;
	if( lstpos < 0 )
		lstpos = 0;
}

template <typename T>
void LinkedList<T>::set_free(void (*free_fn)(T))
{
	this->free_fn = free_fn;
}

template <typename T>
void LinkedList<T>::set_compare(int(*compare_fn)(const T&, const T&, short sort))
{
	this->compare_fn = compare_fn;
}

template <typename T>
void LinkedList<T>::swap(hash_size i, hash_size j)
{
	if( i < 0 ||
		j < 0 ||
		i > lstsize - 1 ||
		j > lstsize - 1 )
		return;

	ListNode<T> *pi = pArray[i];
	ListNode<T> *pj = pArray[j];
	if( pi && pj )
	{
		T tmp = pi->data;
		pi->data = pj->data;
		pj->data = tmp;
	}
}

template<typename T>
void LinkedList<T>::dump()
{
	printf("lstsize:%d\n", lstsize);
	for(int i = 0; i < maxsize; i++)
	{
		printf("pArray[%d]=%d\n", i, pArray[i]);
	}
}
template<typename T>
void LinkedList<T>::debug()
{
	long len = maxsize * sizeof(ListNode<T> *);
	long nodelen = lstsize * sizeof(ListNode<T>);

	printf("LinkedList (count:%d) memory%lu\n", maxsize, len);
	printf("Node (count: %d) memory %lu\n", lstsize, nodelen);
	printf("Total memory %lu\n", (nodelen + len));
	printf("Count memory %lu\n", mem);
}

template<typename T>
void LinkedList<T>::sort(short order)
{
	hash_size n = size() - 1;
	quicksort(this, 0, n, order);
}

template<typename T>
void LinkedList<T>::quicksort(LinkedList<T>* list, hash_size left, hash_size right, short order)
{
	if (list == NULL || compare_fn == NULL)
		return;

	hash_size i = left;
	hash_size j = right;

	// pick a mid point for the pivot
	T pivot = list->at((left + right) / 2);

	while (i <= j)
	{
		while (compare_fn(list->at(i), pivot, order) < 0)
			i++;

		while (compare_fn(list->at(j), pivot, order) > 0)
			j--;

		if (i <= j)
		{
			list->swap(i, j);

			i++;
			j--;
		}
	}

	// sort the left half using the same function recurisively
	if (left < j)
		quicksort(list, left, j, order);
	// sort the right half using the same function recursively
	if (i < right)
		quicksort(list, i, right, order);

}

#endif
