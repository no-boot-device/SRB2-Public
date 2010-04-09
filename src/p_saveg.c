// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
/// \brief Archiving: SaveGame I/O

#include "doomdef.h"
#include "byteptr.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_random.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "r_things.h"
#include "r_state.h"
#include "w_wad.h"
#include "y_inter.h"
#include "z_zone.h"
#include "r_main.h"
#include "r_sky.h"
#include "p_polyobj.h"

savedata_t savedata;
byte *save_p;

// Note: This cannot be bigger
// than an UINT16
typedef enum
{
	RFLAGPOINT = 0x01,
	BFLAGPOINT = 0x02,
	CAPSULE    = 0x04,
	AWAYVIEW   = 0x08,
	FIRSTAXIS  = 0x10,
	SECONDAXIS = 0x20,
} player_saveflags;

//
// P_ArchivePlayer
//
static inline void P_ArchivePlayer(void)
{
	const player_t *player = &players[consoleplayer];

	WRITEBYTE(save_p, player->skincolor);
	WRITEBYTE(save_p, player->skin);

	WRITELONG(save_p, player->score);
	WRITELONG(save_p, player->lives);
	WRITELONG(save_p, player->continues);
}

//
// P_UnArchivePlayer
//
static inline void P_UnArchivePlayer(void)
{
	savedata.skincolor = READBYTE(save_p);
	savedata.skin = READBYTE(save_p);

	savedata.score = READLONG(save_p);
	savedata.lives = READLONG(save_p);
	savedata.continues = READLONG(save_p);
}

//
// P_NetArchivePlayers
//
static inline void P_NetArchivePlayers(void)
{
	INT32 i, j, flags;
	size_t q;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		flags = 0;

		WRITEANGLE(save_p, players[i].aiming);
		WRITEANGLE(save_p, players[i].awayviewaiming);
		WRITELONG(save_p, players[i].awayviewtics);
		WRITELONG(save_p, players[i].health);

		WRITELONG(save_p, players[i].currentweapon);
		WRITELONG(save_p, players[i].ringweapons);
		WRITEFIXED(save_p, players[i].tossstrength);

		for (j = 0; j < NUMPOWERS; j++)
			WRITELONG(save_p, players[i].powers[j]);

		WRITEBYTE(save_p, players[i].playerstate);
		WRITELONG(save_p, players[i].pflags);
		WRITEBYTE(save_p, players[i].spectator);

		WRITELONG(save_p, players[i].bonuscount);
		WRITELONG(save_p, players[i].skincolor);
		WRITELONG(save_p, players[i].skin);
		WRITEULONG(save_p, players[i].score);
		WRITELONG(save_p, players[i].dashspeed);
		WRITELONG(save_p, players[i].lives);
		WRITELONG(save_p, players[i].continues);
		WRITELONG(save_p, players[i].xtralife);
		WRITELONG(save_p, players[i].speed);
		WRITELONG(save_p, players[i].jumping);
		WRITEBYTE(save_p, players[i].secondjump);
		WRITELONG(save_p, players[i].fly1);
		WRITEULONG(save_p, players[i].scoreadd);
		WRITEULONG(save_p, players[i].glidetime);
		WRITELONG(save_p, players[i].climbing);
		WRITELONG(save_p, players[i].deadtimer);
		WRITELONG(save_p, players[i].splish);
		WRITEULONG(save_p, players[i].exiting);
		WRITELONG(save_p, players[i].blackow);
		WRITEBYTE(save_p, players[i].homing);

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		WRITEFIXED(save_p, players[i].cmomx); // Conveyor momx
		WRITEFIXED(save_p, players[i].cmomy); // Conveyor momy
		WRITEFIXED(save_p, players[i].rmomx); // "Real" momx (momx - cmomx)
		WRITEFIXED(save_p, players[i].rmomy); // "Real" momy (momy - cmomy)

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		WRITELONG(save_p, players[i].numboxes);
		WRITELONG(save_p, players[i].totalring);
		WRITEULONG(save_p, players[i].realtime);
		WRITEULONG(save_p, players[i].racescore);
		WRITEULONG(save_p, players[i].laps);

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].tagzone);
		WRITELONG(save_p, players[i].taglag);

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].ctfteam);
		WRITEUSHORT(save_p, players[i].gotflag);

		WRITELONG(save_p, players[i].dbginfo);
		WRITELONG(save_p, players[i].emeraldhunt);

		WRITELONG(save_p, players[i].weapondelay);
		WRITELONG(save_p, players[i].tossdelay);
		WRITELONG(save_p, players[i].shielddelay);
		WRITEULONG(save_p, players[i].taunttimer);

		WRITEULONG(save_p, players[i].starposttime);
		WRITELONG(save_p, players[i].starpostx);
		WRITELONG(save_p, players[i].starposty);
		WRITELONG(save_p, players[i].starpostz);
		WRITELONG(save_p, players[i].starpostnum);
		WRITEANGLE(save_p, players[i].starpostangle);
		WRITEULONG(save_p, players[i].starpostbit);

		WRITEANGLE(save_p, players[i].angle_pos);
		WRITEANGLE(save_p, players[i].old_angle_pos);

		WRITELONG(save_p, players[i].flyangle);
		WRITEULONG(save_p, players[i].drilltimer);
		WRITELONG(save_p, players[i].linkcount);
		WRITEULONG(save_p, players[i].linktimer);
		WRITELONG(save_p, players[i].anotherflyangle);
		WRITEULONG(save_p, players[i].nightstime);
		WRITEULONG(save_p, players[i].bumpertime);
		WRITELONG(save_p, players[i].drillmeter);
		WRITEBYTE(save_p, players[i].drilldelay);
		WRITEBYTE(save_p, players[i].bonustime);
		WRITEBYTE(save_p, players[i].mare);

		if (players[i].capsule)
			flags |= CAPSULE;

		if (players[i].awayviewmobj)
			flags |= AWAYVIEW;

		if (players[i].axis1)
			flags |= FIRSTAXIS;

		if (players[i].axis2)
			flags |= SECONDAXIS;

		WRITESHORT(save_p, players[i].lastsidehit);
		WRITESHORT(save_p, players[i].lastlinehit);


		WRITELONG(save_p, players[i].losscount);

		WRITELONG(save_p, players[i].onconveyor);

		WRITEULONG(save_p, players[i].jointime);

		if (i == 0)
		{
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == rflagpoint)
				{
					flags |= RFLAGPOINT;
					break;
				}
			}

			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == bflagpoint)
				{
					flags |= BFLAGPOINT;
					break;
				}
			}
		}

		WRITEUSHORT(save_p, flags);

		if (flags & CAPSULE)
			WRITEULONG(save_p, players[i].capsule->mobjnum);

		if (flags & FIRSTAXIS)
			WRITEULONG(save_p, players[i].axis1->mobjnum);

		if (flags & SECONDAXIS)
			WRITEULONG(save_p, players[i].axis2->mobjnum);

		if (flags & AWAYVIEW)
			WRITEULONG(save_p, players[i].awayviewmobj->mobjnum);

		if (flags & RFLAGPOINT)
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == rflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}

		if (flags & BFLAGPOINT)
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == bflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}
	}
}

//
// P_NetUnArchivePlayers
//
static inline void P_NetUnArchivePlayers(void)
{
	INT32 i, j, flags;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		memset(&players[i], 0, sizeof (player_t));
		if (!playeringame[i])
			continue;

		players[i].aiming = READANGLE(save_p);
		players[i].awayviewaiming = READANGLE(save_p);
		players[i].awayviewtics = READLONG(save_p);
		players[i].health = READLONG(save_p);

		players[i].currentweapon = READLONG(save_p);
		players[i].ringweapons = READLONG(save_p);
		players[i].tossstrength = READFIXED(save_p);

		for (j = 0; j < NUMPOWERS; j++)
			players[i].powers[j] = READLONG(save_p);

		players[i].playerstate = READBYTE(save_p);
		players[i].pflags = READLONG(save_p);
		players[i].spectator = READBYTE(save_p);

		players[i].bonuscount = READLONG(save_p);
		players[i].skincolor = READLONG(save_p);
		players[i].skin = READLONG(save_p);
		players[i].score = READULONG(save_p);
		players[i].dashspeed = READLONG(save_p); // dashing speed
		players[i].lives = READLONG(save_p);
		players[i].continues = READLONG(save_p); // continues that player has acquired
		players[i].xtralife = READLONG(save_p); // Ring Extra Life counter
		players[i].speed = READLONG(save_p); // Player's speed (distance formula of MOMX and MOMY values)
		players[i].jumping = READLONG(save_p); // Jump counter
		players[i].secondjump = READBYTE(save_p);
		players[i].fly1 = READLONG(save_p); // Tails flying
		players[i].scoreadd = READULONG(save_p); // Used for multiple enemy attack bonus
		players[i].glidetime = READULONG(save_p); // Glide counter for thrust
		players[i].climbing = READLONG(save_p); // Climbing on the wall
		players[i].deadtimer = READLONG(save_p); // End game if game over lasts too long
		players[i].splish = READLONG(save_p); // Don't make splish repeat tons
		players[i].exiting = READULONG(save_p); // Exitlevel timer
		players[i].blackow = READLONG(save_p);
		players[i].homing = READBYTE(save_p); // Are you homing?

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		players[i].cmomx = READFIXED(save_p); // Conveyor momx
		players[i].cmomy = READFIXED(save_p); // Conveyor momy
		players[i].rmomx = READFIXED(save_p); // "Real" momx (momx - cmomx)
		players[i].rmomy = READFIXED(save_p); // "Real" momy (momy - cmomy)

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		players[i].numboxes = READLONG(save_p); // Number of item boxes obtained for Race Mode
		players[i].totalring = READLONG(save_p); // Total number of rings obtained for Race Mode
		players[i].realtime = READULONG(save_p); // integer replacement for leveltime
		players[i].racescore = READULONG(save_p); // Total of won categories
		players[i].laps = READULONG(save_p); // Number of laps (optional)

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		players[i].tagzone = READLONG(save_p); // Tag Zone timer
		players[i].taglag = READLONG(save_p); // Don't go back in the tag zone too early

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		players[i].ctfteam = READLONG(save_p); // 1 == Red, 2 == Blue
		players[i].gotflag = READUSHORT(save_p); // 1 == Red, 2 == Blue Do you have the flag?

		players[i].dbginfo = READLONG(save_p); // Debugger
		players[i].emeraldhunt = READLONG(save_p); // # of emeralds found

		players[i].weapondelay = READLONG(save_p);
		players[i].tossdelay = READLONG(save_p);
		players[i].shielddelay = READLONG(save_p);
		players[i].taunttimer = READULONG(save_p);

		players[i].starposttime = READULONG(save_p);
		players[i].starpostx = READLONG(save_p);
		players[i].starposty = READLONG(save_p);
		players[i].starpostz = READLONG(save_p);
		players[i].starpostnum = READLONG(save_p);
		players[i].starpostangle = READANGLE(save_p);
		players[i].starpostbit = READULONG(save_p);

		players[i].angle_pos = READANGLE(save_p);
		players[i].old_angle_pos = READANGLE(save_p);

		players[i].flyangle = READLONG(save_p);
		players[i].drilltimer = READULONG(save_p);
		players[i].linkcount = READLONG(save_p);
		players[i].linktimer = READULONG(save_p);
		players[i].anotherflyangle = READLONG(save_p);
		players[i].nightstime = READULONG(save_p);
		players[i].bumpertime = READULONG(save_p);
		players[i].drillmeter = READLONG(save_p);
		players[i].drilldelay = READBYTE(save_p);
		players[i].bonustime = READBYTE(save_p);
		players[i].mare = READBYTE(save_p);

		players[i].lastsidehit = READSHORT(save_p);
		players[i].lastlinehit = READSHORT(save_p);

		players[i].losscount = READLONG(save_p);

		players[i].onconveyor = READLONG(save_p);

		players[i].jointime = READULONG(save_p);

		flags = READUSHORT(save_p);

		if (flags & CAPSULE)
			players[i].capsule = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & FIRSTAXIS)
			players[i].axis1 = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & SECONDAXIS)
			players[i].axis2 = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & AWAYVIEW)
			players[i].awayviewmobj = (mobj_t *)(size_t)READULONG(save_p);

		if (i == 0)
		{
			if (flags & RFLAGPOINT)
				rflagpoint = &mapthings[READULONG(save_p)];

			if (flags & BFLAGPOINT)
				bflagpoint = &mapthings[READULONG(save_p)];
		}

		players[i].viewheight = cv_viewheight.value<<FRACBITS;

		SetPlayerSkinByNum(i, players[i].skin);
	}
}

