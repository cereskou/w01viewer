/*............................................................................
*
*  FILE NAME .............: SHA1.h
*
*  DESCRIPTION ...........: Header for SHA1 hashes. 
*
*  REVISION HISTORY ......:
*
*  When        Who          What
*  02/08/2012  K.K          FT8833 Newly Created
............................................................................*/

#ifndef __SHA1_H__
#define __SHA1_H__

#pragma once

#include <windows.h>

typedef unsigned char	byte_t;		// 8-bit byte
typedef unsigned int	word_t;		// 32-bit word

typedef struct _sha1_context {
	word_t	count[2];		// number of bits modulo 2^64
	word_t	abcde[5];		// state 
	byte_t	buf[64];		// input buffer
} sha1_context;

class SHA1
{
public:
	SHA1();
	virtual ~SHA1();

public:
	char* GetDigest(const char* fname);
	char* GetDigest(const void* pData, size_t len);

private:
	sha1_context	sha1_ctx;
	char			sha1_digest[41];

private:
	void SHA1_init(sha1_context *context);
	void SHA1_final(sha1_context *context, byte_t *digest);
	void SHA1_update(sha1_context *context, byte_t *buf, word_t len);
	void SHA1_transform(sha1_context *context, const byte_t *buf);

	void Encode(byte_t *out, const word_t *in, word_t len);
	void Decode(word_t *out, const byte_t *buf, word_t len);
};

#endif
