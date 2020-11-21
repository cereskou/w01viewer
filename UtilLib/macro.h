#ifndef __MACRO_H__
#define __MACRO_H__
#pragma once

// ----------------------------------------------------------------------------
// MACRO
// ----------------------------------------------------------------------------
#ifndef _FREE
#define _FREE(p) if( p != NULL ) { free(p); p = NULL; }
#endif
#ifndef _DELETE
#define _DELETE(p) if( (p) != NULL ) { delete (p); (p) = NULL; }
#endif
#ifndef _DELETEM
#define _DELETEM(p) if( (p) ) { delete[] (p); (p) = NULL; }
#endif
#ifndef _RELEASE
#define _RELEASE(p) if( (p) ) { (p).Release(); (p) = NULL; }
#endif
#ifndef _RELEASEP
#define _RELEASEP(p) if( (p) ) { (p)->Release(); (p) = NULL; }
#endif
#ifndef _CLOSE
#define _CLOSE(p) if( (p) != NULL ) { ::CloseHandle((p)); (p) = NULL; }
#endif

#define ABS(x)	((x) * ((x) > 0 ? 1 : -1))

// ----------------------------------------------------------------------------
// DEFINE 
// ----------------------------------------------------------------------------
#define SERVER_SHUTDOWN_TIMEOUT		15000
#define THREAD_SHUTDOWN_TIMEOUT		10000

#ifndef MAKELONGLONG
#define MAKELONGLONG(a, b) ((LONGLONG)((DWORD)(a) | ((DWORDLONG)(DWORD)(b)) << 32))
#endif
#ifndef LOLONG
#define LOLONG(dwl) ((DWORD)(dwl))
#endif
#ifndef HILONG
#define HILONG(dwl) ((DWORD)(((DWORDLONG)(dwl) >> 32) & 0xFFFFFFFF)) 
#endif

#endif
