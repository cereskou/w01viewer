#ifndef __THREADT_H__
#define __THREADT_H__
#define WIN32_LEAN_AND MEAN
#include <Windows.h>
#include <process.h>

// ----------------------------------------------------------------------------
// #define
// ----------------------------------------------------------------------------
#define DEFAULT_STACK_SIZE		0

//template <class T>
//__declspec(naked) DWORD WINAPI _ThreadThunk(void*) {
//	__asm {
//		pop    eax
//		pop    ecx
//		push   eax
//		xor    eax, eax
//		jmp T::Run
//	}
//}

template<typename T>
class _Thread {
public:
	// new type Method: pointer to a object's method
	typedef INT(T::* Method)(PVOID lpParam);
public:
	_Thread() {
		_object = NULL;
		_method = NULL;
		_parameter = NULL;
		_running = FALSE;
		_stacksize = DEFAULT_STACK_SIZE;
		_hThread = NULL;
		_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	_Thread(T* object, Method method, PVOID param = NULL) {
		setOwner(object);
		_method = method;
		_parameter = param;
		_running = FALSE;
		_stacksize = DEFAULT_STACK_SIZE;
		_hThread = NULL;
		_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~_Thread() {
		_CLOSE(_hEvent);
		_CLOSE(_hThread);
	}

public:
	static unsigned int __stdcall _ThreadThunk(PVOID param) {
		_Thread* cls = (_Thread*)param;
		if (cls != NULL) {
			if (cls->_running) {
				return cls->_RunSub();
			}
		}

		return 0;
	}

	BOOL AssignTask(T* object, Method method, PVOID param = NULL) {
		if (object == NULL || method == NULL) {
			return FALSE;
		}

		SetOwner(object);
		_method = method;
		_parameter = param;

		return TRUE;
	}

	BOOL Start() {
		if (_running)
			return TRUE;

		_CLOSE(_hThread);

#ifdef _ATL_MIN_CRT
#else
		_hThread = (HANDLE)_beginthreadex(NULL, _stacksize, _ThreadThunk, this, 0, &_threadId);
#endif
		if (_hThread == NULL)
			return FALSE;

		_running = TRUE;

		return TRUE;
	}

	void Stop(int timeout = INFINITE) {
		_running = FALSE;
		if (_hThread != NULL) {
			::SetEvent(_hEvent);

			if (::WaitForSingleObject(_hThread, timeout) == WAIT_TIMEOUT) {
				::TerminateThread(_hThread, 1);
			}

			_CLOSE(_hThread);
		}
	}

	BOOL Resume() {
		return ::ResumeThread(_hThread);
	}

	DWORD Suspend() {
		return ::SuspendThread(_hThread);
	}

	BOOL Terminate(DWORD dwExitCode = 0) {
		return ::TerminateThread(_hThread, dwExitCode);
	}

	void Exit(DWORD dwExitCode = 0) {
#ifdef _ATL_MIN_CRT
		::ExitThread(dwExitCode);
#else
		_endthreadex(dwExitCode);
#endif
	}

	DWORD Join(DWORD dwTimeOut = INFINITE) const {
		return ::WaitForSingleObject(_hThread, dwTimeOut);
	}

	BOOL IsIOPending() const {
		BOOL bPending = FALSE;

		::GetThreadIOPendingFlag(_hThread, &bPending);

		return bPending;
	}

	void SetPriority(INT priority) {
		::SetThreadPriority(_hThread, priority);
	}

	INT GetPriority() {
		return ::GetThreadPriority(_hThread);
	}

	DWORD GetExitCode() const {
		DWORD dwExitCode = 0;
		if (::GetExitCodeThread(_hThread, &dwExitCode))
			return dwExitCode;
		else
			return (DWORD)-1;
	}

	BOOL GetThreadTimes(LPFILETIME pcTm, LPFILETIME peTm, LPFILETIME pkTm, LPFILETIME puTm) const {
		// pcTm - CreationTime
		// peTm - ExitTime
		// pkTm - KernelTime
		// puTm - UserTime
		return ::GetThreadTimes(_hThread, pcTm, peTm, pkTm, puTm);
	}

	
	BOOL isRunning() {
		return _running;
	}

	HANDLE& GetTerminateEvent() const {
		return _hEvent;
	}

	// Getter & Setter
	__declspec(property(get = getThreadHandle)) HANDLE ThreadHandle;
	inline HANDLE getThreadHandle() {
		return _hThread;
	}

	__declspec(property(get = getThreadID)) DWORD ThreadID;
	inline DWORD getThreadID() {
		return _threadId;
	}
protected:
	virtual INT Run() {
		return 0;
	}
	virtual INT Init() {
		return 0;
	}
	virtual INT Release() {
		return 0;
	}

	INT _RunSub() {
		INT ret = 0;
		ret = Init();
		if (ret < 0) {
			//
			return ret;
		}

		ret = Run();
		if (ret < 0) {
			//
		}

		ret = Release();

		return ret;
	}

private:
	void SetOwner(T* object) {
		::InterlockedExchangePointer(reinterpret_cast<PVOID*>(&_object), object);
	}

protected:
	T*		_object;
	Method	_method;
	PVOID	_parameter;
	HANDLE	_hThread;
	HANDLE	_hEvent;		// terminate event

	UINT	_threadId;
	INT		_stacksize;
	BOOL	_running;
};
//// class _Thread
//template<class T>
//class _Thread {
//	// new type Method: pointer to a object's method
//	typedef INT (T::* Method)(void);
//
//protected:
//	HANDLE hThread;
//
//private:
//	DWORD	threadID;
//	T*		object;
//	Method	method;
//	HANDLE	hInterrupt;
//	HANDLE	hSingleStart;
//	BOOL	_stop;
//
//private:
//	static DWORD run(LPVOID thread_obj) {
//		_Thread<T>* thread = (_Thread<T>*)thread_obj;
//		return (thread->object->*thread->method)();
//	}
//
//	_Thread(const _Thread<T>& other) {}
//	_Thread<T>& operator=(const _Thread<T>& other) {}
//
//public:
//	explicit _Thread(T* object, INT(T::* method)(void)) {
//		this->hThread		= NULL;
//		this->object		= object;
//		this->method		= method;
//		this->threadID		= 0;
//		this->hInterrupt	= ::CreateSemaphore(NULL, 1, 1, NULL);
//		this->hSingleStart	= ::CreateMutex(NULL, FALSE, NULL);
//		this->_stop = FALSE;
//	}
//
//	~_Thread(void) {
//		if( hInterrupt )
//			::CloseHandle(hInterrupt);
//		if( hThread )
//			::CloseHandle(hThread);
//	}
//
//	BOOL IsStop() {
//		return this->_stop;
//	}
//
//	void stop() {
//		this->_stop = TRUE;
//	}
//
//	BOOL start() {
//		__try {
//			if(::WaitForSingleObject(hSingleStart, 0) != WAIT_OBJECT_0)
//				return FALSE;
//
//			if( hThread ) {
//				if(::WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT)
//					return FALSE;
//
//				::CloseHandle(hThread);
//			}
//
//			::WaitForSingleObject(hInterrupt, 0);
//
//			hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_Thread<T>::run, this, 0, &this->threadID);
//			if(hThread)
//				return TRUE;
//
//			return FALSE;
//		}
//		__finally {
//			::ReleaseMutex(hSingleStart);
//		}
//	}
//
//	inline void join() {
//		::WaitForSingleObject(hThread, INFINITE);
//	}
//
//	inline BOOL interrupt() {
//		if( hInterrupt ) {
//			return ((::ReleaseSemaphore(hInterrupt, 1, NULL) == FALSE) ? FALSE : TRUE);
//		}
//		return FALSE;
//	}
//
//	inline BOOL isInterrupted() {
//		return this->isInterrupted(0);
//	}
//
//	inline BOOL isInterrupted(DWORD millisec) {
//		if(::WaitForSingleObject(hInterrupt, millisec) == WAIT_TIMEOUT)
//			return FALSE;
//
//		::ReleaseSemaphore(hInterrupt, 1, NULL);
//		return TRUE;
//	}
//
//	inline BOOL isRunning() {
//		DWORD exitCode = 0;
//		if( hThread )
//			::GetExitCodeThread(hThread, &exitCode);
//
//		if( exitCode == STILL_ACTIVE)
//			return TRUE;
//
//		return FALSE;
//	}
//
//	// Getter & Setter
//	__declspec(property(get = getThreadHandle)) HANDLE ThreadHandle;
//	inline HANDLE getThreadHandle() {
//		return hThread;
//	}
//
//	__declspec(property(get = getThreadID)) DWORD ThreadID;
//	inline DWORD getThreadID() {
//		return threadID;
//	}
//};

#endif
