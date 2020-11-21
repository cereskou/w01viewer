#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__
#pragma once

#include <vector>
#include <Gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

class ByteStream : public IStream
{
	LONG				_refcount;
	std::vector<BYTE>		m_a;
	long				m_read_position;

public:

	void Reserve(const size_t n)
	{
		m_a.reserve(n);
	}

	void Init()
	{
		m_a.clear();
		m_read_position = 0;
	}

	ByteStream()
	{
		_refcount = 1;
		Init();
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
	{
		if (iid == __uuidof(IUnknown)
		|| iid == __uuidof(IStream)
		|| iid == __uuidof(ISequentialStream))
		{
			*ppvObject = static_cast<IStream*>(this);
			AddRef();
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return (ULONG)InterlockedIncrement(&_refcount);
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
        	ULONG res = (ULONG) InterlockedDecrement(&_refcount);
		if (res == 0) delete this;
		return res;
	}

	// ISequentialStream Interface
public:
	virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		size_t		bytes_left;
		size_t		bytes_out;

		if (pcbRead != NULL) *pcbRead = 0;
		if (m_read_position == m_a.size()) // EOF
		{
			return HRESULT_FROM_WIN32(ERROR_END_OF_MEDIA);
		}

		bytes_left = m_a.size() - m_read_position;
		bytes_out = min(cb, bytes_left);
		memcpy(pv, &m_a[m_read_position], bytes_out);
		m_read_position += bytes_out;
		if (pcbRead != NULL) *pcbRead = bytes_out;

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
	{
		size_t	prev_size;

		if (cb <= 0)
		{
			if (pcbWritten != NULL) *pcbWritten = 0;
			return S_OK;
		}

		prev_size = m_a.size();
		m_a.resize(prev_size + cb);
		memcpy(&m_a[prev_size], pv, cb);
		if (pcbWritten != NULL) *pcbWritten = cb;

		return S_OK;
	}

	// IStream Interface
public:
	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)
	{
        	return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Revert(void)
	{
        	return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
	{
        	return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
	{
		long	start = 0;
		long	new_read_position;

		switch(dwOrigin)
		{
		case STREAM_SEEK_SET:
			start = 0;
			break;
		case STREAM_SEEK_CUR:
			start = m_read_position;
			break;
		case STREAM_SEEK_END:
			start = m_a.size();
			break;
		default:
			return STG_E_INVALIDFUNCTION;
			break;
		}

		new_read_position = start + (long)liDistanceToMove.QuadPart;
		// Allowed to move to m_a.size() which is EOF
		if (new_read_position < 0 || new_read_position > m_a.size())
		{
			return STG_E_SEEKERROR;
		}

		m_read_position = new_read_position;

		if (lpNewFilePointer != NULL)
		{
			lpNewFilePointer->QuadPart = m_read_position;
		}

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag)
	{
		if (pStatstg == NULL)
		{
			return STG_E_INVALIDFUNCTION;
		}

		ZeroMemory(pStatstg, sizeof(STATSTG));
		pStatstg->cbSize.QuadPart = m_a.size();
		return S_OK;
	}
};

#endif
