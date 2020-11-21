/*............................................................................
*
*  FILE NAME .............: MD5.cpp
*
*  DESCRIPTION ...........: MD5 hashes. The MD5 Message-Digest Algorithm
*                           This code implements the MD5 Algorithm defined in 
*                           RFC1321, whose text is available at 
*                           http://www.ietf.org/rfc/rfc1321.txt
*  REVISION HISTORY ......:
*
*  When        Who          What
*  02/08/2012  K.K          FT8833 Newly Created
............................................................................*/

#include "stdafx.h"
#include "md5.h"
#include <fcntl.h>
#include <share.h>
#include <stdio.h>
#include <io.h>
#include <memory.h>

// Padding
static byte_t md5_pad[64] = {
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// MD5 basic functions: selection, majority, parity
// input three 32-bit words and produce as output one 32-bit word.
// F G H I
// F(X,Y,Z) = XY v not(X)Z     (X and Y) or ((not X) and Z)
// G(X,Y,Z) = XZ v Y not(Z)	   (X and Z) or (Y and (not Z)
// H(X,Y,Z) = X xor Y xor Z	   
// I(X,Y,Z) = Y xor (X v not(Z))
#define F(x, y, z)	(((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)	(((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)	((x) ^ (y) ^ (z))
#define I(x, y, z)	((y) ^ ((x) | (~z)))

// RL - rotates x left n bits
#ifndef RL
#define RL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#endif

// Transformations for Rounds 1, 2, 3 and 4.
#define MD5_FF(a, b, c, d, x, s, ac) {(a) += F((b), (c), (d)) + (x) + (word_t)(ac); (a) = RL((a), (s)); (a) += (b);}
#define MD5_GG(a, b, c, d, x, s, ac) {(a) += G((b), (c), (d)) + (x) + (word_t)(ac); (a) = RL((a), (s)); (a) += (b);}
#define MD5_HH(a, b, c, d, x, s, ac) {(a) += H((b), (c), (d)) + (x) + (word_t)(ac); (a) = RL((a), (s)); (a) += (b);}
#define MD5_II(a, b, c, d, x, s, ac) {(a) += I((b), (c), (d)) + (x) + (word_t)(ac); (a) = RL((a), (s)); (a) += (b);}

// 
// Constants for transformation(RFC1321)
// Round 1
#define S11		7
#define S12		12
#define S13		17
#define S14		22
// Round 2
#define S21		5
#define S22		9
#define S23		14
#define S24		20
// Round 3
#define S31		4
#define S32		11
#define S33		16
#define S34		23
// Round 4
#define S41		6
#define S42		10
#define S43		15
#define S44		21

// Round 1
#define F0 	0xd76aa478
#define F1 	0xe8c7b756
#define F2 	0x242070db
#define F3 	0xc1bdceee
#define F4 	0xf57c0faf
#define F5 	0x4787c62a
#define F6 	0xa8304613
#define F7 	0xfd469501
#define F8 	0x698098d8
#define F9 	0x8b44f7af
#define FA 	0xffff5bb1
#define FB 	0x895cd7be
#define FC 	0x6b901122
#define FD 	0xfd987193
#define FE	0xa679438e
#define FF	0x49b40821

// Round 2
#define G0 	0xf61e2562
#define G1 	0xc040b340
#define G2 	0x265e5a51
#define G3 	0xe9b6c7aa
#define G4 	0xd62f105d
#define G5 	0x2441453
#define G6 	0xd8a1e681
#define G7 	0xe7d3fbc8
#define G8 	0x21e1cde6
#define G9 	0xc33707d6
#define GA 	0xf4d50d87
#define GB 	0x455a14ed
#define GC 	0xa9e3e905
#define GD 	0xfcefa3f8
#define GE 	0x676f02d9
#define GF	0x8d2a4c8a

// Round 3
#define H0	0xfffa3942
#define H1	0x8771f681
#define H2	0x6d9d6122
#define H3	0xfde5380c
#define H4	0xa4beea44
#define H5	0x4bdecfa9
#define H6	0xf6bb4b60
#define H7	0xbebfbc70
#define H8	0x289b7ec6
#define H9	0xeaa127fa
#define HA	0xd4ef3085
#define HB	0x4881d05
#define HC	0xd9d4d039
#define HD	0xe6db99e5
#define HE	0x1fa27cf8
#define HF	0xc4ac5665

// Round 4
#define I0	0xf4292244
#define I1 	0x432aff97
#define I2 	0xab9423a7
#define I3 	0xfc93a039
#define I4 	0x655b59c3
#define I5 	0x8f0ccc92
#define I6 	0xffeff47d
#define I7 	0x85845dd1
#define I8 	0x6fa87e4f
#define I9 	0xfe2ce6e0
#define IA 	0xa3014314
#define IB 	0x4e0811a1
#define IC 	0xf7537e82
#define ID 	0xbd3af235
#define IE 	0x2ad7d2bb
#define IF 	0xeb86d391


void encode(byte_t *out, word_t *in, word_t len)
{
	word_t i, j;
	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		out[j] = (byte_t)(in[i] & 0xff);
		out[j + 1] = (byte_t)((in[i] >> 8) & 0xff);
		out[j + 2] = (byte_t)((in[i] >> 16) & 0xff);
		out[j + 3] = (byte_t)((in[i] >> 24) & 0xff);
	}
}

