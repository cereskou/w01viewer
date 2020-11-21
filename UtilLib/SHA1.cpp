/*............................................................................
*
*  FILE NAME .............: SHA1.cpp
*
*  DESCRIPTION ...........: SHA1 hashes. The FIPS-180-1 compliant SHA1 Algorithm
*                           This code implements the SHA1 Algorithm defined in 
*                           FIPS-180-1, whose text is available at 
*                           http://csrc.nist.gov/
*  REVISION HISTORY ......:
*
*  When        Who          What
*  07/20/2015  K.K          Newly Created
............................................................................*/
#include "stdafx.h"
#include "sha1.h"

// Padding
static byte_t sha1_pad[64] = {
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// macros
#define SHA1_ROL(v, n) (((v) << (n)) | ((v) >> (32 - (n))))
#define SHA1_BLK(s,i) (s[i&15] = SHA1_ROL(s[(i+13)&15] ^ s[(i+8)&15] ^ s[(i+2)&15] ^ s[i&15],1))

#define F(x, y, z)	(z ^ (x & (y ^ z)))
#define H(x, y, z)	((x) ^ (y) ^ (z))
#define G(x, y, z)	((x&y) | (z & (x | y)))

// (R0=R1), R2, R3, R4 are the different operations usd in SHA1
#define SHA1_R0(a, b, c, d, e, x) \
	{\
	e += SHA1_ROL(a, 5) + F(b, c, d) + 0x5a827999 + x;\
	b = SHA1_ROL(b, 30);\
	}
#define SHA1_R2(a, b, c, d, e, x) \
	{\
	e += SHA1_ROL(a, 5) + H(b, c, d) + 0x6ed9eba1 + x;\
	b = SHA1_ROL(b, 30);\
	}
#define SHA1_R3(a, b, c, d, e, x) \
	{\
	e += SHA1_ROL(a, 5) + G(b, c, d) + 0x8f1bbcdc + x;\
	b = SHA1_ROL(b, 30);\
	}
#define SHA1_R4(a, b, c, d, e, x) \
	{\
	e += SHA1_ROL(a, 5) + H(b, c, d) + 0xca62c1d6 + x;\
	b = SHA1_ROL(b, 30);\
	}

SHA1::SHA1()
{
	SHA1_init(&sha1_ctx);
}

SHA1::~SHA1()
{
}

char* SHA1::GetDigest(const void* pData, size_t len)
{
	size_t bufflen = 4096;
	// Clear
	memset(&sha1_digest, 0x00, sizeof(sha1_digest));
	memset(&sha1_ctx, 0, sizeof(sha1_context));

	// Initialize
	SHA1_init(&sha1_ctx);

	int readed;

	byte_t* pBuff = (byte_t*)pData;
	readed = (int)len;
	while (readed > 0)
	{
		int readlen = (readed > (int)bufflen) ? (int)bufflen : readed;

		SHA1_update(&sha1_ctx, pBuff, readlen);

		pBuff += readlen;
		readed -= readlen;
	}

	byte_t hash[20];
	SHA1_final(&sha1_ctx, hash);

	const char hex[] = { "0123456789abcdef" };
	for (int i = 20; --i >= 0;)
	{
		sha1_digest[i << 1] = hex[(hash[i] >> 4) & 0xf];
		sha1_digest[(i << 1) + 1] = hex[hash[i] & 0xf];
	}
	sha1_digest[40] = 0;

	return sha1_digest;
}

char* SHA1::GetDigest(const char* fname)
{
	byte_t buff[4096] = { 0 };
	size_t bufflen = 4096;

	// Clear
	memset(&sha1_digest, 0x00, sizeof(sha1_digest));
	memset(&sha1_ctx, 0, sizeof(sha1_context));

	// Initialize
	SHA1_init(&sha1_ctx);

#ifdef WIN32
	DWORD  dwAccess;
	DWORD  dwShare;
	DWORD  dwCreate;
	HANDLE hf;
	DWORD readed;

	dwAccess = GENERIC_READ;
	dwShare = FILE_SHARE_READ;
	dwCreate = OPEN_EXISTING;

	hf = ::CreateFileA(fname, dwAccess, dwShare, NULL, dwCreate,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hf == INVALID_HANDLE_VALUE) return NULL;

	// ファイルの読込み
	while (ReadFile(hf, buff, (DWORD)bufflen, &readed, NULL))
	{
		if (readed <= 0) break;

		SHA1_update(&sha1_ctx, buff, readed);
	}

	::CloseHandle(hf);
#else
	FILE* fp;
	int readed;
	errno_t err;

	if ((err = fopen_s(&fp, fname, "rb")) != 0)
		return 0;

	// ファイルの読込み
	while ((readed = fread(buff, 1, bufflen, fp)) > 0)
	{
		SHA1_update(&sha1_ctx, buff, readed);
	}
	fclose(fp);
#endif
	byte_t hash[20];
	SHA1_final(&sha1_ctx, hash);

	const char hex[] = { "0123456789abcdef" };
	for (int i = 20; --i >= 0;)
	{
		sha1_digest[i << 1] = hex[(hash[i] >> 4) & 0xf];
		sha1_digest[(i << 1) + 1] = hex[hash[i] & 0xf];
	}
	sha1_digest[40] = 0;

	return sha1_digest;
}

void SHA1::SHA1_update(sha1_context *context, byte_t *buf, word_t len)
{
	size_t fill = 0;
	word_t left = 0;
	if (len == 0)
		return;

	left = context->count[0] & 0x3F;
	fill = 64 - left;

	context->count[0] += (word_t)len;
	context->count[0] &= 0xFFFFFFFF;

	if (context->count[0] < (word_t)len)
		context->count[1]++;

	byte_t *p = buf;
	if (left && len >= fill)
	{
		memcpy((void*)(context->buf + left), p, fill);
		SHA1_transform(context, context->buf);

		p += fill;
		len -= fill;
		left = 0;
	}

	while (len >= 64)
	{
		SHA1_transform(context, p);
		p += 64;
		len -= 64;
	}

	if (len > 0)
		memcpy((void*)(context->buf + left), p, len);
}

// Hash a single 512-bits block, This is the core of the algorithm
void SHA1::SHA1_transform(sha1_context *context, const byte_t *buf)
{
	word_t a, b, c, d, e;

	// copy abcde[] to working area
	a = context->abcde[0];
	b = context->abcde[1];
	c = context->abcde[2];
	d = context->abcde[3];
	e = context->abcde[4];

	word_t w[16] = { 0 };

	Decode(w, buf, 64);

	// 4 rounds of 20 operations each. Loop unrolled
	// Round 1
	// 0 - 19
	// F(x, y, z)	(z ^ (x & (y ^ z)))
	SHA1_R0(a, b, c, d, e, w[0]);
	SHA1_R0(e, a, b, c, d, w[1]);
	SHA1_R0(d, e, a, b, c, w[2]);
	SHA1_R0(c, d, e, a, b, w[3]);
	SHA1_R0(b, c, d, e, a, w[4]);
	SHA1_R0(a, b, c, d, e, w[5]);
	SHA1_R0(e, a, b, c, d, w[6]);
	SHA1_R0(d, e, a, b, c, w[7]);
	SHA1_R0(c, d, e, a, b, w[8]);
	SHA1_R0(b, c, d, e, a, w[9]);
	SHA1_R0(a, b, c, d, e, w[10]);
	SHA1_R0(e, a, b, c, d, w[11]);
	SHA1_R0(d, e, a, b, c, w[12]);
	SHA1_R0(c, d, e, a, b, w[13]);
	SHA1_R0(b, c, d, e, a, w[14]);
	SHA1_R0(a, b, c, d, e, w[15]);

	SHA1_R0(e, a, b, c, d, SHA1_BLK(w, 16));
	SHA1_R0(d, e, a, b, c, SHA1_BLK(w, 17));
	SHA1_R0(c, d, e, a, b, SHA1_BLK(w, 18));
	SHA1_R0(b, c, d, e, a, SHA1_BLK(w, 19));

	// Round 2
	// 20 - 39
	// H(x, y, z)	((x) ^ (y) ^ (z))
	SHA1_R2(a, b, c, d, e, SHA1_BLK(w, 20));
	SHA1_R2(e, a, b, c, d, SHA1_BLK(w, 21));
	SHA1_R2(d, e, a, b, c, SHA1_BLK(w, 22));
	SHA1_R2(c, d, e, a, b, SHA1_BLK(w, 23));
	SHA1_R2(b, c, d, e, a, SHA1_BLK(w, 24));
	SHA1_R2(a, b, c, d, e, SHA1_BLK(w, 25));
	SHA1_R2(e, a, b, c, d, SHA1_BLK(w, 26));
	SHA1_R2(d, e, a, b, c, SHA1_BLK(w, 27));
	SHA1_R2(c, d, e, a, b, SHA1_BLK(w, 28));
	SHA1_R2(b, c, d, e, a, SHA1_BLK(w, 29));
	SHA1_R2(a, b, c, d, e, SHA1_BLK(w, 30));
	SHA1_R2(e, a, b, c, d, SHA1_BLK(w, 31));
	SHA1_R2(d, e, a, b, c, SHA1_BLK(w, 32));
	SHA1_R2(c, d, e, a, b, SHA1_BLK(w, 33));
	SHA1_R2(b, c, d, e, a, SHA1_BLK(w, 34));
	SHA1_R2(a, b, c, d, e, SHA1_BLK(w, 35));
	SHA1_R2(e, a, b, c, d, SHA1_BLK(w, 36));
	SHA1_R2(d, e, a, b, c, SHA1_BLK(w, 37));
	SHA1_R2(c, d, e, a, b, SHA1_BLK(w, 38));
	SHA1_R2(b, c, d, e, a, SHA1_BLK(w, 39));

	// Round 3
	// 40 - 59
	// G(x, y, z)	((x&y) | (z & (x | y)))
	SHA1_R3(a, b, c, d, e, SHA1_BLK(w, 40));
	SHA1_R3(e, a, b, c, d, SHA1_BLK(w, 41));
	SHA1_R3(d, e, a, b, c, SHA1_BLK(w, 42));
	SHA1_R3(c, d, e, a, b, SHA1_BLK(w, 43));
	SHA1_R3(b, c, d, e, a, SHA1_BLK(w, 44));
	SHA1_R3(a, b, c, d, e, SHA1_BLK(w, 45));
	SHA1_R3(e, a, b, c, d, SHA1_BLK(w, 46));
	SHA1_R3(d, e, a, b, c, SHA1_BLK(w, 47));
	SHA1_R3(c, d, e, a, b, SHA1_BLK(w, 48));
	SHA1_R3(b, c, d, e, a, SHA1_BLK(w, 49));
	SHA1_R3(a, b, c, d, e, SHA1_BLK(w, 50));
	SHA1_R3(e, a, b, c, d, SHA1_BLK(w, 51));
	SHA1_R3(d, e, a, b, c, SHA1_BLK(w, 52));
	SHA1_R3(c, d, e, a, b, SHA1_BLK(w, 53));
	SHA1_R3(b, c, d, e, a, SHA1_BLK(w, 54));
	SHA1_R3(a, b, c, d, e, SHA1_BLK(w, 55));
	SHA1_R3(e, a, b, c, d, SHA1_BLK(w, 56));
	SHA1_R3(d, e, a, b, c, SHA1_BLK(w, 57));
	SHA1_R3(c, d, e, a, b, SHA1_BLK(w, 58));
	SHA1_R3(b, c, d, e, a, SHA1_BLK(w, 59));

	// Round 4
	// 60 - 79
	// H(x, y, z)	((x) ^ (y) ^ (z))
	SHA1_R4(a, b, c, d, e, SHA1_BLK(w, 60));
	SHA1_R4(e, a, b, c, d, SHA1_BLK(w, 61));
	SHA1_R4(d, e, a, b, c, SHA1_BLK(w, 62));
	SHA1_R4(c, d, e, a, b, SHA1_BLK(w, 63));
	SHA1_R4(b, c, d, e, a, SHA1_BLK(w, 64));
	SHA1_R4(a, b, c, d, e, SHA1_BLK(w, 65));
	SHA1_R4(e, a, b, c, d, SHA1_BLK(w, 66));
	SHA1_R4(d, e, a, b, c, SHA1_BLK(w, 67));
	SHA1_R4(c, d, e, a, b, SHA1_BLK(w, 68));
	SHA1_R4(b, c, d, e, a, SHA1_BLK(w, 69));
	SHA1_R4(a, b, c, d, e, SHA1_BLK(w, 70));
	SHA1_R4(e, a, b, c, d, SHA1_BLK(w, 71));
	SHA1_R4(d, e, a, b, c, SHA1_BLK(w, 72));
	SHA1_R4(c, d, e, a, b, SHA1_BLK(w, 73));
	SHA1_R4(b, c, d, e, a, SHA1_BLK(w, 74));
	SHA1_R4(a, b, c, d, e, SHA1_BLK(w, 75));
	SHA1_R4(e, a, b, c, d, SHA1_BLK(w, 76));
	SHA1_R4(d, e, a, b, c, SHA1_BLK(w, 77));
	SHA1_R4(c, d, e, a, b, SHA1_BLK(w, 78));
	SHA1_R4(b, c, d, e, a, SHA1_BLK(w, 79));

	// add the working vars back into digest[]
	context->abcde[0] += a;
	context->abcde[1] += b;
	context->abcde[2] += c;
	context->abcde[3] += d;
	context->abcde[4] += e;
}


// add padding and return the message digest
void SHA1::SHA1_final(sha1_context *context, byte_t *digest)
{
	word_t last, padn;
	word_t total[2] = { 0 };

	byte_t bits[8] = { 0 };

	// total number of hashed bits
	total[0] = (context->count[0] >> 29) | (context->count[1] << 3);
	total[1] = (context->count[0] << 3);

	Encode(bits, total, 8);

	last = context->count[0] & 0x3F;
	padn = (last < 56) ? (56 - last) : (120 - last);

	// padding
	SHA1_update(context, sha1_pad, padn);
	// append length
	SHA1_update(context, bits, 8);

	// store state in digest
	Encode(digest, context->abcde, 20);
}

void SHA1::SHA1_init(sha1_context *context)
{
	context->count[0] = 0;
	context->count[1] = 0;

	// low-order bytes first
	// word A: 01 23 45 67
	// WORD B: 89 ab cd ef
	// WORD C: fe dc ba 98
	// WORD D: 76 54 32 10
	// WORD E: 10 32 54 76
	context->abcde[0] = 0x67452301;
	context->abcde[1] = 0xefcdab89;
	context->abcde[2] = 0x98badcfe;
	context->abcde[3] = 0x10325476;
	context->abcde[4] = 0xc3d2e1f0;
}

// Convert the word_t array to byte buffer
void SHA1::Encode(byte_t *out, const word_t *in, word_t len)
{
	word_t i, j;
	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		out[j] = (byte_t)(in[i] >> 24);
		out[j + 1] = (byte_t)(in[i] >> 16);
		out[j + 2] = (byte_t)(in[i] >> 8);
		out[j + 3] = (byte_t)(in[i]);
	}
}

// Convert the bytes buffer to a word_t array
void SHA1::Decode(word_t *out, const byte_t *buf, word_t len)
{
	word_t	i, j;
	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		out[i] =
			((word_t)buf[j] << 24) |
			((word_t)buf[j + 1] << 16) |
			((word_t)buf[j + 2] << 8) |
			((word_t)buf[j + 3]);
	}
}

