/*
class CStringT MFC-Like
Written by: Qinghai Gou
*/
#ifndef __CSTRING_H__
#define __CSTRING_H__
#pragma once
#include <windows.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

namespace UTL {

template<typename T> struct traits;
// ------------------------------------------------------------------
// function for char
// ------------------------------------------------------------------
template<> struct traits<char>
{
	// 殻文字
	operator char*()
	{
		return "";
	}
	static char Null() {
		return '\0';
	}
	// MakeUpper
	static void MakeUpper(char* buffer) {
		::CharUpperA(buffer);
	}
	// MakeLower
	static void MakeLower(char* buffer) {
		::CharLowerA(buffer);
	}
	// startsWith
	static bool startsWith(const char* buffer, const char* suffix) {
		return GetLength(buffer) > GetLength(suffix) &&
			strncmp(buffer, suffix, GetLength(suffix)) == 0;
	}
	// endsWith
	static bool endsWith(const char* buffer, const char* suffix) {
		return GetLength(buffer) > GetLength(suffix) &&
			strncmp(buffer + GetLength(buffer) - GetLength(suffix), suffix, GetLength(suffix)) == 0;
	}

	// Format
	static int vsnprintf(char* buffer, size_t sizeOfBuffer, const char* format, va_list argptr) {
		return vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr);
	}
	// 文字列の長さを取得します。
	static size_t GetLength(const char* psz)
	{
		if (!psz) return 0;
		return strlen(psz);
	}
	// 整数(int)を文字列に変換します。  
	static void FormatInt(int i, char* pBuffer)
	{
		_itoa_s(i, pBuffer, 20, 10);
	}
	// 整数(long)を文字列に変換します。  
	static void FormatInt(long l, char* pBuffer)
	{
		_ltoa_s(l, pBuffer, 20, 10);
	}
	// 整数(unsigned long)を文字列に変換します。  
	static void FormatInt(unsigned long ul, char* pBuffer)
	{
		_ultoa_s(ul, pBuffer, 20, 10);
	}
	// 整数が空白文字を表すかどうかを決定します。 
	static int space(char ch)
	{
		return isspace(ch);
	}
	// 文字列で最初に見つかった検索文字列へのポインターを返します。 
	static const char* Find(const char* pBuffer, const char* pSub)
	{
		return strstr(pBuffer, pSub);
	}
	// 文字列で最後に見つかった検索文字列へのポインターを返します。 
	static char* ReserveFind(const char* pBuffer, const char* pSub)
	{
		if (*pSub == '\0')
			return (char*)pBuffer;
		char* result = NULL;
		for (;;)
		{
			char* p = (char *)strstr(pBuffer, pSub);
			if (p == NULL)
				break;
			result = p;

			pBuffer = p + 1;
		}

		return result;
	}
	// 左側の文字列が右側の文字列より小さいかどうかを判定します。 
	static bool Less(const char* pStr1, const char* pStr2)
	{
		int result = strcmp(pStr1, pStr2);
		if (result < 0)
			return true;

		return false;
	}
	// 左側の文字列が右側の文字列より大きいかどうかを判定します。 
	static bool Greater(const char* pStr1, const char* pStr2)
	{
		int result = strcmp(pStr1, pStr2);
		if (result > 0)
			return true;

		return false;
	}
	// SpanIncluding
	static size_t SpanIncluding(const char* pStr1, const char* pStr2) 
	{
		return (size_t)strspn(pStr1, pStr2);
	}
	// isdigit
	static bool IsDigit(const char* pStr)
	{
		const char* p = pStr;
		while (*p) {
			if (isdigit(*p) == 0)
				return false;
			p++;
		}

		return true;
	}
};