void decode(word_t *out, byte_t *buf, word_t len)
{
	word_t	i, j;
	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		out[i] =
			((word_t)buf[j]) |
			(((word_t)buf[j + 1]) << 8) |
			(((word_t)buf[j + 2]) << 16) |
			(((word_t)buf[j + 3]) << 24);
	}
}

MD5::MD5(void)
{
}

MD5::~MD5(void)
{
}

char* MD5::GetDigest(const LPVOID pData, DWORD dwLen)
{
	size_t bufflen = 4096;
	// Initialize
	memset(&md5_digest, 0x00, sizeof(md5_digest));
	memset(&md5_ctx, 0, sizeof(md5_context));
	MD5_init(&md5_ctx);

	int readed;

	byte_t* pBuff = (byte_t*)pData;
	readed = dwLen;
	while (readed > 0)
	{
		int len = (int)((readed > (int)bufflen) ? bufflen : readed);
		MD5_update(&md5_ctx, pBuff, len);

		pBuff += len;
		readed -= len;
	}

	byte_t digest[16];
	MD5_final(&md5_ctx, digest);

	memset(&md5_digest, 0x00, sizeof(md5_digest));
	for (int i = 0, p = 0; i < 16; i++, p += 2)
	{
		char tmp[4] = { 0 };
		sprintf_s(tmp, 4, "%02x\0", digest[i]);
		strcat_s(md5_digest, 33, tmp);
//		sprintf_s(&md5_digest[p], 33, "%02x", digest[i]);
	}

	return md5_digest;
}

char* MD5::GetDigest(const char* fname)
{
	byte_t buff[4096];
	size_t bufflen = 4096;
	// Initialize
	memset(&md5_digest, 0x00, sizeof(md5_digest));
	memset(&md5_ctx, 0, sizeof(md5_context));
	MD5_init(&md5_ctx);

#ifdef WIN32
	/*	errno_t err;
		FILE* f;
		size_t readed;

		if( (err = fopen_s(&f, fname, "rb"))!= 0)
		{
			return NULL;
		}
		size_t st = sizeof(byte_t);

		while((readed = fread_s(buff, bufflen, st, bufflen, f)) > 0)
		{
			MD5_update(&md5_ctx, buff, readed);
		}

		fclose(f);
	*/
	DWORD  dwAccess;
	DWORD  dwShare;
	DWORD  dwCreate;
	HANDLE hf;
	DWORD readed;

	dwAccess = GENERIC_READ;
	dwShare = FILE_SHARE_READ;
	dwCreate = OPEN_EXISTING;

	hf = CreateFileA(fname, dwAccess, dwShare, NULL, dwCreate,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hf == INVALID_HANDLE_VALUE) return NULL;

	// ファイルの読込み
	while (ReadFile(hf, buff, (DWORD)bufflen, &readed, NULL))
	{
		if (readed <= 0) break;

		MD5_update(&md5_ctx, buff, readed);
	}

	CloseHandle(hf);
#else
	int fmode;
	int pmode;
	FILE* fp;
	int readed;

	fmode = _O_BINARY | _O_RDONLY | _O_RANDOM;
	pmode = 0;
	fp = fopen(fname, "r");
	if (fp == NULL) return 0;

	// ファイルの読込み
	while ((readed = fread(buff, 1, bufflen, fp)) > 0)
	{
		MD5_update(&md5_ctx, buff, readed);
	}
	fclose(fp);
#endif
	byte_t digest[16];
	MD5_final(&md5_ctx, digest);

	memset(&md5_digest, 0x00, sizeof(md5_digest));
	for (int i = 0, p = 0; i < 16; i++, p += 2)
	{
		char tmp[4] = { 0 };
		sprintf_s(tmp, 4, "%02x\0", digest[i]);
		strcat_s(md5_digest, 33, tmp);
		//sprintf_s(&md5_digest[p], 33, "%02x", digest[i]);
	}

	return md5_digest;
}