#define SD_FLOORHT  0x01
#define SD_CEILHT   0x02
#define SD_FLOORPIC 0x04
#define SD_CEILPIC  0x08
#define SD_LIGHT    0x10
#define SD_SPECIAL  0x20
#define SD_DIFF2    0x40

// diff2 flags
#define SD_FXOFFS    0x01
#define SD_FYOFFS    0x02
#define SD_CXOFFS    0x04
#define SD_CYOFFS    0x08
#define SD_TAG       0x10
#define SD_FLOORANG  0x20
#define SD_CEILANG   0x40

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x80

// diff2 flags
#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08

//
// P_NetArchiveWorld
//
static void P_NetArchiveWorld(void)
{
	size_t i;
	INT32 statsec = 0, statline = 0;
	const line_t *li = lines;
	const side_t *si;
	byte *put = save_p;

	// reload the map just to see difference
	const mapsector_t *ms;
	const mapsidedef_t *msd;
	const maplinedef_t *mld;
	const sector_t *ss = sectors;
	byte diff, diff2;

	ms = W_CacheLumpNum(lastloadedmaplumpnum+ML_SECTORS, PU_CACHE);

	for (i = 0; i < numsectors; i++, ss++, ms++)
	{
		diff = diff2 = 0;
		if (ss->floorheight != SHORT(ms->floorheight)<<FRACBITS)
			diff |= SD_FLOORHT;
		if (ss->ceilingheight != SHORT(ms->ceilingheight)<<FRACBITS)
			diff |= SD_CEILHT;
		//
		// flats
		//
		// P_AddLevelFlat should not add but just return the number
		if (ss->floorpic != P_AddLevelFlat(ms->floorpic, levelflats))
			diff |= SD_FLOORPIC;
		if (ss->ceilingpic != P_AddLevelFlat(ms->ceilingpic, levelflats))
			diff |= SD_CEILPIC;

		if (ss->lightlevel != SHORT(ms->lightlevel))
			diff |= SD_LIGHT;
		if (ss->special != SHORT(ms->special))
			diff |= SD_SPECIAL;

		/// \todo this makes Flat Alignment (linetype 7) increase the savegame size!
		if (ss->floor_xoffs != 0)
			diff2 |= SD_FXOFFS;
		if (ss->floor_yoffs != 0)
			diff2 |= SD_FYOFFS;
		if (ss->ceiling_xoffs != 0)
			diff2 |= SD_CXOFFS;
		if (ss->ceiling_yoffs != 0)
			diff2 |= SD_CYOFFS;
		if (ss->floorpic_angle != 0)
			diff2 |= SD_FLOORANG;
		if (ss->ceilingpic_angle != 0)
			diff2 |= SD_CEILANG;

		if (ss->tag != SHORT(ms->tag))
			diff2 |= SD_TAG;

		if (diff2)
			diff |= SD_DIFF2;

		if (diff)
		{
			statsec++;

			WRITEUSHORT(put, i);
			WRITEBYTE(put, diff);
			if (diff & SD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & SD_FLOORHT)
				WRITEFIXED(put, ss->floorheight);
			if (diff & SD_CEILHT)
				WRITEFIXED(put, ss->ceilingheight);
			if (diff & SD_FLOORPIC)
				WRITEMEM(put, levelflats[ss->floorpic].name, 8);
			if (diff & SD_CEILPIC)
				WRITEMEM(put, levelflats[ss->ceilingpic].name, 8);
			if (diff & SD_LIGHT)
				WRITESHORT(put, ss->lightlevel);
			if (diff & SD_SPECIAL)
				WRITESHORT(put, ss->special);
			if (diff2 & SD_FXOFFS)
				WRITEFIXED(put, ss->floor_xoffs);
			if (diff2 & SD_FYOFFS)
				WRITEFIXED(put, ss->floor_yoffs);
			if (diff2 & SD_CXOFFS)
				WRITEFIXED(put, ss->ceiling_xoffs);
			if (diff2 & SD_CYOFFS)
				WRITEFIXED(put, ss->ceiling_yoffs);
			if (diff2 & SD_TAG)
			{
				WRITESHORT(put, ss->tag);
				WRITELONG(put, ss->firsttag);
				WRITELONG(put, ss->nexttag);
			}
			if (diff2 & SD_FLOORANG)
				WRITEANGLE(put, ss->floorpic_angle);
			if (diff2 & SD_CEILANG)
				WRITEANGLE(put, ss->ceilingpic_angle);
		}
	}

	WRITEUSHORT(put, 0xffff);

	mld = W_CacheLumpNum(lastloadedmaplumpnum+ML_LINEDEFS, PU_CACHE);
	msd = W_CacheLumpNum(lastloadedmaplumpnum+ML_SIDEDEFS, PU_CACHE);
	// do lines
	for (i = 0; i < numlines; i++, mld++, li++)
	{
		diff = diff2 = 0;

		if (li->special != SHORT(mld->special))
			diff |= LD_SPECIAL;

		if (li->sidenum[0] != 0xffff)
		{
			si = &sides[li->sidenum[0]];
			if (si->textureoffset != SHORT(msd[li->sidenum[0]].textureoffset)<<FRACBITS)
				diff |= LD_S1TEXOFF;
			//SoM: 4/1/2000: Some textures are colormaps. Don't worry about invalid textures.
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].toptexture, li->sidenum[0]) != -1
				&& si->toptexture != R_TextureNumForName(msd[li->sidenum[0]].toptexture, li->sidenum[0]))
				diff |= LD_S1TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].bottomtexture, li->sidenum[0]) != -1
				&& si->bottomtexture != R_TextureNumForName(msd[li->sidenum[0]].bottomtexture, li->sidenum[0]))
				diff |= LD_S1BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].midtexture, li->sidenum[0]) != -1
				&& si->midtexture != R_TextureNumForName(msd[li->sidenum[0]].midtexture, li->sidenum[0]))
				diff |= LD_S1MIDTEX;
		}
		if (li->sidenum[1] != 0xffff)
		{
			si = &sides[li->sidenum[1]];
			if (si->textureoffset != SHORT(msd[li->sidenum[1]].textureoffset)<<FRACBITS)
				diff2 |= LD_S2TEXOFF;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].toptexture, li->sidenum[1]) != -1
				&& si->toptexture != R_TextureNumForName(msd[li->sidenum[1]].toptexture, li->sidenum[1]))
				diff2 |= LD_S2TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].bottomtexture, li->sidenum[1]) != -1
				&& si->bottomtexture != R_TextureNumForName(msd[li->sidenum[1]].bottomtexture, li->sidenum[1]))
				diff2 |= LD_S2BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].midtexture, li->sidenum[1]) != -1
				&& si->midtexture != R_TextureNumForName(msd[li->sidenum[1]].midtexture, li->sidenum[1]))
				diff2 |= LD_S2MIDTEX;
			if (diff2)
				diff |= LD_DIFF2;
		}

		if (diff)
		{
			statline++;
			WRITESHORT(put, (short)i);
			WRITEBYTE(put, diff);
			if (diff & LD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & LD_FLAG)
				WRITESHORT(put, li->flags);
			if (diff & LD_SPECIAL)
				WRITESHORT(put, li->special);

			si = &sides[li->sidenum[0]];
			if (diff & LD_S1TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff & LD_S1TOPTEX)
				WRITELONG(put, si->toptexture);
			if (diff & LD_S1BOTTEX)
				WRITELONG(put, si->bottomtexture);
			if (diff & LD_S1MIDTEX)
				WRITELONG(put, si->midtexture);

			si = &sides[li->sidenum[1]];
			if (diff2 & LD_S2TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff2 & LD_S2TOPTEX)
				WRITELONG(put, si->toptexture);
			if (diff2 & LD_S2BOTTEX)
				WRITELONG(put, si->bottomtexture);
			if (diff2 & LD_S2MIDTEX)
				WRITELONG(put, si->midtexture);
		}
	}
	WRITEUSHORT(put, 0xffff);

	save_p = put;
}