// ------------------------------------------------------------------
// function for wchar_t
// ------------------------------------------------------------------
template<> struct traits<wchar_t>
{
	// 殻文字
	operator wchar_t*()
	{
		return L"";
	}
	static wchar_t Null() {
		return L'\0';
	}
	// MakeUpper
	static void MakeUpper(wchar_t* buffer) {
		::CharUpperW(buffer);
	}
	// MakeLower
	static void MakeLower(wchar_t* buffer) {
		::CharLowerW(buffer);
	}
	// startsWith
	static bool startsWith(const wchar_t* buffer, const wchar_t* suffix) {
		return GetLength(buffer) > GetLength(suffix) &&
			wcsncmp(buffer, suffix, GetLength(suffix)) == 0;
	}
	// endsWith
	static bool endsWith(const wchar_t* buffer, const wchar_t* suffix) {
		return GetLength(buffer) > GetLength(suffix) &&
			wcsncmp(buffer + GetLength(buffer) - GetLength(suffix), suffix, GetLength(suffix)) == 0;
	}

	// Format
	static int vsnprintf(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, va_list argptr) {
		return _vsnwprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr);
	}
	// 文字列の長さを取得します。
	static size_t GetLength(const wchar_t* psz)
	{
		if (!psz) return 0;
		return wcslen(psz);
	}
	// 整数(int)を文字列に変換します。  
	static void FormatInt(int i, wchar_t* pBuffer)
	{
		_itow_s(i, pBuffer, 20, 10);
	}
	// 整数(long)を文字列に変換します。  
	static void FormatInt(long l, wchar_t* pBuffer)
	{
		_ltow_s(l, pBuffer, 20, 10);
	}
	// 整数(unsigned long)を文字列に変換します。  
	static void FormatInt(unsigned long ul, wchar_t* pBuffer)
	{
		_ultow_s(ul, pBuffer, 20, 10);
	}
	// 整数が空白文字を表すかどうかを決定します。 
	static int space(wchar_t ch)
	{
		return iswspace(ch);
	}
	// 文字列で最初に見つかった検索文字列へのポインターを返します。 
	static const wchar_t* Find(const wchar_t* pBuffer, const wchar_t* pSub)
	{
		return wcsstr(pBuffer, pSub);
	}
	// 文字列で最後に見つかった検索文字列へのポインターを返します。 
	static wchar_t* ReserveFind(const wchar_t* pBuffer, const wchar_t* pSub)
	{
		if (*pSub == L'\0')
			return (wchar_t*)pBuffer;
		wchar_t* result = NULL;
		for (;;)
		{
			wchar_t* p = (wchar_t *)wcsstr(pBuffer, pSub);
			if (p == NULL)
				break;
			result = p;

			pBuffer = p + 1;
		}

		return result;
	}
	// 左側の文字列が右側の文字列より小さいかどうかを判定します。 
	static bool Less(const wchar_t* pStr1, const wchar_t* pStr2)
	{
		int result = wcscmp(pStr1, pStr2);
		if (result < 0)
			return true;

		return false;
	}
	// 左側の文字列が右側の文字列より大きいかどうかを判定します。 
	static bool Greater(const wchar_t* pStr1, const wchar_t* pStr2)
	{
		int result = wcscmp(pStr1, pStr2);
		if (result > 0)
			return true;

		return false;
	}
	// SpanIncluding
	static size_t SpanIncluding(const wchar_t* pStr1, const wchar_t* pStr2)
	{
		return (size_t)wcsspn(pStr1, pStr2);
	}
	// isdigit
	static bool IsDigit(const wchar_t* pStr)
	{
		const wchar_t* p = pStr;
		while (*p) {
			if (iswdigit(*p) == 0)
				return false;
			p++;
		}

		return true;
	}
};

// ----------------------------------------------------------------------------
// class CStringT MFC-like
// ----------------------------------------------------------------------------
template<class Type>
class CStringT
{
public:
	// コンストラクター
	CStringT() {
		Init();

		*this = traits<Type>();
	}

	// Copy constructor
	CStringT(const CStringT& str) {
		Init();

		*this = str;
	}

