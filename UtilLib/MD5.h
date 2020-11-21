/*............................................................................
*
*  FILE NAME .............: MD5.h
*
*  DESCRIPTION ...........: Header for MD5 hashes. 
*
*  REVISION HISTORY ......:
*
*  When        Who          What
*  02/08/2012  K.K          FT8833 Newly Created
............................................................................*/

#ifndef __MD5_H__
#define __MD5_H__

#pragma once

#include <windows.h>

typedef unsigned char	byte_t;		// 8-bit byte
typedef unsigned int	word_t;		// 32-bit word

typedef struct _md5_context {
	word_t	count[2];		// number of bits modulo 2^64
	word_t	abcd[4];		// state 
	byte_t	buf[64];		// input buffer
} md5_context;

class MD5
{
public:
	MD5(void);
	~MD5(void);
public:
	char* MD5::GetDigest(const char* fname);
	char* MD5::GetDigest(const LPVOID pData, DWORD dwLen);

private:
	md5_context	md5_ctx;
	char	md5_digest[33];

private:
	void MD5_init(md5_context *context);
	void MD5_final(md5_context *context, byte_t *digest);
	void MD5_update(md5_context *context, byte_t *buf, word_t len);
	void MD5_transform(md5_context *context, byte_t *buf);
};

#endif
