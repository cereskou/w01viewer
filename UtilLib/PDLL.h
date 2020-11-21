////////////////////////////////////////////////////////////////////////
//     Class: PDll     
//
// Authors: MicHael Galkovsky // Date: April 14, 1998 
// Company: Pervasive Software 
// Purpose: Base class to wrap dynamic use of dll 
//
// https://msdn.microsoft.com/en-us/library/ms810279.aspx
////////////////////////////////////////////////////////////////////////
#ifndef __PDLL_H__
#define __PDLL_H__
#pragma once

#include <stdio.h>
#include <windows.h>

#define FUNC_LOADED 3456

#define DECLARE_FUNCTION0(convention, retVal, FuncName) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName; \
  retVal FuncName() { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	        }\
      if (NULL != m_##FuncName) \
        return m_##FuncName(); \
	        else \
        return (retVal)NULL; \
	    } else \
      return (retVal)NULL; \
    }

// FUNCTION with 1 params
#define DECLARE_FUNCTION1(convention, retVal, FuncName, Param1) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1) { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	        }\
      if (NULL != m_##FuncName) \
        return m_##FuncName(p1); \
	        else \
        return (retVal)NULL; \
	    } else \
      return (retVal)NULL; \
    }

// FUNCTION with 2 params
#define DECLARE_FUNCTION2(convention, retVal, FuncName, Param1, Param2) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1, Param2); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1, Param2 p2) { \
     if (_dllHandle) { \
       if (FUNC_LOADED != m_is##FuncName) {\
         m_##FuncName = NULL; \
         m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
         m_is##FuncName = FUNC_LOADED;\
	          }\
       if (NULL != m_##FuncName) \
         return m_##FuncName(p1, p2); \
	          else \
         return (retVal)NULL; \
	      } else \
       return (retVal)NULL; \
    }

// FUNCTION with 3 params
#define DECLARE_FUNCTION3(convention, retVal, FuncName, Param1, Param2, Param3) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1, Param2, Param3); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1, Param2 p2, Param3 p3) { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	        }\
      if (NULL != m_##FuncName) \
        return m_##FuncName(p1, p2, p3); \
	        else \
        return (retVal)NULL; \
	    } else \
      return (retVal)NULL; \
    }

// FUNCTION with 4 params
#define DECLARE_FUNCTION4(convention, retVal, FuncName, Param1, Param2, Param3, Param4) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1, Param2, Param3, Param4); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1, Param2 p2, Param3 p3, Param4 p4) { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	        }\
      if (NULL != m_##FuncName) {\
        return m_##FuncName(p1, p2, p3, p4); \
	  	       } else \
        return (retVal)NULL; \
	    } else \
      return (retVal)NULL; \
    }

// FUNCTION with 5 params
#define DECLARE_FUNCTION5(convention, retVal, FuncName, Param1, Param2, Param3, Param4, Param5) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1, Param2, Param3, Param4, Param5); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	        }\
      if (NULL != m_##FuncName) \
        return m_##FuncName(p1, p2, p3, p4, p5); \
	        else \
        return (retVal)NULL; \
	    } else \
      return (retVal)NULL; \
    }

// FUNCTION with 6 params
#define DECLARE_FUNCTION6(convention, retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6) \
public: \
  typedef retVal (convention* TYPE_##FuncName)(Param1, Param2, Param3, Param4, Param5, Param6); \
  TYPE_##FuncName m_##FuncName; \
  short m_is##FuncName;\
  retVal FuncName(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) { \
    if (_dllHandle) { \
      if (FUNC_LOADED != m_is##FuncName) {\
        m_##FuncName = NULL; \
        m_##FuncName = (TYPE_##FuncName)::GetProcAddress(_dllHandle, #FuncName); \
        m_is##FuncName = FUNC_LOADED;\
	  }\
      if (NULL != m_##FuncName) \
        return m_##FuncName(p1, p2, p3, p4, p5, p6); \
	        else \
        return (retVal)NULL; \
	  } else \
        return (retVal)NULL; \
    }

// declare constructors and Load
#define DECLARE_CLASS(ClassName) \
	public: \
    ClassName(LPCTSTR pszName) {Initialize(pszName);} \
    ClassName() {PDLL();}

class PDLL {
public:
	PDLL();
	virtual ~PDLL();

	virtual BOOL Initialize(LPCTSTR dllName, LPCTSTR dllDir = NULL);
	virtual void Uninitialize(void);

protected:
	HINSTANCE	_dllHandle;
	int			_refCount;
	DWORD		_erroCode;
}; // PDLL

#endif