	// from a single character
	CStringT(Type ch, int nRepeat = 1) {
		Init();

		*this = traits<Type>();
		while (nRepeat--) *this += ch;
	}

	// from ANSI string
	CStringT(const Type* pszStr) {
		Init();

		*this = pszStr;
	}

	// from integer
	CStringT(int i) {
		Init();

		*this = i;
	}

	// from long
	CStringT(long l)
	{
		Init();

		*this = l;
	}

	// from long
	CStringT(unsigned ul) {
		Init();

		*this = ul;
	}

	size_t GetLength() const {
		return nDataLength;
	}

	bool IsEmpty() const {
		return (nDataLength == 0 || m_pchData == NULL);
	}

	// clear contents to empty
	void Empty()
	{
		*this = traits<Type>();
	}

	// return single character at zero-based index
	Type GetAt(size_t nIndex) const {
		if ((nIndex >= 0) && (nIndex < GetLength()))
			return m_pchData[nIndex];
		else
			return 0;
	}

	// set a single character at zero-based index
	void SetAt(size_t nIndex, Type ch)
	{
		if ((nIndex >= 0) && (nIndex < GetLength()))
			m_pchData[nIndex] = ch;
	}

	// overloaded operator helpers
	// return single character at zero-based index
	Type operator[](INT_PTR nIndex) const {
		return (Type)GetAt(nIndex);
	}

	Type& operator[](INT_PTR nIndex) {
		return (Type&)m_pchData[nIndex];
	}

	// copy from another CStringT
	const CStringT& operator=(const CStringT& str)
	{
		if (&str == NULL) return *this;

		size_t nLength = ((CStringT&)str).GetLength();

		SetAllocSize(nLength + 1, sizeof(Type));
		SetLength(nLength);

		CopyChars(m_pchData, nLength, ((CStringT&)str).GetBuffer(), nLength);

		return *this;
	}

	// set string content to single character
	const CStringT& operator=(Type ch)
	{
		SetAllocSize(1, sizeof(Type));
		SetLength(1);

		CopyChars(m_pchData, nLength, &ch, nLength);

		return *this;
	}

	// copy string content from ANSI string
	const CStringT& operator=(const Type* pszStr)
	{
		if (!pszStr) return *this;

		size_t nLength = traits<Type>::GetLength(pszStr);

		SetAllocSize(nLength + 1, sizeof(Type));
		SetLength(nLength);

		CopyChars(m_pchData, nLength + 1, (void*)pszStr, nLength);

		return *this;
	}

	// copy string content from integer
	const CStringT& operator=(int i)
	{
		SetAllocSize(20, sizeof(Type));
		traits<Type>::FormatInt(i, m_pchData);
		size_t nLength = traits<Type>::GetLength(m_pchData);
		SetLength(nLength);

		return *this;
	}

	// copy string content from long
	const CStringT& operator=(long l)
	{
		SetAllocSize(20, sizeof(Type));
		traits<Type>::FormatInt(l, m_pchData);
		size_t nLength = traits<Type>::GetLength(m_pchData);
		SetLength(nLength);

		return *this;
	}

	// copy string content from long
	const CStringT& operator=(unsigned long ul)
	{
		SetAllocSize(20, sizeof(Type));
		traits<Type>::FormatInt(ul, m_pchData);
		size_t nLength = traits<Type>::GetLength(m_pchData);
		SetLength(nLength);

		return *this;
	}

	// return pointer to const string
	operator const Type*() {
		return this->GetBuffer();
	}

	// Concatenation
	const CStringT& operator+=(const CStringT& str)
	{
		if (&str == NULL) return *this;
		size_t nLength = ((CStringT&)str).GetLength();
		size_t nOldLength = GetLength();

		size_t nNewLength = nOldLength + nLength;

		SetAllocSize(nNewLength + 1, sizeof(Type));
		SetLength(nNewLength);

		CopyChars(m_pchData + nOldLength, nLength, ((CStringT&)str).GetBuffer(), nLength);


		return *this;
	}

