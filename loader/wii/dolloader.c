/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef volatile u32 vu32;

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

static void sync_cache(void *p, u32 n)
{
	u32 start, end;

	start = (u32)p & ~31;
	end = ((u32)p + n + 31) & ~31;
	n = (end - start) >> 5;

	while (n--) {
		asm("dcbst 0,%0 ; icbi 0,%0" : : "b"(p));
		p += 32;
	}
	asm("sync ; isync");
}

void *_memset(void *ptr, int c, int size)
{
	char* ptr2 = ptr;
	while(size--)
		*ptr2++ = (char)c;
	return ptr;
}

void *_memcpy(void *ptr, const void *src, int size)
{
	char* ptr2 = ptr;
	const char* src2 = src;
	while(size--)
		*ptr2++ = *src2++;
	return ptr;
}

u8 *MEM2_start = (u8*)0x90600000;
void __attribute__ ((noreturn)) _main()
{
	int i;
	dolheader *dolfile = (dolheader*)MEM2_start;
	//write down boot.dol in memory
	for (i = 0; i < 7; i++)
	{
		if ((!dolfile->text_size[i]) || (dolfile->text_start[i] < 0x100))
			continue;
		_memcpy((void *) dolfile->text_start[i], MEM2_start + dolfile->text_pos[i], dolfile->text_size[i]);
		sync_cache((void *) dolfile->text_start[i], dolfile->text_size[i]);
	}
	for (i = 0; i < 11; i++)
	{
		if ((!dolfile->data_size[i]) || (dolfile->data_start[i] < 0x100))
			continue;
		_memcpy((void *) dolfile->data_start[i], MEM2_start + dolfile->data_pos[i], dolfile->data_size[i]);
		sync_cache((void *) dolfile->data_start[i], dolfile->data_size[i]);
	}
	//copy over arg struct
	if(*(vu32*)((dolfile->entry_point)+4) == 0x5F617267 && *(vu32*)0x817C4000 == 0x5F617267)
	{
		_memcpy((void*)((dolfile->entry_point)+8), (void*)0x817C4000, 12);
		sync_cache((void*)((dolfile->entry_point)+8), 12);
	}
	//lets jump
	__asm__ volatile(
		"mtlr %0\n"
		"blr\n"
	 : : "r"(dolfile->entry_point));
	 __builtin_unreachable();
}
