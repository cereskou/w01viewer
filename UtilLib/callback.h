#ifndef __CALLBACK_H__
#define __CALLBACK_H__
#pragma once

#include <stdlib.h>

class CEveryCallback
{
public:
	virtual INT CALL(DWORD code, DWORD result = 0, void* pData = NULL, size_t len = 0) = 0;
};

template <class T>
class CEveryCall : public CEveryCallback
{
protected:
	typedef INT (T::*CEveryEvent)(DWORD code, DWORD result, void* pData, size_t len);
	virtual INT CALL(DWORD code, DWORD result, void* pData, size_t len) {
		if (pEvent) {
			return (pInst->*pEvent)(code, result, pData, len);
		}
		return -1;
	}
public:
	CEveryCall() {
		this->pInst = NULL;
		this->pEvent = NULL;
	}

	CEveryCall(T *pInst, CEveryEvent pMethod) {
		this->pInst = pInst;
		this->pEvent = pMethod;
	}

	INT InvokeHelper(DWORD code, DWORD result, void* pData, size_t len) {
		return CALL(code, result, pData, len);
	}

	INT InvokeHelper(DWORD code, void* pData, size_t len) {
		return CALL(code, 0, pData, len);
	}

	void SetCallback(T* pInst, CEveryEvent pCall) {
		this->pInst = pInst;
		this->pEvent = pCall;
	}
	
private:
	T* pInst;
	CEveryEvent	pEvent;
};

#endif