	// concatenate a single character
	const CStringT& operator +=(Type ch)	{
		size_t nOldLength = GetLength();

		int nNewLength = (int)(nOldLength + 1);

		SetAllocSize(nNewLength + 1, sizeof(Type));
		SetLength(nNewLength);

		CopyChars(m_pchData + nOldLength, 1, &ch, 1);

		return *this;
	}

	friend CStringT __stdcall operator+(const CStringT& str1, const CStringT& str2)	{
		CStringT s;
		s = str1;
		s += str2;

		return s;
	}

	friend CStringT __stdcall operator+(const CStringT& str, Type ch) {
		CStringT s;
		s = str;
		s += ch;

		return s;
	}

	friend CStringT __stdcall operator+(const CStringT& str, const Type* lpsz) {
		CStringT s;
		s = str;
		s += lpsz;

		return s;
	}

	friend CStringT __stdcall operator+(const Type* lpsz, const CStringT& str) {
		CStringT s;
		s = lpsz;
		s += str;

		return s;
	}

	// string comparison
	int Compare(const Type* pszStr)
	{
		if (!pszStr)
			return 0;

		size_t nLengthM = GetLength();
		size_t nLengthY = traits<Type>::GetLength(pszStr);

		size_t nLength = (nLengthM < nLengthY) ? nLengthM : nLengthY;
		int nResult = memcmp(m_pchData, pszStr, nLength);
		if (nResult)
			return nResult;

		if (nLengthM == nLengthY)
			return 0;
		if (nLengthM < nLengthY)
			return -1;
		if (nLengthM > nLengthY)
			return 1;

		return 0;
	}

	// string comparison no case
	int CompareNoCase(const Type* pszStr)
	{
		if (!pszStr)
			return 0;

		size_t nLengthM = GetLength();
		size_t nLengthY = traits<Type>::GetLength(pszStr);

		size_t nLength = (nLengthM < nLengthY) ? nLengthM : nLengthY;
		int nResult = _memicmp(m_pchData, pszStr, nLength);
		if (nResult)
			return nResult;

		if (nLengthM == nLengthY)
			return 0;
		if (nLengthM < nLengthY)
			return -1;
		if (nLengthM > nLengthY)
			return 1;

		return 0;
	}

	bool IsDigit() const
	{
		return (bool)traits<Type>::IsDigit(m_pchData);
	}

	int SpanIncluding(const Type* pszCharSet) const
	{
		return traits<Type>::SpanIncluding(GetBuffer(), pszCharSet);
	}

	bool operator==(const Type* pszStr)
	{
		return !(this->Compare(pszStr));
	}

	bool operator!=(const Type* pszStr)
	{
		return (this->Compare(pszStr));
	}

	friend bool operator<(const CStringT& str1, const CStringT& str2)
	{
		return traits<Type>::Less(str1, str2);
	}

	friend bool operator>(const CStringT& str1, const CStringT& str2)
	{
		return traits<Type>::Greater(str1, str2);
	}

	// search
	int ReverseFind(const Type* pszSub)
	{
		int pos = -1;
		if (!pszSub) return -1;
		Type* pch = traits<Type>::ReserveFind(m_pchData, pszSub);
		if (pch != NULL)
			pos = (pch - m_pchData);

		return pos;
	}

	int Find(const Type* pszSub, int nStart)
	{
		if (!pszSub) return -1;

		int pos = -1;
		const Type* pch = traits<Type>::Find(m_pchData, pszSub);
		if (pch != NULL)
			pos = (int)(pch - m_pchData);

		return pos;
	}

	int Find(const Type* pszSub)
	{
		return Find(pszSub, 0);
	}

