#include "stdafx.h"
#include "pdll.h"

PDLL::PDLL()
	: _dllHandle(NULL)
	, _refCount(0)
	, _erroCode(0)
{
}

PDLL::~PDLL() {
	//force this to be a true uninitialize
	_refCount = 1;
	Uninitialize();
}

BOOL PDLL::Initialize(LPCTSTR dllName, LPCTSTR dllDir) {

	BOOL retVal = FALSE;

	//Add one to our internal reference counter
	_refCount++;

	if (_refCount == 1) {
		//we are assuming the name is already set
		if (NULL != dllDir) {
			::SetDllDirectory(dllDir);
		}

		_dllHandle = ::LoadLibrary(dllName);
		_erroCode = ::GetLastError();
		retVal = (_dllHandle != NULL);
	}

	return retVal;
}

//virtual 
void PDLL::Uninitialize(void) {
	//If we're already completely unintialized, early exit
	if (!_refCount)
		return;

	//if this is the last time this instance has been unitialized, 
	//then do a full uninitialization
	_refCount--;

	if (_refCount < 1) {
		if (_dllHandle != NULL) {
			::FreeLibrary(_dllHandle);
			_erroCode = ::GetLastError();

			_dllHandle = nullptr;
		}
	}
}