//
// P_NetUnArchiveWorld
//
static void P_NetUnArchiveWorld(void)
{
	ULONG i;
	line_t *li;
	side_t *si;
	byte *get;
	byte diff, diff2;

	get = save_p;

	for (;;)
	{
		i = READUSHORT(get);

		if (i == 0xffff)
			break;

		if (i > numsectors)
			I_Error("Invalid sector number %u from server (expected end at %"PRIdS")", i, numsectors);

		diff = READBYTE(get);
		if (diff & SD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;

		if (diff & SD_FLOORHT)
			sectors[i].floorheight = READFIXED(get);
		if (diff & SD_CEILHT)
			sectors[i].ceilingheight = READFIXED(get);
		if (diff & SD_FLOORPIC)
		{
			sectors[i].floorpic = P_AddLevelFlat((char *)get, levelflats);
			get += 8;
		}
		if (diff & SD_CEILPIC)
		{
			sectors[i].ceilingpic = P_AddLevelFlat((char *)get, levelflats);
			get += 8;
		}
		if (diff & SD_LIGHT)
			sectors[i].lightlevel = READSHORT(get);
		if (diff & SD_SPECIAL)
			sectors[i].special = READSHORT(get);

		if (diff2 & SD_FXOFFS)
			sectors[i].floor_xoffs = READFIXED(get);
		if (diff2 & SD_FYOFFS)
			sectors[i].floor_yoffs = READFIXED(get);
		if (diff2 & SD_CXOFFS)
			sectors[i].ceiling_xoffs = READFIXED(get);
		if (diff2 & SD_CYOFFS)
			sectors[i].ceiling_yoffs = READFIXED(get);
		if (diff2 & SD_TAG)
		{
			short tag;
			tag = READSHORT(get);
			sectors[i].firsttag = READLONG(get);
			sectors[i].nexttag = READLONG(get);
			P_ChangeSectorTag(i, tag);
		}
		if (diff2 & SD_FLOORANG)
			sectors[i].floorpic_angle  = READANGLE(get);
		if (diff2 & SD_CEILANG)
			sectors[i].ceilingpic_angle = READANGLE(get);
	}

	for (;;)
	{
		i = READUSHORT(get);

		if (i == 0xffff)
			break;
		if (i > numlines)
			I_Error("Invalid line number %u from server", i);

		diff = READBYTE(get);
		li = &lines[i];

		if (diff & LD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;
		if (diff & LD_FLAG)
			li->flags = READSHORT(get);
		if (diff & LD_SPECIAL)
			li->special = READSHORT(get);

		si = &sides[li->sidenum[0]];
		if (diff & LD_S1TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff & LD_S1TOPTEX)
			si->toptexture = READLONG(get);
		if (diff & LD_S1BOTTEX)
			si->bottomtexture = READLONG(get);
		if (diff & LD_S1MIDTEX)
			si->midtexture = READLONG(get);

		si = &sides[li->sidenum[1]];
		if (diff2 & LD_S2TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff2 & LD_S2TOPTEX)
			si->toptexture = READLONG(get);
		if (diff2 & LD_S2BOTTEX)
			si->bottomtexture = READLONG(get);
		if (diff2 & LD_S2MIDTEX)
			si->midtexture = READLONG(get);
	}

	save_p = get;
}

//
// Thinkers
//

typedef enum
{
	MD_SPAWNPOINT  = 0x0000001,
	MD_POS         = 0x0000002,
	MD_TYPE        = 0x0000004,
	MD_MOM         = 0x0000008,
	MD_RADIUS      = 0x0000010,
	MD_HEIGHT      = 0x0000020,
	MD_FLAGS       = 0x0000040,
	MD_HEALTH      = 0x0000080,
	MD_RTIME       = 0x0000100,
	MD_STATE       = 0x0000200,
	MD_TICS        = 0x0000400,
	MD_SPRITE      = 0x0000800,
	MD_FRAME       = 0x0001000,
	MD_EFLAGS      = 0x0002000,
	MD_PLAYER      = 0x0004000,
	MD_MOVEDIR     = 0x0008000,
	MD_MOVECOUNT   = 0x0010000,
	MD_THRESHOLD   = 0x0020000,
	MD_LASTLOOK    = 0x0040000,
	MD_TARGET      = 0x0080000,
	MD_TRACER      = 0x0100000,
	MD_FRICTION    = 0x0200000,
	MD_MOVEFACTOR  = 0x0400000,
	MD_FLAGS2      = 0x0800000,
	MD_FUSE        = 0x1000000,
	MD_WATERTOP    = 0x2000000,
	MD_WATERBOTTOM = 0x4000000,
	MD_SCALE       = 0x8000000,
	MD_DSCALE      = 0x10000000,
} mobj_diff_t;

typedef enum
{
	tc_mobj,
	tc_ceiling,
	tc_floor,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_fireflicker,
	tc_thwomp,
	tc_camerascanner,
	tc_elevator,
	tc_continuousfalling,
	tc_bouncecheese,
	tc_startcrumble,
	tc_marioblock,
	tc_spikesector,
	tc_floatsector,
	tc_bridgethinker,
	tc_crushceiling,
	tc_scroll,
	tc_friction,
	tc_pusher,
	tc_laserflash,
	tc_lightfade,
	tc_executor,
	tc_raisesector,
	tc_noenemies,
	tc_eachtime,
	tc_disappear,
#ifdef POLYOBJECTS
	tc_polyrotate, // haleyjd 03/26/06: polyobjects
	tc_polymove,
	tc_polywaypoint,
	tc_polyslidedoor,
	tc_polyswingdoor,
	tc_polyflag,
#endif
	tc_end
} specials_e;

static inline ULONG SaveMobjnum(const mobj_t *mobj)
{
	if (mobj) return mobj->mobjnum;
	return 0;
}

static UINT32 SaveSector(const sector_t *sector)
{
	if (sector) return (UINT32)(sector - sectors);
	return 0xFFFFFFFF;
}

static UINT32 SaveLine(const line_t *line)
{
	if (line) return (UINT32)(line - lines);
	return 0xFFFFFFFF;
}

static inline UINT32 SavePlayer(const player_t *player)
{
	if (player) return (UINT32)(player - players);
	return 0xFFFFFFFF;
}

#ifndef REMOVE_FOR_205
static inline UINT32 SaveFfloor(const ffloor_t *pfloor, const sector_t *sector)
{
	if (sector && sector->ffloors && pfloor)
		return (UINT32)(pfloor - sector->ffloors);
	return 0xFFFFFFFF;
}
#endif

//
// SaveSpecialLevelThinker
//
// Saves a levelspecthink_t thinker
//
static void SaveSpecialLevelThinker(const thinker_t *th, const byte type)
{
	const levelspecthink_t *ht  = (const void *)th;
	size_t i;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->type);
	for (i = 0; i < 16; i++)
		WRITEFIXED(save_p, ht->vars[i]); //var[16]
	WRITEULONG(save_p, 0); //activator dummy
	WRITEULONG(save_p, SaveLine(ht->sourceline));
	WRITEULONG(save_p, SaveSector(ht->sector));
}

//
// SaveCeilingThinker
//
// Saves a ceiling_t thinker
//
static void SaveCeilingThinker(const thinker_t *th, const byte type)
{
	const ceiling_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->type);
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITEFIXED(save_p, ht->bottomheight);
	WRITEFIXED(save_p, ht->topheight);
	WRITEFIXED(save_p, ht->speed);
	WRITEFIXED(save_p, ht->oldspeed);
	WRITEFIXED(save_p, ht->delay);
	WRITEFIXED(save_p, ht->delaytimer);
	WRITELONG(save_p, ht->crush);
	WRITELONG(save_p, ht->texture);
	WRITELONG(save_p, ht->direction);
	WRITELONG(save_p, ht->tag);
	WRITELONG(save_p, ht->olddirection);
	WRITELONG(save_p, 0);  //*list dummy
	WRITEFIXED(save_p, ht->origspeed);
	WRITEFIXED(save_p, ht->sourceline);
}

//
// SaveFloormoveThinker
//
// Saves a floormove_t thinker
//
static inline void SaveFloormoveThinker(const thinker_t *th, const byte type)
{
	const floormove_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->type);
	WRITELONG(save_p, ht->crush);
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->direction);
	WRITELONG(save_p, ht->texture);
	WRITEFIXED(save_p, ht->floordestheight);
	WRITEFIXED(save_p, ht->speed);
	WRITEFIXED(save_p, ht->origspeed);
	WRITEFIXED(save_p, ht->delay);
	WRITEFIXED(save_p, ht->delaytimer);
}

//
// SaveLightflashThinker
//
// Saves a lightflash_t thinker
//
static inline void SaveLightflashThinker(const thinker_t *th, const byte type)
{
	const lightflash_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->maxlight);
	WRITELONG(save_p, ht->minlight);
}

//
// SaveStrobeThinker
//
// Saves a strobe_t thinker
//
static inline void SaveStrobeThinker(const thinker_t *th, const byte type)
{
	const strobe_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->count);
	WRITELONG(save_p, ht->minlight);
	WRITELONG(save_p, ht->maxlight);
	WRITELONG(save_p, ht->darktime);
	WRITELONG(save_p, ht->brighttime);
}

//
// SaveGlowThinker
//
// Saves a glow_t thinker
//
static inline void SaveGlowThinker(const thinker_t *th, const byte type)
{
	const glow_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->minlight);
	WRITELONG(save_p, ht->maxlight);
	WRITELONG(save_p, ht->direction);
	WRITELONG(save_p, ht->speed);
}
//
// SaveFireflickerThinker
//
// Saves a fireflicker_t thinker
//
static inline void SaveFireflickerThinker(const thinker_t *th, const byte type)
{
	const fireflicker_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->count);
	WRITELONG(save_p, ht->resetcount);
	WRITELONG(save_p, ht->maxlight);
	WRITELONG(save_p, ht->minlight);
}
//
// SaveElevatorThinker
//
// Saves a elevator_t thinker
//
static void SaveElevatorThinker(const thinker_t *th, const byte type)
{
	const elevator_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->type);
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITEULONG(save_p, SaveSector(ht->actionsector));
	WRITELONG(save_p, ht->direction);
	WRITEFIXED(save_p, ht->floordestheight);
	WRITEFIXED(save_p, ht->ceilingdestheight);
	WRITEFIXED(save_p, ht->speed);
	WRITEFIXED(save_p, ht->origspeed);
	WRITEFIXED(save_p, ht->low);
	WRITEFIXED(save_p, ht->high);
	WRITEFIXED(save_p, ht->distance);
	WRITEFIXED(save_p, ht->delay);
	WRITEFIXED(save_p, ht->delaytimer);
	WRITEFIXED(save_p, ht->floorwasheight);
	WRITEFIXED(save_p, ht->ceilingwasheight);
	WRITEULONG(save_p, SavePlayer(ht->player)); // was dummy
	WRITEULONG(save_p, SaveLine(ht->sourceline));
}

//
// SaveScrollThinker
//
// Saves a scroll_t thinker
//
static inline void SaveScrollThinker(const thinker_t *th, const byte type)
{
	const scroll_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEFIXED(save_p, ht->dx);
	WRITEFIXED(save_p, ht->dy);
	WRITELONG(save_p, ht->affectee);
	WRITELONG(save_p, ht->control);
	WRITEFIXED(save_p, ht->last_height);
	WRITEFIXED(save_p, ht->vdx);
	WRITEFIXED(save_p, ht->vdy);
	WRITELONG(save_p, ht->accel);
	WRITELONG(save_p, ht->exclusive);
	WRITELONG(save_p, ht->type);
}

//
// SaveFrictionThinker
//
// Saves a friction_t thinker
//
static inline void SaveFrictionThinker(const thinker_t *th, const byte type)
{
	const friction_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->friction);
	WRITELONG(save_p, ht->movefactor);
	WRITELONG(save_p, ht->affectee);
	WRITELONG(save_p, ht->referrer);
	WRITEBYTE(save_p, ht->roverfriction);
	WRITEBYTE(save_p, 0x00); //gap dummy
	WRITEBYTE(save_p, 0x00); //gap dummy
	WRITEBYTE(save_p, 0x00); //gap dummy
}

//
// SavePusherThinker
//
// Saves a pusher_t thinker
//
static inline void SavePusherThinker(const thinker_t *th, const byte type)
{
	const pusher_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->type);
	WRITEULONG(save_p, 0); //source dummy, used affectee
	WRITELONG(save_p, ht->x_mag);
	WRITELONG(save_p, ht->y_mag);
	WRITELONG(save_p, ht->magnitude);
	WRITELONG(save_p, ht->radius);
	WRITELONG(save_p, ht->x);
	WRITELONG(save_p, ht->y);
	WRITELONG(save_p, ht->z);
	WRITELONG(save_p, ht->affectee);
	WRITEBYTE(save_p, ht->roverpusher);
	WRITEBYTE(save_p, 0x00); //gap dummy
	WRITEBYTE(save_p, 0x00); //gap dummy
	WRITEBYTE(save_p, 0x00); //gap dummy
	WRITELONG(save_p, ht->referrer);
	WRITELONG(save_p, ht->exclusive);
	WRITELONG(save_p, ht->slider);
}

