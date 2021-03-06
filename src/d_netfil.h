// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief File transferring related structs and functions.

#ifndef __D_NETFIL__
#define __D_NETFIL__

#include "w_wad.h"

typedef enum
{
	SF_FILE,
	SF_Z_RAM,
	SF_RAM,
	SF_NOFREERAM
} freemethod_t;

typedef enum
{
	FS_NOTFOUND,
	FS_FOUND,
	FS_REQUESTED,
	FS_DOWNLOADING,
	FS_OPEN, // is opened and used in w_wad
	FS_MD5SUMBAD
} filestatus_t;

typedef struct
{
	UINT8 important;
	UINT8 willsend; // is the server willing to send it?
	char filename[MAX_WADPATH];
	UINT8 md5sum[16];
	// used only for download
	FILE *phandle;
	UINT32 currentsize;
	UINT32 totalsize;
	filestatus_t status; // the value returned by recsearch
} fileneeded_t;

extern INT32 fileneedednum;
extern fileneeded_t fileneeded[MAX_WADFILES];
extern char downloaddir[256];

UINT8 *PutFileNeeded(void);
void D_ParseFileneeded(INT32 fileneedednum_parm, UINT8 *fileneededstr);
void CL_PrepareDownloadSaveGame(const char *tmpsave);

// check file list in wadfiles return 0 when a file is not found
//                                    1 if all file are found
//                                    2 if you cannot connect (different wad version or
//                                                   no enought space to download files)
INT32 CL_CheckFiles(void);
void CL_LoadServerFiles(void);
void SendRam(INT32 node, void *data, size_t size, freemethod_t freemethod,
	UINT8 fileid);

void FiletxTicker(void);
void Got_Filetxpak(void);

boolean SendRequestFile(void);
void Got_RequestFilePak(INT32 node);

void AbortSendFiles(INT32 node);
void CloseNetFile(void);

boolean fileexist(char *filename, time_t ptime);

// search a file in the wadpath, return FS_FOUND when found
filestatus_t findfile(char *filename, const UINT8 *wantedmd5sum,
	boolean completepath);
filestatus_t checkfilemd5(char *filename, const UINT8 *wantedmd5sum);

void nameonly(char *s);
size_t nameonlylength(const char *s);

#endif // __D_NETFIL__