	// Format
	bool __cdecl Format(const Type* pszFormat, ...) {
		va_list list;
		va_start(list, pszFormat);

		// Format
		int count = traits<Type>::vsnprintf(GetBufferSetLength(65535), 65535, pszFormat, list);

		va_end(list);

		ReleaseBuffer();

		return (count != -1);
	}

	// cut
	CStringT Mid(int nFirst, int nCount)
	{
		if (nFirst < 0) nFirst = 0;
		if (nCount < 0) nCount = 0;

		size_t len = GetLength();
		if ((size_t)(nFirst + nCount) > len)
			nCount = (int)(len - nFirst);
		if ((size_t)nFirst > len)
			nCount = 0;

		if (nFirst == 0 && (size_t)(nFirst + nCount) == len)
			return *this;

		CStringT dest;
		dest = (GetBuffer() + nFirst);
		dest.SetLength(nCount);

		return dest;
	}

	CStringT Mid(int nFirst)
	{
		return Mid(nFirst, (int)(GetLength() - nFirst));
	}

	CStringT Left(int nCount) const {
		if (nCount < 0)
			nCount = 0;
		else if (nCount > GetLength())
			nCount = GetLength();

		CStringT dest;
		AllocCopy(dest, nCount, 0, 0);

		return dest;
	}

	CStringT Right(int nCount) const {
		if (nCount < 0)
			nCount = 0;
		else if ((size_t)nCount > GetLength())
			nCount = (int)GetLength();

		CStringT dest;
		AllocCopy(dest, nCount, GetLength() - nCount, 0);

		return dest;
	}

	// Trim
	void TrimRight()
	{
		int pos = (int)(GetLength() - 1);
		while (traits<Type>::space(GetAt(pos)) && (pos >= 0))
			pos--;
		pos++;
		SetLength(pos);
	}

	void TrimLeft()
	{
		int pos = 0;
		int len = (int)GetLength();
		while (traits<Type>::space(GetAt(pos)) && (pos < len))
			pos++;

		*this = Mid(pos);
	}

	void Trim()
	{
		TrimLeft();
		TrimRight();
	}

	Type* GetBuffer(int nMinBufferLength = -1)
	{
		if (nMinBufferLength != -1) {
			if (!SetAllocSize(nMinBufferLength + 1, sizeof(Type)))
				return NULL;
		}

		return (Type*)m_pchData;
	}

	void ReleaseBuffer(int nNewLength = -1)
	{
		if (nNewLength != -1)
			SetLength(nNewLength);
		else
			SetLength(traits<Type>::GetLength((Type*)m_pchData));

		SetAllocSize(GetLength() + 1, sizeof(Type));
	}

	// 文字の置き換え
	int Replace(Type chOld, Type chNew) {
		int nCount = 0;
		if (chOld != chNew) {
			Type* p = m_pchData;
			Type* e = p + GetLength();
			while (p < e) {
				if (*p == chOld) {
					*p = chNew;
					nCount++;
				}
				p++;
			}
		}
		return nCount;
	}