//
// SaveLaserThinker
//
// Saves a laserthink_t thinker
//
static void SaveLaserThinker(const thinker_t *th, const byte type)
{
	const laserthink_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
#ifndef REMOVE_FOR_205
	WRITEULONG(save_p, SaveFfloor(ht->ffloor, ht->sector)); ///< TODO: remove
#endif
	WRITEULONG(save_p, SaveSector(ht->sector));
#ifdef REMOVE_FOR_205
	WRITEULONG(save_p, SaveSector(ht->sec));
	WRITEULONG(save_p, SaveLine(ht->sourceline));
#endif
}

//
// SaveLightlevelThinker
//
// Saves a lightlevel_t thinker
//
static void SaveLightlevelThinker(const thinker_t *th, const byte type)
{
	const lightlevel_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveSector(ht->sector));
	WRITELONG(save_p, ht->destlevel);
	WRITELONG(save_p, ht->speed);
}

//
// SaveExecutorThinker
//
// Saves a executor_t thinker
//
static void SaveExecutorThinker(const thinker_t *th, const byte type)
{
	const executor_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, SaveLine(ht->line));
	WRITEULONG(save_p, SaveMobjnum(ht->caller));
	WRITELONG(save_p, ht->timer);
}

//
// SaveDisappearThinker
//
// Saves a disappear_t thinker
//
static void SaveDisappearThinker(const thinker_t *th, const byte type)
{
	const disappear_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEULONG(save_p, ht->appeartime);
	WRITEULONG(save_p, ht->disappeartime);
	WRITEULONG(save_p, ht->offset);
	WRITEULONG(save_p, ht->timer);
	WRITELONG(save_p, ht->affectee);
	WRITELONG(save_p, ht->sourceline);
	WRITELONG(save_p, ht->exists);
}

#ifdef POLYOBJECTS

//
// SavePolyrotateThinker
//
// Saves a polyrotate_t thinker
//
static inline void SavePolyrotatetThinker(const thinker_t *th, const byte type)
{
	const polyrotate_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->polyObjNum);
	WRITELONG(save_p, ht->speed);
	WRITELONG(save_p, ht->distance);
}

//
// SavePolymoveThinker
//
// Saves a polymovet_t thinker
//
static void SavePolymoveThinker(const thinker_t *th, const byte type)
{
	const polymove_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->polyObjNum);
	WRITELONG(save_p, ht->speed);
	WRITEFIXED(save_p, ht->momx);
	WRITEFIXED(save_p, ht->momy);
	WRITELONG(save_p, ht->distance);
	WRITEULONG(save_p, ht->angle);
}

//
// SavePolywaypointThinker
//
// Saves a polywaypoint_t thinker
//
static void SavePolywaypointThinker(const thinker_t *th, byte type)
{
	const polywaypoint_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->polyObjNum);
	WRITELONG(save_p, ht->speed);
	WRITELONG(save_p, ht->sequence);
	WRITELONG(save_p, ht->pointnum);
	WRITELONG(save_p, ht->direction);
	WRITELONG(save_p, ht->comeback);
	WRITELONG(save_p, ht->wrap);
	WRITELONG(save_p, ht->continuous);
	WRITELONG(save_p, ht->stophere);
	WRITEFIXED(save_p, ht->diffx);
	WRITEFIXED(save_p, ht->diffy);
	WRITEFIXED(save_p, ht->diffz);
}

//
// SavePolyslidedoorThinker
//
// Saves a polyslidedoor_t thinker
//
static void SavePolyslidedoorThinker(const thinker_t *th, const byte type)
{
	const polyslidedoor_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->polyObjNum);
	WRITELONG(save_p, ht->delay);
	WRITELONG(save_p, ht->delayCount);
	WRITELONG(save_p, ht->initSpeed);
	WRITELONG(save_p, ht->speed);
	WRITELONG(save_p, ht->initDistance);
	WRITELONG(save_p, ht->distance);
	WRITEULONG(save_p, ht->initAngle);
	WRITEULONG(save_p, ht->angle);
	WRITEULONG(save_p, ht->revAngle);
	WRITEFIXED(save_p, ht->momx);
	WRITEFIXED(save_p, ht->momy);
	WRITELONG(save_p, ht->closing);
}

//
// SavePolyswingdoorThinker
//
// Saves a polyswingdoor_t thinker
//
static void SavePolyswingdoorThinker(const thinker_t *th, const byte type)
{
	const polyswingdoor_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITELONG(save_p, ht->polyObjNum);
	WRITELONG(save_p, ht->delay);
	WRITELONG(save_p, ht->delayCount);
	WRITELONG(save_p, ht->initSpeed);
	WRITELONG(save_p, ht->speed);
	WRITELONG(save_p, ht->initDistance);
	WRITELONG(save_p, ht->distance);
	WRITELONG(save_p, ht->closing);
}

#endif
/*
//
// SaveWhatThinker
//
// Saves a what_t thinker
//
static inline void SaveWhatThinker(const thinker_t *th, const byte type)
{
	const what_t *ht = (const void *)th;
	WRITEBYTE(save_p, type);
	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
}
*/

//
// P_NetArchiveThinkers
//
//
static void P_NetArchiveThinkers(void)
{
	const thinker_t *th;
	const mobj_t *mobj;
	ULONG diff;

	// save off the current thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (const mobj_t *)th;

			// Ignore stationary hoops - these will be respawned from mapthings.
			if (mobj->type == MT_HOOP)
				continue;

			// These are NEVER saved.
			if (mobj->type == MT_HOOPCOLLIDE)
				continue;

			// This hoop has already been collected.
			if (mobj->type == MT_HOOPCENTER && mobj->threshold == 4242)
				continue;

			if (mobj->spawnpoint && (mobj->info->doomednum != -1 || mobj->type == MT_HOOPCENTER))
			{
				// spawnpoint is not modified but we must save it since it is an identifier
				diff = MD_SPAWNPOINT;

				if (mobj->type != MT_HOOPCENTER)
				{
					if ((mobj->x != mobj->spawnpoint->x << FRACBITS) ||
						(mobj->y != mobj->spawnpoint->y << FRACBITS) ||
						(mobj->angle != (angle_t)(ANGLE_45 * (mobj->spawnpoint->angle/45))))
						diff |= MD_POS;

					if (mobj->info->doomednum != mobj->spawnpoint->type)
						diff |= MD_TYPE;
				}
			}
			else
				diff = MD_POS | MD_TYPE; // not a map spawned thing so make it from scratch

			// not the default but the most probable
			if (mobj->momx != 0 || mobj->momy != 0 || mobj->momz != 0)
				diff |= MD_MOM;
			if (mobj->radius != mobj->info->radius)
				diff |= MD_RADIUS;
			if (mobj->height != mobj->info->height)
				diff |= MD_HEIGHT;
			if (mobj->flags != mobj->info->flags)
				diff |= MD_FLAGS;
			diff |= MD_FLAGS2; // Force saving of flags2
			if (mobj->health != mobj->info->spawnhealth)
				diff |= MD_HEALTH;
			if (mobj->reactiontime != mobj->info->reactiontime)
				diff |= MD_RTIME;
			if ((statenum_t)(mobj->state-states) != mobj->info->spawnstate)
				diff |= MD_STATE;
			if (mobj->tics != mobj->state->tics)
				diff |= MD_TICS;
			if (mobj->sprite != mobj->state->sprite)
				diff |= MD_SPRITE;
			if (mobj->frame != mobj->state->frame)
				diff |= MD_FRAME;
			if (mobj->eflags)
				diff |= MD_EFLAGS;
			if (mobj->player)
				diff |= MD_PLAYER;

			if (mobj->movedir)
				diff |= MD_MOVEDIR;
			if (mobj->movecount)
				diff |= MD_MOVECOUNT;
			if (mobj->threshold)
				diff |= MD_THRESHOLD;
			if (mobj->lastlook != -1)
				diff |= MD_LASTLOOK;
			if (mobj->target)
				diff |= MD_TARGET;
			if (mobj->tracer)
				diff |= MD_TRACER;
			if (mobj->friction != ORIG_FRICTION)
				diff |= MD_FRICTION;
			if (mobj->movefactor != ORIG_FRICTION_FACTOR)
				diff |= MD_MOVEFACTOR;
			if (mobj->fuse)
				diff |= MD_FUSE;
			if (mobj->watertop)
				diff |= MD_WATERTOP;
			if (mobj->waterbottom)
				diff |= MD_WATERBOTTOM;
			if (mobj->scale != 100)
				diff |= MD_SCALE;
			if (mobj->destscale != mobj->scale)
				diff |= MD_DSCALE;

			WRITEBYTE(save_p, tc_mobj);
			WRITEULONG(save_p, diff);

			// save pointer, at load time we will search this pointer to reinitilize pointers
			WRITEULONG(save_p, (ULONG)(size_t)mobj);

			WRITEFIXED(save_p, mobj->z); // Force this so 3dfloor problems don't arise.
			WRITEFIXED(save_p, mobj->floorz);
			WRITEFIXED(save_p, mobj->ceilingz);

			if (diff & MD_SPAWNPOINT)
			{
				size_t z;

				for (z = 0; z < nummapthings; z++)
					if (&mapthings[z] == mobj->spawnpoint)
						WRITESHORT(save_p, z);

				if (mobj->type == MT_HOOPCENTER)
					continue;
			}

			if (diff & MD_TYPE)
				WRITEULONG(save_p, mobj->type);
			if (diff & MD_POS)
			{
				WRITEFIXED(save_p, mobj->x);
				WRITEFIXED(save_p, mobj->y);
				WRITEANGLE(save_p, mobj->angle);
			}
			if (diff & MD_MOM)
			{
				WRITEFIXED(save_p, mobj->momx);
				WRITEFIXED(save_p, mobj->momy);
				WRITEFIXED(save_p, mobj->momz);
			}
			if (diff & MD_RADIUS)
				WRITEFIXED(save_p, mobj->radius);
			if (diff & MD_HEIGHT)
				WRITEFIXED(save_p, mobj->height);
			if (diff & MD_FLAGS)
				WRITELONG(save_p, mobj->flags);
			if (diff & MD_FLAGS2)
				WRITELONG(save_p, mobj->flags2);
			if (diff & MD_HEALTH)
				WRITELONG(save_p, mobj->health);
			if (diff & MD_RTIME)
				WRITELONG(save_p, mobj->reactiontime);
			if (diff & MD_STATE)
				WRITEUSHORT(save_p, mobj->state-states);
			if (diff & MD_TICS)
				WRITELONG(save_p, mobj->tics);
			if (diff & MD_SPRITE)
				WRITEUSHORT(save_p, mobj->sprite);
			if (diff & MD_FRAME)
				WRITEULONG(save_p, mobj->frame);
			if (diff & MD_EFLAGS)
				WRITEULONG(save_p, mobj->eflags);
			if (diff & MD_PLAYER)
				WRITEBYTE(save_p, mobj->player-players);
			if (diff & MD_MOVEDIR)
				WRITEANGLE(save_p, mobj->movedir);
			if (diff & MD_MOVECOUNT)
				WRITELONG(save_p, mobj->movecount);
			if (diff & MD_THRESHOLD)
				WRITELONG(save_p, mobj->threshold);
			if (diff & MD_LASTLOOK)
				WRITELONG(save_p, mobj->lastlook);
			if (diff & MD_TARGET)
				WRITEULONG(save_p, mobj->target->mobjnum);
			if (diff & MD_TRACER)
				WRITEULONG(save_p, mobj->tracer->mobjnum);
			if (diff & MD_FRICTION)
				WRITELONG(save_p, mobj->friction);
			if (diff & MD_MOVEFACTOR)
				WRITELONG(save_p, mobj->movefactor);
			if (diff & MD_FUSE)
				WRITELONG(save_p, mobj->fuse);
			if (diff & MD_WATERTOP)
				WRITELONG(save_p, mobj->watertop);
			if (diff & MD_WATERBOTTOM)
				WRITELONG(save_p, mobj->waterbottom);
			if (diff & MD_SCALE)
				WRITEUSHORT(save_p, mobj->scale);
			if (diff & MD_DSCALE)
				WRITEUSHORT(save_p, mobj->destscale);

			WRITEBYTE(save_p, mobj->scalespeed);

			WRITEULONG(save_p, mobj->mobjnum);
		}
