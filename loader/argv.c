/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "cache.h"

typedef unsigned int u32;
typedef volatile u32 vu32;

extern vu32 *__system_argv;
extern void *memmove(void *dest, void *src, int n);

void copyArgv()
{
	u32 curArgMagic = __system_argv[0];
	void *curCmdPtr = (void*)__system_argv[1];
	u32 curCmdLen = __system_argv[2];

	if (curArgMagic != 0x5F617267 || curCmdPtr == 0 || curCmdLen == 0) {
		//no arguments given
		*(vu32*)0x817C4000 = 0;
		return;
	}
	//move cmd into high MEM1 to protect
	memmove((void*)0x817C4020, curCmdPtr, curCmdLen);
	DCFlushRange((void*)0x817C4020, curCmdLen);
	//set up basic args structure
	*(vu32*)0x817C4000 = 0x5F617267;
	*(vu32*)0x817C4004 = 0x817C4020;
	*(vu32*)0x817C4008 = curCmdLen;
	DCFlushRange((void*)0x817C4000, 0x20);
}