	// 文字の置き換え
	int Replace(const Type* pszOld, const Type* pszNew)	{
		if (!pszOld || !pszNew)
			return -1;

		size_t nSrcLen = traits<Type>::GetLength(pszOld);
		if (nSrcLen == 0)
			return 0;
		size_t nRplLen = traits<Type>::GetLength(pszNew);

		int nCount = 0;
		Type* pchS = m_pchData;
		Type* pchE = pchS + GetLength();
		Type* pchT = NULL;

		while (pchS < pchE) {
			while ((pchT = (Type*)traits<Type>::Find(pchS, pszOld)) != NULL) {
				nCount++;
				pchS = pchT + nSrcLen;
			}
			pchS += traits<Type>::GetLength(pchS) + 1;
		}

		if (nCount > 0) {
			size_t nOldLen = GetLength();
			size_t nNewLen = nOldLen + (nRplLen - nSrcLen) * nCount;
			Type* pBuf = NULL;
			bool bFree = false;
			if (nOldLen != nNewLen) {
				pBuf = (Type*)malloc(GetAllocSize());
				memcpy_s(pBuf, GetAllocSize(), m_pchData, GetAllocSize());

				bFree = true;
				// Alloc 
				SetAllocSize(nNewLen + 1, sizeof(Type));
				SetLength(nNewLen);
			}
			else {
				pBuf = m_pchData;
				bFree = false;
			}

			size_t offset = 0;
			pchS = pBuf;
			pchE = pchS + nOldLen;
			while (pchS < pchE) {
				while ((pchT = (Type*)traits<Type>::Find(pchS, pszOld)) != NULL) {
					if (pchT > pchS) {
						memcpy_s(m_pchData + offset, GetAllocSize(), pchS, (pchT - pchS) * sizeof(Type));
						offset += (pchT - pchS);
					}
					memcpy_s(m_pchData + offset, GetAllocSize(), pszNew, nRplLen * sizeof(Type));
					offset += nRplLen;
					pchS = pchT + nSrcLen;
				}
				if (pchS) {
					memcpy_s(m_pchData + offset, GetAllocSize(), pchS, traits<Type>::GetLength(pchS) * sizeof(Type));
				}

				pchS += traits<Type>::GetLength(pchS) + 1;
			}

			// Free
			if (pBuf != NULL && bFree) {
				free(pBuf);
			}
		}
		return nCount;
	}

	//
	Type* GetBufferSetLength(int nNewLength) {
		if (nNewLength < 0)
			return NULL;

		if (GetBuffer(nNewLength) == NULL)
			return NULL;

		// Set Length
		SetLength(nNewLength);
		m_pchData[nNewLength] = traits<Type>::Null();

		return m_pchData;
	}

	void MakeUpper() {
		traits<Type>::MakeUpper(m_pchData);
	}

	void MakeLower() {
		traits<Type>::MakeLower(m_pchData);
	}

	// Begin with
	bool startsWith(const Type* suffix) {
		return traits<Type>::startsWith(GetBuffer(), suffix);
	}

	bool endsWith(const Type* suffix) {
		return traits<Type>::endsWith(GetBuffer(), suffix);
	}

protected:
	bool AllocCopy(CStringT& dest, size_t nCopyLen, size_t nCopyIndex, size_t nExtraLen) const {
		bool bRet = false;
		size_t nNewLen = nCopyLen + nExtraLen;
		if (nNewLen == 0) {
			dest.Init();
			bRet = true;
		}
		else if (nNewLen >= nCopyLen) {
			dest.SetAllocSize(nNewLen + 1, sizeof(Type));
			dest.SetLength(nNewLen);

			dest.CopyChars(dest.m_pchData, (nNewLen + 1), m_pchData + nCopyIndex, nCopyLen);
			bRet = true;
		}
		return bRet;
	}

	void CopyChars(void* pchDest, size_t nDestLen, void* pchSrc, size_t nChars) {
		memcpy_s(pchDest, nDestLen * sizeof(Type), pchSrc, nChars * sizeof(Type));
	}

	void CopyChars(void* pchDest, const void* pchSrc, size_t nChars) {
		if (pchSrc != NULL) {
			memcpy(pchDest, pchSrc, nChars * sizeof(Type));
		}
	}

	void SetLength(size_t nLength)
	{
		if ((nLength * sizeof(Type)) > nAllocLength) return;
		if (!m_pchData) return;

		m_pchData[nLength] = traits<Type>::Null();
		nDataLength = nLength;
	}

	bool SetAllocSize(size_t nChars, size_t nCharSize)
	{
		if (!m_pchData)
		{
			m_pchData = (Type *)malloc(nChars * nCharSize);
			if (m_pchData == NULL)
				return false;

			nAllocLength = nChars * nCharSize;
		}

		if (GetAllocSize() != nChars * nCharSize)
		{
			Type* p = m_pchData;

			// alloc new memory
			m_pchData = (Type *)realloc(m_pchData, nChars * nCharSize);
			if (m_pchData == NULL) {
				m_pchData = p;
				return false;
			}
			nAllocLength = nChars * nCharSize;
		}

		return true;
	}

public:
	// destructor
	~CStringT() {
		if (m_pchData) {
			free(m_pchData);
			m_pchData = NULL;
		}
	}

