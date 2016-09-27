/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "cache.h"
#include "xz.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef volatile u32 vu32;
typedef volatile u16 vu16;

#if defined(HW_DOL)
#include "cube/dolloader.h"
#elif defined(HW_RVL)
#include "wii/dolloader.h"
#endif

#define ALIGN32(x)			(((x) + 31) & ~31)
extern vu32 dol_size;

typedef struct _dolheader {
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

void *memset(void *ptr, int c, int size)
{
	char* ptr2 = ptr;
	while(size--)
		*ptr2++ = (char)c;
	return ptr;
}

void *memcpy(void *ptr, const void *src, int size)
{
	char* ptr2 = ptr;
	const char* src2 = src;
	while(size--)
		*ptr2++ = *src2++;
	return ptr;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
     const unsigned char *us1 = (const unsigned char *) s1;
     const unsigned char *us2 = (const unsigned char *) s2;
     while (n-- != 0)
	 {
         if (*us1 != *us2)
             return (*us1 < *us2) ? -1 : +1;
         us1++;
         us2++;
     }
     return 0;
}

void *memmove(void *dest, void *src, size_t n)
{
	char *d =(char *)dest; 
	char *s =(char *)src; 

	if(s == d)
		return dest;

	if(s < d)
	{
		//copy from back
		s=s+n-1; 
		d=d+n-1; 
		while(n--)
			*d-- = *s--;
	}
	else
	{ 
		//copy from front
		while(n--)
			*d++ = *s++;
	}
	return dest; 
} 

#if defined(HW_DOL)
#define TO_ARAM 0
#define TO_MRAM 1
void ar_dma(u32 type, u32 mram, u32 aram, u32 len)
{
	*(vu16*)0xCC005020 = (mram>>16);
	*(vu16*)0xCC005022 = (mram&0xFFFF);
	*(vu16*)0xCC005024 = (aram>>16);
	*(vu16*)0xCC005026 = (aram&0xFFFF);
	*(vu16*)0xCC005028 = (type<<15)|(len>>16);
	*(vu16*)0xCC00502A = (len&0xFFFF);
	while(*(vu16*)0xCC00500A & 0x200) ;
}
#elif defined(HW_RVL)
u8 *MEM2_start = (u8*)0x90600000;
#endif

#if defined(DEBUG)

#if defined(HW_DOL)
#define EXI 0xCC006814
#elif defined(HW_RVL)
#define EXI 0xCD806814
#endif
void EXISendByte( char byte )
{
loop:
	*(vu32*)EXI			= 0xD0;
	*(vu32*)(EXI+0x10)	= 0xB0000000 | (byte<<20);
	*(vu32*)(EXI+0x0C)	= 0x19;

	while( *(vu32*)(EXI+0x0C)&1 );

	u32 loop = *(vu32*)(EXI+0x10)&0x4000000;
	
	*(vu32*)EXI	= 0;

	if( !loop )
		goto loop;

	return;
}
void pChar(char byte)
{
	EXISendByte(byte);EXISendByte('\n');
}
void pHex(u32 h)
{
	int i;
	for(i = 28; i >= 0; i-=4)
	{
		u32 cBit = ((h>>i)&0xF);
		if(cBit < 10)
			EXISendByte('0'+cBit);
		else
			EXISendByte('A'+(cBit-10));
	}
	EXISendByte('\n');
}
#else
#define pChar(...)
#define pHex(...)

#endif

extern u8 systemcallhandler_start[],systemcallhandler_end[];
void _main() 
{
	//make sure "sc" handler is installed
	void *syscallMem = (void*)0x80000C00;
	int syscallLen = (systemcallhandler_end-systemcallhandler_start);
	memcpy(syscallMem,systemcallhandler_start,syscallLen);
	DCFlushRangeNoSync(syscallMem,syscallLen);
	ICInvalidateRange(syscallMem,syscallLen);

	pChar('1');
	xz_crc32_init();
	struct xz_dec *decStr = xz_dec_init(XZ_SINGLE,0);
	pChar('2');
	struct xz_buf b;
	//as defined in the .ld file
#if defined(HIGH)
	u32 inPos = 0x81320000;
#elif defined(LOW)
	u32 inPos = 0x80020000;
#endif
	b.in = (void*)inPos;
	pHex(*(vu32*)b.in);
	b.in_pos = 0;
	b.in_size = dol_size;
	
#if defined(HIGH)
	//start decompression at lowest MEM1 point
#if defined(HW_RVL)
	u32 outPos = 0x80003400;
#elif defined(HW_DOL)
	u32 outPos = 0x80003100;
#endif
	//our loader comes above this
	u32 maxOutSize = 0x81300000 - outPos;
#elif defined(LOW)
	//use remaining MEM1 for decompression
	u32 outPos = inPos + ALIGN32(dol_size);
	//dolloader/args come above this
	u32 maxOutSize = 0x817C0000 - outPos;
#endif
	b.out = (void*)outPos;
	b.out_pos = 0;
	b.out_size = maxOutSize;
	xz_dec_run(decStr, &b);
	pHex(b.out_pos);
	xz_dec_end(decStr);
	//out_pos after decompress is uncompressed total
#if defined(HW_DOL)
	u32 aligned_out_size = ALIGN32(b.out_pos);
	DCFlushRange((void*)outPos, aligned_out_size);
	ar_dma(TO_ARAM, outPos, 0, aligned_out_size);
#elif defined(HW_RVL)
	memcpy(MEM2_start, (void*)outPos, b.out_pos);
	DCFlushRange((void*)MEM2_start, b.out_pos);
#endif
	pChar('3');
	//done with setup, start up dolloader
	memcpy((void*)0x817C0000, dolloader, dolloader_size);
	DCFlushRangeNoSync((void*)0x817C0000, dolloader_size);
	ICInvalidateRange((void*)0x817C0000, dolloader_size);
	__asm__ volatile(
		"lis 3, 0x817C\n"
		"mtlr 3\n"
		"blr\n"
	);
	__builtin_unreachable();
}