#ifdef PARANOIA
		else if (th->function.acp1 == (actionf_p1)P_RainThinker
			|| th->function.acp1 == (actionf_p1)P_SnowThinker);
#endif
		else if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
		{
			SaveCeilingThinker(th, tc_ceiling);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_CrushCeiling)
		{
			SaveCeilingThinker(th, tc_crushceiling);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MoveFloor)
		{
			SaveFloormoveThinker(th, tc_floor);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LightningFlash)
		{
			SaveLightflashThinker(th, tc_flash);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
		{
			SaveStrobeThinker(th, tc_strobe);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Glow)
		{
			SaveGlowThinker(th, tc_glow);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_FireFlicker)
		{
			SaveFireflickerThinker(th, tc_fireflicker);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MoveElevator)
		{
			SaveElevatorThinker(th, tc_elevator);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ContinuousFalling)
		{
			SaveSpecialLevelThinker(th, tc_continuousfalling);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ThwompSector)
		{
			SaveSpecialLevelThinker(th, tc_thwomp);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_NoEnemiesSector)
		{
			SaveSpecialLevelThinker(th, tc_noenemies);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_EachTimeThinker)
		{
			SaveSpecialLevelThinker(th, tc_eachtime);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_RaiseSector)
		{
			SaveSpecialLevelThinker(th, tc_raisesector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_CameraScanner)
		{
			SaveElevatorThinker(th, tc_camerascanner);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Scroll)
		{
			SaveScrollThinker(th, tc_scroll);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Friction)
		{
			SaveFrictionThinker(th, tc_friction);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Pusher)
		{
			SavePusherThinker(th, tc_pusher);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_BounceCheese)
		{
			SaveSpecialLevelThinker(th, tc_bouncecheese);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_StartCrumble)
		{
			SaveElevatorThinker(th, tc_startcrumble);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MarioBlock)
		{
			SaveSpecialLevelThinker(th, tc_marioblock);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_SpikeSector)
		{
			SaveSpecialLevelThinker(th, tc_spikesector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_FloatSector)
		{
			SaveSpecialLevelThinker(th, tc_floatsector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_BridgeThinker)
		{
			SaveSpecialLevelThinker(th, tc_bridgethinker);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LaserFlash)
		{
			SaveLaserThinker(th, tc_laserflash);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LightFade)
		{
			SaveLightlevelThinker(th, tc_lightfade);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ExecutorDelay)
		{
			SaveExecutorThinker(th, tc_executor);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Disappear)
		{
			SaveDisappearThinker(th, tc_disappear);
			continue;
		}
#ifdef POLYOBJECTS
		else if (th->function.acp1 == (actionf_p1)T_PolyObjRotate)
		{
			SavePolyrotatetThinker(th, tc_polyrotate);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjMove)
		{
			SavePolymoveThinker(th, tc_polymove);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjWaypoint)
		{
			SavePolywaypointThinker(th, tc_polywaypoint);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyDoorSlide)
		{
			SavePolyslidedoorThinker(th, tc_polyslidedoor);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyDoorSwing)
		{
			SavePolyswingdoorThinker(th, tc_polyswingdoor);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjFlag)
		{
			SavePolymoveThinker(th, tc_polyflag);
			continue;
		}
#endif
#ifdef PARANOIA
		else if (th->function.acv != P_RemoveThinkerDelayed) // wait garbage collection
			I_Error("unknown thinker type %p", th->function.acp1);
#endif
	}

	WRITEBYTE(save_p, tc_end);
}

// Now save the pointers, tracer and target, but at load time we must
// relink to this; the savegame contains the old position in the pointer
// field copyed in the info field temporarily, but finally we just search
// for the old position and relink to it.
static mobj_t *FindNewPosition(ULONG oldposition)
{
	thinker_t *th;
	mobj_t *mobj;

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mobj = (mobj_t *)th;
		if (mobj->mobjnum == oldposition)
			return mobj;
	}
	if (devparm)
		CONS_Printf("\2not found\n");
	DEBFILE("not found\n");
	return NULL;
}

static inline mobj_t *LoadMobj(UINT32 mobjnum)
{
	if (mobjnum == 0) return NULL;
	return (mobj_t *)(size_t)mobjnum;
}

static sector_t *LoadSector(UINT32 sector)
{
	if (sector >= numsectors) return NULL;
	return &sectors[sector];
}

static line_t *LoadLine(UINT32 line)
{
	if (line >= numlines) return NULL;
	return &lines[line];
}

static inline player_t *LoadPlayer(UINT32 player)
{
	if (player >= MAXPLAYERS) return NULL;
	return &players[player];
}

#ifndef REMOVE_FOR_205
static inline ffloor_t *LoadFfloor(UINT32 pfloors, sector_t *sector)
{
	if (pfloors != 0xFFFFFFFF && sector && sector->ffloors)
		return &sector->ffloors[pfloors];
	return NULL;
}
#endif

//
// LoadSpecialLevelThinker
//
// Loads a levelspecthink_t from a save game
//
// floorOrCeiling:
//		0 - Don't set
//		1 - Floor Only
//		2 - Ceiling Only
//		3 - Both
//
static void LoadSpecialLevelThinker(actionf_p1 thinker, byte floorOrCeiling)
{
	levelspecthink_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	size_t i;
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->type = READLONG(save_p); //type
	for (i = 0; i < 16; i++)
		ht->vars[i] = READFIXED(save_p); //var[16]

	(void)READULONG(save_p); // activator dummy
	ht->sourceline = LoadLine(READULONG(save_p));
	ht->sector = LoadSector(READULONG(save_p));

	if (ht->sector)
	{
		if (floorOrCeiling & 2)
			ht->sector->ceilingdata = ht;
		if (floorOrCeiling & 1)
			ht->sector->floordata = ht;
	}

	P_AddThinker(&ht->thinker);
}

//
// LoadCeilingThinker
//
// Loads a ceiling_t from a save game
//
static void LoadCeilingThinker(actionf_p1 thinker)
{
	ceiling_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->type = READLONG(save_p);
	ht->sector = LoadSector(READULONG(save_p));
	ht->bottomheight = READFIXED(save_p);
	ht->topheight = READFIXED(save_p);
	ht->speed = READFIXED(save_p);
	ht->oldspeed = READFIXED(save_p);
	ht->delay = READFIXED(save_p);
	ht->delaytimer = READFIXED(save_p);
	ht->crush = READLONG(save_p);
	ht->texture = READLONG(save_p);
	ht->direction = READLONG(save_p);
	ht->tag = READLONG(save_p);
	ht->olddirection = READLONG(save_p);
	(void)READLONG(save_p);  //*list dummy
	ht->origspeed = READFIXED(save_p);
	ht->sourceline = READFIXED(save_p);
	if (ht->sector)
		ht->sector->ceilingdata = ht;
	P_AddThinker(&ht->thinker);
}

//
// LoadFloormoveThinker
//
// Loads a floormove_t from a save game
//
static void LoadFloormoveThinker(actionf_p1 thinker)
{
	floormove_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->type = READLONG(save_p);
	ht->crush = READLONG(save_p);
	ht->sector = LoadSector(READULONG(save_p));
	ht->direction = READLONG(save_p);
	ht->texture = READLONG(save_p);
	ht->floordestheight = READFIXED(save_p);
	ht->speed = READFIXED(save_p);
	ht->origspeed = READFIXED(save_p);
	ht->delay = READFIXED(save_p);
	ht->delaytimer = READFIXED(save_p);
	if (ht->sector)
		ht->sector->floordata = ht;
	P_AddThinker(&ht->thinker);
}

//
// LoadLightflashThinker
//
// Loads a lightflash_t from a save game
//
static inline void LoadLightflashThinker(actionf_p1 thinker)
{
	lightflash_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->sector = LoadSector(READULONG(save_p));
	ht->maxlight = READLONG(save_p);
	ht->minlight = READLONG(save_p);
	if (ht->sector)
		ht->sector->lightingdata = ht;
	P_AddThinker(&ht->thinker);
}

//
// LoadStrobeThinker
//
// Loads a strobe_t from a save game
//
static void LoadStrobeThinker(actionf_p1 thinker)
{
	strobe_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->sector = LoadSector(READULONG(save_p));
	ht->count = READLONG(save_p);
	ht->minlight = READLONG(save_p);
	ht->maxlight = READLONG(save_p);
	ht->darktime = READLONG(save_p);
	ht->brighttime = READLONG(save_p);
	if (ht->sector)
		ht->sector->lightingdata = ht;
	P_AddThinker(&ht->thinker);
}

//
// LoadGlowThinker
//
// Loads a glow_t from a save game
//
static void LoadGlowThinker(actionf_p1 thinker)
{
	glow_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->sector = LoadSector(READULONG(save_p));
	ht->minlight = READLONG(save_p);
	ht->maxlight = READLONG(save_p);
	ht->direction = READLONG(save_p);
	ht->speed = READLONG(save_p);
	if (ht->sector)
		ht->sector->lightingdata = ht;
	P_AddThinker(&ht->thinker);
}
//
// LoadFireflickerThinker
//
// Loads a fireflicker_t from a save game
//
static void LoadFireflickerThinker(actionf_p1 thinker)
{
	fireflicker_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->sector = LoadSector(READULONG(save_p));
	ht->count = READLONG(save_p);
	ht->resetcount = READLONG(save_p);
	ht->maxlight = READLONG(save_p);
	ht->minlight = READLONG(save_p);
	if (ht->sector)
		ht->sector->lightingdata = ht;
	P_AddThinker(&ht->thinker);
}
//
// LoadElevatorThinker
//
// Loads a elevator_t from a save game
//
static void LoadElevatorThinker(actionf_p1 thinker, byte floorOrCeiling)
{
	elevator_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->type = READLONG(save_p);
	ht->sector = LoadSector(READULONG(save_p));
	ht->actionsector = LoadSector(READULONG(save_p));
	ht->direction = READLONG(save_p);
	ht->floordestheight = READFIXED(save_p);
	ht->ceilingdestheight = READFIXED(save_p);
	ht->speed = READFIXED(save_p);
	ht->origspeed = READFIXED(save_p);
	ht->low = READFIXED(save_p);
	ht->high = READFIXED(save_p);
	ht->distance = READFIXED(save_p);
	ht->delay = READFIXED(save_p);
	ht->delaytimer = READFIXED(save_p);
	ht->floorwasheight = READFIXED(save_p);
	ht->ceilingwasheight = READFIXED(save_p);
	ht->player = LoadPlayer(READULONG(save_p)); // was dummy
	ht->sourceline = LoadLine(READULONG(save_p));

	if (ht->sector)
	{
		if (floorOrCeiling & 2)
			ht->sector->ceilingdata = ht;
		if (floorOrCeiling & 1)
			ht->sector->floordata = ht;
	}

	P_AddThinker(&ht->thinker);
}

//
// LoadScrollThinker
//
// Loads a scroll_t from a save game
//
static void LoadScrollThinker(actionf_p1 thinker)
{
	scroll_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->dx = READFIXED(save_p);
	ht->dy = READFIXED(save_p);
	ht->affectee = READLONG(save_p);
	ht->control = READLONG(save_p);
	ht->last_height = READFIXED(save_p);
	ht->vdx = READFIXED(save_p);
	ht->vdy = READFIXED(save_p);
	ht->accel = READLONG(save_p);
	ht->exclusive = READLONG(save_p);
	ht->type = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadFrictionThinker
//
// Loads a friction_t from a save game
//
static inline void LoadFrictionThinker(actionf_p1 thinker)
{
	friction_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->friction = READLONG(save_p);
	ht->movefactor = READLONG(save_p);
	ht->affectee = READLONG(save_p);
	ht->referrer = READLONG(save_p);
	ht->roverfriction = READBYTE(save_p);
	(void)READBYTE(save_p); //gap dummy
	(void)READBYTE(save_p); //gap dummy
	(void)READBYTE(save_p); //gap dummy
	P_AddThinker(&ht->thinker);
}

//
// LoadPusherThinker
//
// Loads a pusher_t from a save game
//
static void LoadPusherThinker(actionf_p1 thinker)
{
	pusher_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->type = READLONG(save_p);
	(void)READULONG(save_p); //source dummy, used affectee
	ht->x_mag = READLONG(save_p);
	ht->y_mag = READLONG(save_p);
	ht->magnitude = READLONG(save_p);
	ht->radius = READLONG(save_p);
	ht->x = READLONG(save_p);
	ht->y = READLONG(save_p);
	ht->z = READLONG(save_p);
	ht->affectee = READLONG(save_p);
	ht->roverpusher = READBYTE(save_p);
	(void)READBYTE(save_p); //gap dummy
	(void)READBYTE(save_p); //gap dummy
	(void)READBYTE(save_p); //gap dummy
	ht->referrer = READLONG(save_p);
	ht->exclusive = READLONG(save_p);
	ht->slider = READLONG(save_p);
	ht->source = P_GetPushThing(ht->affectee);
	P_AddThinker(&ht->thinker);
}

//
// LoadLaserThinker
//
// Loads a laserthink_t from a save game
//
static inline void LoadLaserThinker(actionf_p1 thinker)
{
#ifndef REMOVE_FOR_205
	UINT32 tmp;
#endif
	laserthink_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
#ifndef REMOVE_FOR_205
	tmp = READULONG(save_p);
#endif
	ht->sector = LoadSector(READULONG(save_p));
	ht->ffloor = NULL;
#ifdef REMOVE_FOR_205
	ht->sec = LoadSector(READULONG(save_p));
	ht->sourceline = LoadLine(READULONG(save_p));
#else
	ht->ffloor = LoadFfloor(tmp, ht->sector);
#endif
	P_AddThinker(&ht->thinker);
}

//
// LoadLightlevelThinker
//
// Loads a lightlevel_t from a save game
//
static inline void LoadLightlevelThinker(actionf_p1 thinker)
{
	lightlevel_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->sector = LoadSector(READULONG(save_p));
	ht->destlevel = READLONG(save_p);
	ht->speed = READLONG(save_p);
	if (ht->sector)
		ht->sector->lightingdata = ht;
	P_AddThinker(&ht->thinker);
}

//
// LoadExecutorThinker
//
// Loads a executor_t from a save game
//
static inline void LoadExecutorThinker(actionf_p1 thinker)
{
	executor_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->line = LoadLine(READULONG(save_p));
	ht->caller = LoadMobj(READULONG(save_p));
	ht->timer = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadDisappearThinker
//
// Loads a disappear_t thinker
//
static inline void LoadDisappearThinker(actionf_p1 thinker)
{
	disappear_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->appeartime = READULONG(save_p);
	ht->disappeartime = READULONG(save_p);
	ht->offset = READULONG(save_p);
	ht->timer = READULONG(save_p);
	ht->affectee = READLONG(save_p);
	ht->sourceline = READLONG(save_p);
	ht->exists = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}

#ifdef POLYOBJECTS

//
// LoadPolyrotateThinker
//
// Loads a polyrotate_t thinker
//
static inline void LoadPolyrotatetThinker(actionf_p1 thinker)
{
	polyrotate_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->polyObjNum = READLONG(save_p);
	ht->speed = READLONG(save_p);
	ht->distance = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadPolymoveThinker
//
// Loads a polymovet_t thinker
//
static void LoadPolymoveThinker(actionf_p1 thinker)
{
	polymove_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->polyObjNum = READLONG(save_p);
	ht->speed = READLONG(save_p);
	ht->momx = READFIXED(save_p);
	ht->momy = READFIXED(save_p);
	ht->distance = READLONG(save_p);
	ht->angle = READULONG(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadPolywaypointThinker
//
// Loads a polywaypoint_t thinker
//
static inline void LoadPolywaypointThinker(actionf_p1 thinker)
{
	polywaypoint_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->polyObjNum = READLONG(save_p);
	ht->speed = READLONG(save_p);
	ht->sequence = READLONG(save_p);
	ht->pointnum = READLONG(save_p);
	ht->direction = READLONG(save_p);
	ht->comeback = READLONG(save_p);
	ht->wrap = READLONG(save_p);
	ht->continuous = READLONG(save_p);
	ht->stophere = READLONG(save_p);
	ht->diffx = READFIXED(save_p);
	ht->diffy = READFIXED(save_p);
	ht->diffz = READFIXED(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadPolyslidedoorThinker
//
// loads a polyslidedoor_t thinker
//
static inline void LoadPolyslidedoorThinker(actionf_p1 thinker)
{
	polyslidedoor_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->polyObjNum = READLONG(save_p);
	ht->delay = READLONG(save_p);
	ht->delayCount = READLONG(save_p);
	ht->initSpeed = READLONG(save_p);
	ht->speed = READLONG(save_p);
	ht->initDistance = READLONG(save_p);
	ht->distance = READLONG(save_p);
	ht->initAngle = READULONG(save_p);
	ht->angle = READULONG(save_p);
	ht->revAngle = READULONG(save_p);
	ht->momx = READFIXED(save_p);
	ht->momy = READFIXED(save_p);
	ht->closing = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}

//
// LoadPolyswingdoorThinker
//
// Loads a polyswingdoor_t thinker
//
static inline void LoadPolyswingdoorThinker(actionf_p1 thinker)
{
	polyswingdoor_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	ht->polyObjNum = READLONG(save_p);
	ht->delay = READLONG(save_p);
	ht->delayCount = READLONG(save_p);
	ht->initSpeed = READLONG(save_p);
	ht->speed = READLONG(save_p);
	ht->initDistance = READLONG(save_p);
	ht->distance = READLONG(save_p);
	ht->closing = READLONG(save_p);
	P_AddThinker(&ht->thinker);
}
#endif

/*
//
// LoadWhatThinker
//
// load a what_t thinker
//
static inline void LoadWhatThinker(actionf_p1 thinker)
{
	what_t *ht = Z_Malloc(sizeof (*ht), PU_LEVSPEC, NULL);
	ht->thinker.function.acp1 = thinker;
	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READLONG(save_p);  //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
}
*/

//
// P_NetUnArchiveThinkers
//
static void P_NetUnArchiveThinkers(void)
{
	thinker_t *currentthinker;
	thinker_t *next;
	mobj_t *mobj;
	ULONG diff;
	INT32 i;
	byte tclass;
	byte restoreNum = false;
	fixed_t z, floorz, ceilingz;

	// remove all the current thinkers
	currentthinker = thinkercap.next;
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = next)
	{
		next = currentthinker->next;

		mobj = (mobj_t *)currentthinker;
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
			P_RemoveSavegameMobj((mobj_t *)currentthinker); // item isn't saved, don't remove it
		else
			Z_Free(currentthinker);
	}

	// we don't want the removed mobjs to come back
	iquetail = iquehead = 0;
	P_InitThinkers();

	// read in saved thinkers
	for (;;)
	{
		tclass = READBYTE(save_p);

		if (tclass == tc_end)
			break; // leave the saved thinker reading loop

		switch (tclass)
		{
			case tc_mobj:
				diff = READULONG(save_p);

				next = (void *)(size_t)READULONG(save_p);

				z = READFIXED(save_p); // Force this so 3dfloor problems don't arise.
				floorz = READFIXED(save_p);
				ceilingz = READFIXED(save_p);

				if (diff & MD_SPAWNPOINT)
				{
					short spawnpointnum = READSHORT(save_p);

					if (mapthings[spawnpointnum].type == 1705) // NiGHTS Hoop special case
					{
						P_SpawnHoopsAndRings(&mapthings[spawnpointnum]);
						continue;
					}

					mobj = Z_Calloc(sizeof (*mobj), PU_LEVEL, NULL);

					mobj->spawnpoint = &mapthings[spawnpointnum];
					mapthings[spawnpointnum].mobj = mobj;
				}
				else
					mobj = Z_Calloc(sizeof (*mobj), PU_LEVEL, NULL);

				mobj->z = z;
				mobj->floorz = floorz;
				mobj->ceilingz = ceilingz;

				if (diff & MD_TYPE)
					mobj->type = READULONG(save_p);
				else
				{
					for (i = 0; i < NUMMOBJTYPES; i++)
						if (mobj->spawnpoint && mobj->spawnpoint->type == mobjinfo[i].doomednum)
							break;
					if (i == NUMMOBJTYPES)
					{
						if (mobj->spawnpoint)
							CONS_Printf("found mobj with unknown map thing type %d\n",
							mobj->spawnpoint->type);
						else
							CONS_Printf("found mobj with unknown map thing type NULL\n");
						I_Error("Savegame corrupted");
					}
					mobj->type = i;
				}
				mobj->info = &mobjinfo[mobj->type];
				if (diff & MD_POS)
				{
					mobj->x = READFIXED(save_p);
					mobj->y = READFIXED(save_p);
					mobj->angle = READANGLE(save_p);
				}
				else
				{
					mobj->x = mobj->spawnpoint->x << FRACBITS;
					mobj->y = mobj->spawnpoint->y << FRACBITS;
					mobj->angle = ANGLE_45 * (mobj->spawnpoint->angle/45); /// \bug unknown
				}
				if (diff & MD_MOM)
				{
					mobj->momx = READFIXED(save_p);
					mobj->momy = READFIXED(save_p);
					mobj->momz = READFIXED(save_p);
				} // otherwise they're zero, and the memset took care of it

				if (diff & MD_RADIUS)
					mobj->radius = READFIXED(save_p);
				else
					mobj->radius = mobj->info->radius;
				if (diff & MD_HEIGHT)
					mobj->height = READFIXED(save_p);
				else
					mobj->height = mobj->info->height;
				if (diff & MD_FLAGS)
					mobj->flags = READLONG(save_p);
				else
					mobj->flags = mobj->info->flags;
				mobj->flags2 = READLONG(save_p);
				if (diff & MD_HEALTH)
					mobj->health = READLONG(save_p);
				else
					mobj->health = mobj->info->spawnhealth;
				if (diff & MD_RTIME)
					mobj->reactiontime = READLONG(save_p);
				else
					mobj->reactiontime = mobj->info->reactiontime;

				if (diff & MD_STATE)
					mobj->state = &states[READUSHORT(save_p)];
				else
					mobj->state = &states[mobj->info->spawnstate];
				if (diff & MD_TICS)
					mobj->tics = READLONG(save_p);
				else
					mobj->tics = mobj->state->tics;
				if (diff & MD_SPRITE)
					mobj->sprite = READUSHORT(save_p);
				else
					mobj->sprite = mobj->state->sprite;
				if (diff & MD_FRAME)
					mobj->frame = READULONG(save_p);
				else
					mobj->frame = mobj->state->frame;
				if (diff & MD_EFLAGS)
					mobj->eflags = READULONG(save_p);
				if (diff & MD_PLAYER)
				{
					i = READBYTE(save_p);
					mobj->player = &players[i];
					mobj->player->mo = mobj;
					// added for angle prediction
					if (consoleplayer == i)
						localangle = mobj->angle;
					if (secondarydisplayplayer == i)
						localangle2 = mobj->angle;
				}
				if (diff & MD_MOVEDIR)
					mobj->movedir = READANGLE(save_p);
				if (diff & MD_MOVECOUNT)
					mobj->movecount = READLONG(save_p);
				if (diff & MD_THRESHOLD)
					mobj->threshold = READLONG(save_p);
				if (diff & MD_LASTLOOK)
					mobj->lastlook = READLONG(save_p);
				else
					mobj->lastlook = -1;
				if (diff & MD_TARGET)
					mobj->target = (mobj_t *)(size_t)READULONG(save_p);
				if (diff & MD_TRACER)
					mobj->tracer = (mobj_t *)(size_t)READULONG(save_p);
				if (diff & MD_FRICTION)
					mobj->friction = READLONG(save_p);
				else
					mobj->friction = ORIG_FRICTION;
				if (diff & MD_MOVEFACTOR)
					mobj->movefactor = READLONG(save_p);
				else
					mobj->movefactor = ORIG_FRICTION_FACTOR;
				if (diff & MD_FUSE)
					mobj->fuse = READLONG (save_p);
				if (diff & MD_WATERTOP)
					mobj->watertop = READLONG(save_p);
				if (diff & MD_WATERBOTTOM)
					mobj->waterbottom = READLONG(save_p);
				if (diff & MD_SCALE)
					mobj->scale = READUSHORT(save_p);
				else
					mobj->scale = 100;
				if (diff & MD_DSCALE)
					mobj->destscale = READUSHORT(save_p);
				else
					mobj->destscale = mobj->scale;

				mobj->scalespeed = READBYTE(save_p);

				// now set deductable field
				/// \todo save this too
				mobj->skin = NULL;

				// set sprev, snext, bprev, bnext, subsector
				P_SetThingPosition(mobj);

				mobj->mobjnum = READULONG(save_p);

				if (mobj->player)
				{
					if (mobj->eflags & MFE_VERTICALFLIP)
						mobj->player->viewz = mobj->z + mobj->height - mobj->player->viewheight;
					else
						mobj->player->viewz = mobj->player->mo->z + mobj->player->viewheight;
				}

				mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
				P_AddThinker(&mobj->thinker);

				mobj->info = (mobjinfo_t *)next; // temporarily, set when leave this function
				break;

			case tc_ceiling:
				LoadCeilingThinker((actionf_p1)T_MoveCeiling);
				break;

			case tc_crushceiling:
				LoadCeilingThinker((actionf_p1)T_CrushCeiling);
				break;

			case tc_floor:
				LoadFloormoveThinker((actionf_p1)T_MoveFloor);
				break;

			case tc_flash:
				LoadLightflashThinker((actionf_p1)T_LightningFlash);
				break;

			case tc_strobe:
				LoadStrobeThinker((actionf_p1)T_StrobeFlash);
				break;

			case tc_glow:
				LoadGlowThinker((actionf_p1)T_Glow);
				break;

			case tc_fireflicker:
				LoadFireflickerThinker((actionf_p1)T_FireFlicker);
				break;

			case tc_elevator:
				LoadElevatorThinker((actionf_p1)T_MoveElevator, 3);
				break;

			case tc_continuousfalling:
				LoadSpecialLevelThinker((actionf_p1)T_ContinuousFalling, 3);
				break;

			case tc_thwomp:
				LoadSpecialLevelThinker((actionf_p1)T_ThwompSector, 3);
				break;

			case tc_noenemies:
				LoadSpecialLevelThinker((actionf_p1)T_NoEnemiesSector, 0);
				break;

			case tc_eachtime:
				LoadSpecialLevelThinker((actionf_p1)T_EachTimeThinker, 0);
				break;

			case tc_raisesector:
				LoadSpecialLevelThinker((actionf_p1)T_RaiseSector, 0);
				break;

			/// \todo rewrite all the shit that uses an elevator_t but isn't an elevator
			/// \note working on it!
			case tc_camerascanner:
				LoadElevatorThinker((actionf_p1)T_CameraScanner, 0);
				break;

			case tc_bouncecheese:
				LoadSpecialLevelThinker((actionf_p1)T_BounceCheese, 2);
				break;

			case tc_startcrumble:
				LoadElevatorThinker((actionf_p1)T_StartCrumble, 1);
				break;

			case tc_marioblock:
				LoadSpecialLevelThinker((actionf_p1)T_MarioBlock, 3);
				break;

			case tc_spikesector:
				LoadSpecialLevelThinker((actionf_p1)T_SpikeSector, 0);
				break;

			case tc_floatsector:
				LoadSpecialLevelThinker((actionf_p1)T_FloatSector, 0);
				break;

			case tc_bridgethinker:
				LoadSpecialLevelThinker((actionf_p1)T_BridgeThinker, 3);
				break;

			case tc_laserflash:
				LoadLaserThinker((actionf_p1)T_LaserFlash);
				break;

			case tc_lightfade:
				LoadLightlevelThinker((actionf_p1)T_LightFade);
				break;

			case tc_executor:
				LoadExecutorThinker((actionf_p1)T_ExecutorDelay);
				restoreNum = true;
				break;

			case tc_disappear:
				LoadDisappearThinker((actionf_p1)T_Disappear);
				break;
#ifdef POLYOBJECTS
			case tc_polyrotate:
				LoadPolyrotatetThinker((actionf_p1)T_PolyObjRotate);
				break;

			case tc_polymove:
				LoadPolymoveThinker((actionf_p1)T_PolyObjMove);
				break;

			case tc_polywaypoint:
				LoadPolywaypointThinker((actionf_p1)T_PolyObjWaypoint);
				break;

			case tc_polyslidedoor:
				LoadPolyslidedoorThinker((actionf_p1)T_PolyDoorSlide);
				break;

			case tc_polyswingdoor:
				LoadPolyswingdoorThinker((actionf_p1)T_PolyDoorSwing);
				break;
			case tc_polyflag:
				LoadPolymoveThinker((actionf_p1)T_PolyObjFlag);
				break;
#endif
			case tc_scroll:
				LoadScrollThinker((actionf_p1)T_Scroll);
				break;

			case tc_friction:
				LoadFrictionThinker((actionf_p1)T_Friction);
				break;

			case tc_pusher:
				LoadPusherThinker((actionf_p1)T_Pusher);
				break;

			default:
				I_Error("P_UnarchiveSpecials: Unknown tclass %d in savegame", tclass);
		}
	}

	if (restoreNum)
	{
		executor_t *delay = NULL;
		ULONG mobjnum;
		for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
			currentthinker = currentthinker->next)
		{
			if (currentthinker->function.acp1 == (actionf_p1)T_ExecutorDelay)
			{
				delay = (void *)currentthinker;
				if ((mobjnum = (ULONG)(size_t)delay->caller))
					delay->caller = FindNewPosition(mobjnum);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// haleyjd 03/26/06: PolyObject saving code
//
#ifdef POLYOBJECTS
static inline void P_ArchivePolyObj(polyobj_t *po)
{
	WRITELONG(save_p, po->id);
	WRITEULONG(save_p, po->angle);

	WRITEULONG(save_p, 0); //thinker.prev dummy
	WRITEULONG(save_p, 0); //thinker.next dummy
	WRITEULONG(save_p, 0); //thinker.actionf_t dummy
	WRITELONG(save_p, 0);  //thinker.references dummy
	WRITEFIXED(save_p, po->spawnSpot.x);
	WRITEFIXED(save_p, po->spawnSpot.y);
	WRITEFIXED(save_p, po->spawnSpot.z); //z dummy
}

static inline void P_UnArchivePolyObj(polyobj_t *po)
{
	INT32 id;
	UINT32 angle;
	fixed_t x, y;

	// nullify all polyobject thinker pointers;
	// the thinkers themselves will fight over who gets the field
	// when they first start to run.
	po->thinker = NULL;

	id = READLONG(save_p);

	angle = READULONG(save_p);

	(void)READULONG(save_p); //thinker.prev dummy
	(void)READULONG(save_p); //thinker.next dummy
	(void)READULONG(save_p); //thinker.actionf_t dummy
	(void)READLONG(save_p);  //thinker.references dummy
	x = READFIXED(save_p);
	y = READFIXED(save_p);
	(void)READFIXED(save_p); //z dummy

	// if the object is bad or isn't in the id hash, we can do nothing more
	// with it, so return now
	if (po->isBad || po != Polyobj_GetForNum(id))
		return;

	// rotate and translate polyobject
	Polyobj_MoveOnLoad(po, angle, x, y);
}

static inline void P_ArchivePolyObjects(void)
{
	INT32 i;

	// save number of polyobjects
	WRITELONG(save_p, numPolyObjects);

	for (i = 0; i < numPolyObjects; ++i)
		P_ArchivePolyObj(&PolyObjects[i]);
}

static inline void P_UnArchivePolyObjects(void)
{
	INT32 i, numSavedPolys;

	numSavedPolys = READLONG(save_p);

	if (numSavedPolys != numPolyObjects)
		I_Error("P_UnArchivePolyObjects: polyobj count inconsistency\n");

	for (i = 0; i < numSavedPolys; ++i)
		P_UnArchivePolyObj(&PolyObjects[i]);
}
#endif
//
// P_FinishMobjs
//
static inline void P_FinishMobjs(void)
{
	thinker_t *currentthinker;
	mobj_t *mobj;

	// put info field there real value
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
		currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)currentthinker;
			mobj->info = &mobjinfo[mobj->type];
		}
	}
}

static inline void P_RelinkPointers(void)
{
	thinker_t *currentthinker;
	mobj_t *mobj;
	ULONG temp;

	// use info field (value = oldposition) to relink mobjs
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
		currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)currentthinker;

			if (mobj->type == MT_HOOP || mobj->type == MT_HOOPCOLLIDE || mobj->type == MT_HOOPCENTER)
				continue;

			if (mobj->tracer)
			{
				temp = (ULONG)(size_t)mobj->tracer;
				mobj->tracer = NULL;
				if (!P_SetTarget(&mobj->tracer, FindNewPosition(temp)))
					CONS_Printf("tracer not found on %d\n", mobj->type);
			}
			if (mobj->target)
			{
				temp = (ULONG)(size_t)mobj->target;
				mobj->target = NULL;
				if (!P_SetTarget(&mobj->target, FindNewPosition(temp)))
					CONS_Printf("target not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->capsule)
			{
				temp = (ULONG)(size_t)mobj->player->capsule;
				mobj->player->capsule = NULL;
				if (!P_SetTarget(&mobj->player->capsule, FindNewPosition(temp)))
					CONS_Printf("capsule not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->axis1)
			{
				temp = (ULONG)(size_t)mobj->player->axis1;
				mobj->player->axis1 = NULL;
				if (!P_SetTarget(&mobj->player->axis1, FindNewPosition(temp)))
					CONS_Printf("axis1 not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->axis2)
			{
				temp = (ULONG)(size_t)mobj->player->axis2;
				mobj->player->axis2 = NULL;
				if (!P_SetTarget(&mobj->player->axis2, FindNewPosition(temp)))
					CONS_Printf("axis2 not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->awayviewmobj)
			{
				temp = (ULONG)(size_t)mobj->player->awayviewmobj;
				mobj->player->awayviewmobj = NULL;
				if (!P_SetTarget(&mobj->player->awayviewmobj, FindNewPosition(temp)))
					CONS_Printf("awayviewmobj not found on %d\n", mobj->type);
			}
		}
	}
}

//
// P_NetArchiveSpecials
//
static inline void P_NetArchiveSpecials(void)
{
	size_t i, z;

	// itemrespawn queue for deathmatch
	i = iquetail;
	while (iquehead != i)
	{
		for (z = 0; z < nummapthings; z++)
		{
			if (&mapthings[z] == itemrespawnque[i])
			{
				WRITELONG(save_p, z);
				break;
			}
		}
		WRITELONG(save_p, itemrespawntime[i]);
		i = (i + 1) & (ITEMQUESIZE-1);
	}

	// end delimiter
	WRITEULONG(save_p, 0xffffffff);

	// Sky number
	WRITELONG(save_p, globallevelskynum);

	// Current global weather type
	WRITEBYTE(save_p, globalweather);
}

//
// P_NetUnArchiveSpecials
//
static inline void P_NetUnArchiveSpecials(void)
{
	size_t i;
	INT32 j;

	// BP: added save itemrespawn queue for deathmatch
	iquetail = iquehead = 0;
	while ((i = READULONG(save_p)) != 0xffffffff)
	{
		itemrespawnque[iquehead] = &mapthings[i];
		itemrespawntime[iquehead++] = READLONG(save_p);
	}

	j = READLONG(save_p);
	if (j != globallevelskynum)
	{
		globallevelskynum = levelskynum = j;
		P_SetupLevelSky(levelskynum);
	}

	globalweather = READBYTE(save_p);

	if (globalweather)
	{
		if (curWeather == globalweather)
			curWeather = PRECIP_NONE;

		P_SwitchWeather(globalweather);
	}
	else // PRECIP_NONE
	{
		if (curWeather != PRECIP_NONE)
			P_SwitchWeather(globalweather);
	}
}

// =======================================================================
//          Misc
// =======================================================================
static inline void P_ArchiveMisc(void)
{
	if (gamecomplete)
		WRITESHORT(save_p, spstage_end);
	else
		WRITESHORT(save_p, gamemap);

	lastmapsaved = gamemap;

	WRITEUSHORT(save_p, emeralds+357);
	WRITESTRINGN(save_p, timeattackfolder, sizeof(timeattackfolder));
}

static inline boolean P_UnArchiveSPGame(short mapoverride)
{
	char testname[sizeof(timeattackfolder)];

	gamemap = READSHORT(save_p);

	if (mapoverride != 0)
	{
		gamemap = mapoverride;
		gamecomplete = true;
	}
	else
		gamecomplete = false;

	lastmapsaved = gamemap;

	tokenlist = 0;
	token = 0;

	savedata.emeralds = (UINT16)(READUSHORT(save_p)-357);

	READSTRINGN(save_p, testname, sizeof(testname));

	if (strcmp(testname, timeattackfolder))
	{
		if (modifiedgame)
			I_Error("Save game not for this modification.");
		else
			I_Error("This save file is for a particular mod, it cannot be used with the regular game.");
	}

	memset(playeringame, 0, sizeof(*playeringame));
	playeringame[consoleplayer] = true;

	P_UnArchivePlayer();

	G_DeferedInitNew(false, G_BuildMapName(gamemap), savedata.skin, false, true);
	COM_BufAddText("dummyconsvar 1\n"); // G_DeferedInitNew doesn't do this

	return true;
}

static inline void P_NetArchiveMisc(void)
{
	ULONG pig = 0;
	INT32 i, j;

	WRITESHORT(save_p, gamemap);
	WRITESHORT(save_p, gamestate);

	WRITEULONG(save_p, tokenlist);

	for (i = 0; i < MAXPLAYERS; i++)
		pig |= (playeringame[i] != 0)<<i;

	WRITEULONG(save_p, pig);

	for (i = 0; i < MAXPLAYERS; i++)
		for (j = 0; j < S_PLAY_SUPERTRANS9+1; j++)
			WRITELONG(save_p, playerstatetics[i][j]);

	WRITEULONG(save_p, leveltime);
	WRITEULONG(save_p, totalrings);
	WRITESHORT(save_p, lastmap);

	WRITEUSHORT(save_p, emeralds);

	WRITEULONG(save_p, token);
	WRITELONG(save_p, sstimer);
	WRITEULONG(save_p, bluescore);
	WRITEULONG(save_p, redscore);
	WRITEULONG(save_p, blueflagloose);
	WRITEULONG(save_p, redflagloose);

	WRITESHORT(save_p, autobalance);
	WRITESHORT(save_p, teamscramble);

	for (i = 0; i < MAXPLAYERS; i++)
		WRITESHORT(save_p, scrambleplayers[i]);

	for (i = 0; i < MAXPLAYERS; i++)
		WRITESHORT(save_p, scrambleteams[i]);

	WRITESHORT(save_p, scrambletotal);
	WRITESHORT(save_p, scramblecount);

	WRITEULONG(save_p, countdown);
	WRITEULONG(save_p, countdown2);

	WRITEFIXED(save_p, gravity);

	WRITEULONG(save_p, countdowntimer);
	WRITEBYTE(save_p, countdowntimeup);

	WRITEBYTE(save_p, P_GetRandIndex());

	WRITELONG(save_p, matchtype);
	WRITELONG(save_p, tagtype);

	WRITEULONG(save_p, hidetime);

	// Is it paused?
	if (paused)
		WRITEBYTE(save_p, 0x2f);
	else
		WRITEBYTE(save_p, 0x2e);
}

static inline boolean P_NetUnArchiveMisc(void)
{
	ULONG pig;
	INT32 i, j;

	gamemap = READSHORT(save_p);
	G_SetGamestate(READSHORT(save_p));

	tokenlist = READULONG(save_p);

	pig = READULONG(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		playeringame[i] = (pig & (1<<i)) != 0;
		players[i].playerstate = PST_REBORN;
	}

	if (!P_SetupLevel(gamemap, true))
		return false;
	// the "\2" instead of NULL is a hackishly hackish hack to avoid loading precipitation

	for (i = 0; i < MAXPLAYERS; i++)
		for (j = 0; j < S_PLAY_SUPERTRANS9+1; j++)
			playerstatetics[i][j] = READLONG(save_p);

	// get the time
	leveltime = READULONG(save_p);
	totalrings = READULONG(save_p);
	lastmap = READSHORT(save_p);

	emeralds = READUSHORT(save_p);

	token = READULONG(save_p);
	sstimer = READLONG(save_p);
	bluescore = READULONG(save_p);
	redscore = READULONG(save_p);
	blueflagloose = READULONG(save_p);
	redflagloose = READULONG(save_p);

	autobalance = READSHORT(save_p);
	teamscramble = READSHORT(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
		scrambleplayers[i] = READSHORT(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
		scrambleteams[i] = READSHORT(save_p);

	scrambletotal = READSHORT(save_p);
	scramblecount = READSHORT(save_p);

	countdown = READULONG(save_p);
	countdown2 = READULONG(save_p);

	gravity = READFIXED(save_p);

	countdowntimer = READULONG(save_p);
	countdowntimeup = READBYTE(save_p);

	P_SetRandIndex(READBYTE(save_p));

	matchtype = READLONG(save_p);
	tagtype = READLONG(save_p);

	hidetime = READULONG(save_p);

	// Is it paused?
	if (READBYTE(save_p) == 0x2f)
		paused = true;

	return true;
}

void P_SaveGame(void)
{
	P_ArchiveMisc();
	P_ArchivePlayer();

	WRITEBYTE(save_p, 0x1d); // consistency marker
}

void P_SaveNetGame(void)
{
	thinker_t *th;
	mobj_t *mobj;
	INT32 i = 0;

	CV_SaveNetVars(&save_p);
	P_NetArchiveMisc();

	// Assign the mobjnumber for pointer tracking
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)th;
			mobj->mobjnum = i++;
		}
	}

	P_NetArchivePlayers();
	P_NetArchiveWorld();
#ifdef POLYOBJECTS
	P_ArchivePolyObjects();
#endif
	P_NetArchiveThinkers();
	P_NetArchiveSpecials();

	WRITEBYTE(save_p, 0x1d); // consistency marker
}

boolean P_LoadGame(short mapoverride)
{
	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission();
	G_SetGamestate(GS_NULL); // should be changed in P_UnArchiveMisc

	if (!P_UnArchiveSPGame(mapoverride))
		return false;

	return READBYTE(save_p) == 0x1d;
}

boolean P_LoadNetGame(void)
{
	CV_LoadNetVars(&save_p);
	if (!P_NetUnArchiveMisc())
		return false;
	P_NetUnArchivePlayers();
	P_NetUnArchiveWorld();
#ifdef POLYOBJECTS
	P_UnArchivePolyObjects();
#endif
	P_NetUnArchiveThinkers();
	P_NetUnArchiveSpecials();
	P_RelinkPointers();
	P_FinishMobjs();

	// The precipitation would normally be spawned in P_SetupLevel, which is called by
	// P_NetUnArchiveMisc above. However, that would place it up before P_NetUnArchiveThinkers,
	// so the thinkers would be deleted later. Therefore, P_SetupLevel will *not* spawn
	// precipitation when loading a netgame save. Instead, precip has to be spawned here.
	// This is done in P_NetUnArchiveSpecials now.

	return READBYTE(save_p) == 0x1d;
}