	size_t GetAllocSize()
	{
		return nAllocLength;
	}

private:
	Type*	m_pchData;
	size_t	nDataLength;
	size_t	nAllocLength;

	void Init() {
		m_pchData = NULL;
		nDataLength = 0;
		nAllocLength = 0;
	}
};

typedef CStringT<char>		CStringA;
typedef CStringT<wchar_t>	CStringW;

#ifdef  UNICODE
#define CString		CStringW
#ifndef __T
#define __T(x)      L ## x
#endif
#else
#define CString		CStringA
#ifndef __T
#define __T(x)      x
#endif
#endif

#ifndef _T
#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)
#endif

// ----------------------------------------------------------------------------
// inline functions
// ----------------------------------------------------------------------------
inline CStringA _CW2A(CStringW pszW, UINT acp = CP_ACP) {
	int len = ::WideCharToMultiByte(acp,
		0,
		pszW.GetBuffer(),
		-1,
		NULL,
		0,
		NULL,
		NULL);

	CStringA destA;
	int ret = ::WideCharToMultiByte(acp,
		0,
		pszW.GetBuffer(),
		-1,
		destA.GetBufferSetLength(len),
		len,
		NULL,
		NULL);

	if (ret == 0)
		return NULL;
	destA.ReleaseBuffer();

	return destA;
}

inline CStringW _CA2W(CStringA pszA, UINT acp = CP_ACP)
{
	int len = ::MultiByteToWideChar(acp,
		0,
		pszA.GetBuffer(),
		-1,
		NULL,
		0);
	CStringW destW;

	int ret = ::MultiByteToWideChar(acp,
		0,
		pszA.GetBuffer(),
		-1,
		destW.GetBufferSetLength(len),
		len);

	if (ret == 0)
		return NULL;
	destW.ReleaseBuffer();

	return destW;
}

inline BOOL _IsEmptyA(LPCSTR lpszA) {
	if (lpszA == NULL)
		return TRUE;
	if (strlen(lpszA) == 0)
		return TRUE;

	return FALSE;
}

inline BOOL _IsEmptyW(LPCWSTR lpszW) {
	if (lpszW == NULL)
		return TRUE;
	if (wcslen(lpszW) == 0)
		return TRUE;

	return FALSE;
}

} // namespace UTL

// ----------------------------------------------------------------------------
// define
// ----------------------------------------------------------------------------
#ifdef  UNICODE
#define _IsEmpty	UTL::_IsEmptyW
#define _W2A(x)		(LPCSTR)UTL::_CW2A((LPCTSTR)x)
#define _WW2A(x)	(LPCTSTR)(x)
#define _WW2A2(x,y)	(LPCSTR)UTL::_CW2A((LPCWSTR)x,y)
#define _A2W(x)		(LPCWSTR)(x)
#define _AA2W(x)	(LPCTSTR)UTL::_CA2W((LPCSTR)x)
#else
#define _IsEmpty	UTL::_IsEmptyA
#define _W2A(x)		(LPCSTR)(x)
#define _WW2A(x)	(LPCSTR)UTL::_CW2A((LPCWSTR)x)
#define _WW2A2(x,y)	(LPCSTR)UTL::_CW2A((LPCWSTR)x,y)
#define _A2W(x)		(LPCWSTR)UTL::_CA2W((LPCSTR)x)
#define _A2W2(x,y)	(LPCWSTR)UTL::_CA2W((LPCSTR)x,y)
#define _AA2W(x)	(LPCTSTR)(x)
#endif

#endif