void MD5::MD5_update(md5_context *context, byte_t *buf, word_t len)
{
	word_t left = len;

	if (len <= 0)
		return;

	// compute number of bytes mod 64
	int offset = (word_t)((context->count[0] >> 3) & 63);
	word_t nbits = (word_t)(len << 3);

	// update number of bits
	context->count[1] += (len >> 29);
	context->count[0] += nbits;
	if (context->count[0] < nbits)
		context->count[1]++;

	byte_t *p = buf;
	if (offset)
	{
		int copy = (offset + len > 64 ? 64 - offset : len);
		memcpy(context->buf + offset, p, copy);

		if (offset + copy < 64)
			return;

		p += copy;
		left -= copy;

		MD5_transform(context, context->buf);
	}

	// transform full blocks
	while (left >= 64)
	{
		MD5_transform(context, p);
		p += 64;
		left -= 64;
	}

	// buffer remaining input
	if (left)
	{
		memcpy(context->buf, p, left);
	}
}

void MD5::MD5_transform(md5_context *context, byte_t *buf)
{
	word_t a, b, c, d;

	a = context->abcd[0];
	b = context->abcd[1];
	c = context->abcd[2];
	d = context->abcd[3];

	word_t x[16];

	decode(x, buf, 64);

	// Round 1
	// Let [abcd k s i] denote the operation
	//	a = b + ((a + FF(b, c, d) + X[k] + T[i]) <<< s).
	// Do the following 16 operations
	MD5_FF(a, b, c, d, x[0], S11, F0);
	MD5_FF(d, a, b, c, x[1], S12, F1);
	MD5_FF(c, d, a, b, x[2], S13, F2);
	MD5_FF(b, c, d, a, x[3], S14, F3);
	MD5_FF(a, b, c, d, x[4], S11, F4);
	MD5_FF(d, a, b, c, x[5], S12, F5);
	MD5_FF(c, d, a, b, x[6], S13, F6);
	MD5_FF(b, c, d, a, x[7], S14, F7);
	MD5_FF(a, b, c, d, x[8], S11, F8);
	MD5_FF(d, a, b, c, x[9], S12, F9);
	MD5_FF(c, d, a, b, x[10], S13, FA);
	MD5_FF(b, c, d, a, x[11], S14, FB);
	MD5_FF(a, b, c, d, x[12], S11, FC);
	MD5_FF(d, a, b, c, x[13], S12, FD);
	MD5_FF(c, d, a, b, x[14], S13, FE);
	MD5_FF(b, c, d, a, x[15], S14, FF);

	// Round 2
	// Let [abcd k s i] denote the operation
	//	a = b + ((a + GG(b, c, d) + X[k] + T[i]) <<< s).
	// Do the following 16 operations
	MD5_GG(a, b, c, d, x[1], S21, G0);
	MD5_GG(d, a, b, c, x[6], S22, G1);
	MD5_GG(c, d, a, b, x[11], S23, G2);
	MD5_GG(b, c, d, a, x[0], S24, G3);
	MD5_GG(a, b, c, d, x[5], S21, G4);
	MD5_GG(d, a, b, c, x[10], S22, G5);
	MD5_GG(c, d, a, b, x[15], S23, G6);
	MD5_GG(b, c, d, a, x[4], S24, G7);
	MD5_GG(a, b, c, d, x[9], S21, G8);
	MD5_GG(d, a, b, c, x[14], S22, G9);
	MD5_GG(c, d, a, b, x[3], S23, GA);
	MD5_GG(b, c, d, a, x[8], S24, GB);
	MD5_GG(a, b, c, d, x[13], S21, GC);
	MD5_GG(d, a, b, c, x[2], S22, GD);
	MD5_GG(c, d, a, b, x[7], S23, GE);
	MD5_GG(b, c, d, a, x[12], S24, GF);

	// Round 3
	// Let [abcd k s i] denote the operation
	//	a = b + ((a + HH(b, c, d) + X[k] + T[i]) <<< s).
	// Do the following 16 operations
	MD5_HH(a, b, c, d, x[5], S31, H0);
	MD5_HH(d, a, b, c, x[8], S32, H1);
	MD5_HH(c, d, a, b, x[11], S33, H2);
	MD5_HH(b, c, d, a, x[14], S34, H3);
	MD5_HH(a, b, c, d, x[1], S31, H4);
	MD5_HH(d, a, b, c, x[4], S32, H5);
	MD5_HH(c, d, a, b, x[7], S33, H6);
	MD5_HH(b, c, d, a, x[10], S34, H7);
	MD5_HH(a, b, c, d, x[13], S31, H8);
	MD5_HH(d, a, b, c, x[0], S32, H9);
	MD5_HH(c, d, a, b, x[3], S33, HA);
	MD5_HH(b, c, d, a, x[6], S34, HB);
	MD5_HH(a, b, c, d, x[9], S31, HC);
	MD5_HH(d, a, b, c, x[12], S32, HD);
	MD5_HH(c, d, a, b, x[15], S33, HE);
	MD5_HH(b, c, d, a, x[2], S34, HF);

	// Round 4
	// Let [abcd k s i] denote the operation
	//	a = b + ((a + II(b, c, d) + X[k] + T[i]) <<< s).
	// Do the following 16 operations
	MD5_II(a, b, c, d, x[0], S41, I0);
	MD5_II(d, a, b, c, x[7], S42, I1);
	MD5_II(c, d, a, b, x[14], S43, I2);
	MD5_II(b, c, d, a, x[5], S44, I3);
	MD5_II(a, b, c, d, x[12], S41, I4);
	MD5_II(d, a, b, c, x[3], S42, I5);
	MD5_II(c, d, a, b, x[10], S43, I6);
	MD5_II(b, c, d, a, x[1], S44, I7);
	MD5_II(a, b, c, d, x[8], S41, I8);
	MD5_II(d, a, b, c, x[15], S42, I9);
	MD5_II(c, d, a, b, x[6], S43, IA);
	MD5_II(b, c, d, a, x[13], S44, IB);
	MD5_II(a, b, c, d, x[4], S41, IC);
	MD5_II(d, a, b, c, x[11], S42, ID);
	MD5_II(c, d, a, b, x[2], S43, IE);
	MD5_II(b, c, d, a, x[9], S44, IF);

	context->abcd[0] += a;
	context->abcd[1] += b;
	context->abcd[2] += c;
	context->abcd[3] += d;

	memset(x, 0, sizeof(x));
}

void MD5::MD5_final(md5_context *context, byte_t *digest)
{
	byte_t bits[8];
	word_t index, padlen;

	// save number of bits
	encode(bits, context->count, 8);

	// pad out to 56 mod 64
	index = ((context->count[0] >> 3) & 63);
	padlen = (index < 56) ? (56 - index) : (120 - index);
	MD5_update(context, md5_pad, padlen);

	// appedn length
	MD5_update(context, bits, 8);

	// store state in digest
	encode(digest, context->abcd, 16);

	// clear
	memset(context, 0, sizeof(md5_context));
}

void MD5::MD5_init(md5_context *context)
{
	context->count[0] = 0;
	context->count[1] = 0;
	// low-order bytes first
	// word A: 01 23 45 67
	// WORD B: 89 ab cd ef
	// WORD C: fe dc ba 98
	// WORD D: 76 54 32 10
	context->abcd[0] = 0x67452301;
	context->abcd[1] = 0xefcdab89;
	context->abcd[2] = 0x98badcfe;
	context->abcd[3] = 0x10325476;
}

