/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <stdio.h>
#include <malloc.h>
#include <lzma.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include "loader/loader_cube.h"
#include "loader/loader_cube_high.h"
#include "loader/loader_wii.h"
#include "loader/loader_wii_high.h"

typedef struct _dolheader {
	unsigned int text_pos[7];
	unsigned int data_pos[11];
	unsigned int text_start[7];
	unsigned int data_start[11];
	unsigned int text_size[7];
	unsigned int data_size[11];
	unsigned int bss_start;
	unsigned int bss_size;
	unsigned int entry_point;
} dolheader;

static bool init_encoder(lzma_stream *strm)
{
	lzma_options_lzma opt_lzma2;
	if (lzma_lzma_preset(&opt_lzma2, LZMA_PRESET_DEFAULT))
		return false;

	lzma_filter filters[] = {
		{ .id = LZMA_FILTER_POWERPC, .options = NULL },
		{ .id = LZMA_FILTER_LZMA2, .options = &opt_lzma2 },
		{ .id = LZMA_VLI_UNKNOWN, .options = NULL },
	};
	lzma_ret ret = lzma_stream_encoder(strm, filters, LZMA_CHECK_CRC32);

	if (ret == LZMA_OK)
		return true;

	return false;
}

int cmpSize = 0;

static bool compress(lzma_stream *strm, FILE *infile, FILE *outfile)
{
	lzma_action action = LZMA_RUN;

	uint8_t inbuf[BUFSIZ];
	uint8_t outbuf[BUFSIZ];

	strm->next_in = NULL;
	strm->avail_in = 0;
	strm->next_out = outbuf;
	strm->avail_out = sizeof(outbuf);

	while (true) {
		if (strm->avail_in == 0 && !feof(infile)) {
			strm->next_in = inbuf;
			strm->avail_in = fread(inbuf, 1, sizeof(inbuf),
					infile);

			if (ferror(infile)) {
				fprintf(stderr, "Read error: %s\n",
						strerror(errno));
				return false;
			}

			if (feof(infile))
				action = LZMA_FINISH;
		}

		lzma_ret ret = lzma_code(strm, action);

		if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t write_size = sizeof(outbuf) - strm->avail_out;
			if (fwrite(outbuf, 1, write_size, outfile)
					!= write_size) {
				fprintf(stderr, "Write error: %s\n",
						strerror(errno));
				return false;
			}
			cmpSize += write_size;

			strm->next_out = outbuf;
			strm->avail_out = sizeof(outbuf);
		}

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END)
				return true;
			return false;
		}
	}
}

static void printusage()
{
	printf("Usage:\n"
		"dolxz <in.dol> <out.dol> <system> <additional options>\n\n"
		"Systems:\n"
		"-cube - Create GameCube compatible DOL File\n"
		"-wii - Create Wii compatible DOL File\n\n"
		"Additional Options:\n"
		"-high - Use 0x81300000 as Entrypoint instead of 0x80003100/0x80003400\n");
}

enum
{
	SYS_NONE = 0,
	SYS_CUBE,
	SYS_WII
};

int sysin = SYS_NONE;
int high = 0;
static void parseCmds(int argc, char *argv[])
{
	int i;
	for(i = 1; i < argc; i++)
	{
		if(memcmp(argv[i],"-cube",6) == 0)
			sysin = SYS_CUBE;
		else if(memcmp(argv[i],"-wii",5) == 0)
			sysin = SYS_WII;
		else if(memcmp(argv[i],"-high",6) == 0)
			high = 1;
	}
}

int main(int argc, char *argv[])
{
	printf("dolxz v1.2 by FIX94\n\n");

	if(argc < 4)
	{
		printusage();
		return -1;
	}

	if(strstr(argv[1],".dol") == NULL)
	{
		printf("Input is not a dol file!\n");
		printusage();
		return -2;
	}

	if(strstr(argv[2],".dol") == NULL)
	{
		printf("Output is not a dol file!\n");
		printusage();
		return -3;
	}

	parseCmds(argc, argv);
	if(sysin == SYS_NONE)
	{
		printf("No system specified!\n");
		printusage();
		return -3;
	}

	int loader_size = 0;
	const void *loader_dol = NULL;
	if(sysin == SYS_CUBE)
	{
		printf("Building GameCube DOL\n");
		if(high)
		{
			loader_size = loader_cube_high_size;
			loader_dol = loader_cube_high;
		}
		else
		{
			loader_size = loader_cube_size;
			loader_dol = loader_cube;
		}
	}
	else
	{
		printf("Building Wii DOL\n");
		if(high)
		{
			loader_size = loader_wii_high_size;
			loader_dol = loader_wii_high;
		}
		else
		{
			loader_size = loader_wii_size;
			loader_dol = loader_wii;
		}
	}
	unsigned int dolMemPos = 0x80020000;
	if(high)
	{
		printf("Using High Entrypoint\n");
		dolMemPos = 0x81320000;
	}
	printf(" \nPreparing...\n");

	char *dolBuf = malloc(loader_size);
	memcpy(dolBuf, loader_dol, loader_size);

	dolheader *dolfile = (dolheader *)dolBuf;
	int dolDataLoc = -1;
	int dolWritePos = 0;
	int i;
	for (i = 0; i < 11; i++)
	{
		if (__builtin_bswap32(dolfile->data_start[i]) == dolMemPos) {
			dolWritePos = __builtin_bswap32(dolfile->data_pos[i]);
			dolDataLoc = i;
			break;
		}
	}
	if(dolDataLoc < 0 || !dolWritePos)
	{
		printf("Internal error, should never happen...\n");
		free(dolBuf);
		return -3;
	}

	FILE *inF = fopen(argv[1],"rb");
	if(!inF)
	{
		printf("Failed to open %s!\n", argv[2]);
		free(dolBuf);
		return -4;
	}
	fseek(inF,0,SEEK_END);
	size_t inF_size = ftell(inF);
	fseek(inF,0,SEEK_SET);

	FILE *outF = fopen(argv[2],"wb");
	if(!outF)
	{
		printf("Failed to write to %s!\n", argv[2]);
		free(dolBuf);
		fclose(inF);
		return -5;
	}

	fwrite(dolBuf,1,dolWritePos,outF);
	//compress start
	printf("Compressing %s...\n", argv[1]);
	lzma_stream strm = LZMA_STREAM_INIT;
	bool success = init_encoder(&strm);
	if (success)
		success = compress(&strm, inF, outF);
	if(!success)
	{
		printf("Failed to compress %s!\n", argv[1]);
		free(dolBuf);
		fclose(outF);
		fclose(inF);
		return -5;
	}
	fclose(inF);
	size_t cmpTotalDol = cmpSize + dolWritePos;
	float cmpPercent = (((float)cmpTotalDol) / ((float)inF_size)) * 100.f;
	printf("Compressed %i bytes to %i bytes (%.02f%%)\n", inF_size, cmpTotalDol, cmpPercent);

	//write back compressed size into loader
	cmpSize = __builtin_bswap32(cmpSize);
	dolfile->data_size[dolDataLoc] = cmpSize;
	memcpy(dolBuf+0x124,&cmpSize,4);

	printf("Updating loader...\n");
	fseek(outF,0,SEEK_SET);
	fwrite(dolBuf,1,0x180,outF);
	fclose(outF);
	free(dolBuf);
	printf("Done!\n");

	return 0;
}
