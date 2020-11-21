#ifndef __EVERYCALL_H__
#define __EVERYCALL_H__
#pragma once

class CiEveryCallback
{
	public:
		virtual int DoModal(LPVOID pData = NULL) const = 0;
};

template <class T>
class CiEveryCall : public CiEveryCallback
{
public:
	CiEveryCall()
	{
		pEvent = NULL;
	}

	typedef int (T::*CiEveryEvent)(LPVOID);
	virtual int DoModal(LPVOID pData) const
	{
		if( pEvent )
		{
			return (pInst->*pEvent)(pData);
		}
		return -1;
	}

	void SetCallback(T* inst, CiEveryEvent pCall)
	{
		pInst = inst;
		pEvent = pCall;
	}

private:
	T				*pInst;
	CiEveryEvent	pEvent;
};

#endif