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
/// \brief cd music interface (uses MCI).

#include "../doomdef.h"
#include "win_main.h"
#include <mmsystem.h>

#include "../command.h"
#include "../doomtype.h"
#include "../i_sound.h"
#include "../i_system.h"

#include "../s_sound.h"

#define MAX_CD_TRACKS       255

typedef struct {
	BOOL    IsAudio;
	DWORD   Start, End;
	DWORD   Length;         // minutes
} CDTrack;

// -------
// private
// -------
static  CDTrack          m_nTracks[MAX_CD_TRACKS];
static  int              m_nTracksCount;             // up to MAX_CD_TRACKS
static  MCI_STATUS_PARMS m_MCIStatus;
static  MCI_OPEN_PARMS   m_MCIOpen;

// ------
// protos
// ------
static void Command_Cd_f (void);


// -------------------
// MCIErrorMessageBox
// Retrieve error message corresponding to return value from
//  mciSendCommand() or mciSenString()
// -------------------
static VOID MCIErrorMessageBox (MCIERROR iErrorCode)
{
	char szErrorText[128];
	if (!mciGetErrorStringA (iErrorCode, szErrorText, sizeof (szErrorText)))
		wsprintfA(szErrorText,"MCI CD Audio Unknow Error #%d\n", iErrorCode);
	CONS_Printf(szErrorText);
	/*MessageBox (GetActiveWindow(), szTemp+1, "LEGACY",
				MB_OK | MB_ICONSTOP);*/
}


// --------
// CD_Reset
// --------
static VOID CD_Reset (VOID)
{
	// no win32 equivalent
	//faB: for DOS, some odd drivers like to be reset sometimes.. useless in MCI I guess
}


// ----------------
// CD_ReadTrackInfo
// Read in number of tracks, and length of each track in minutes/seconds
// returns true if error
// ----------------
static BOOL CD_ReadTrackInfo(VOID)
{
	UINT     nTrackLength;
	INT      i;
	MCIERROR iErr;

	m_nTracksCount = 0;

	m_MCIStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(size_t)&m_MCIStatus);
	if (iErr)
	{
		MCIErrorMessageBox (iErr);
		return FALSE;
	}
	m_nTracksCount = (int)m_MCIStatus.dwReturn;
	if (m_nTracksCount > MAX_CD_TRACKS)
		m_nTracksCount = MAX_CD_TRACKS;

	for (i = 0; i < m_nTracksCount; i++)
	{
		m_MCIStatus.dwTrack = (DWORD)(i+1);
		m_MCIStatus.dwItem = MCI_STATUS_LENGTH;
		iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_STATUS, MCI_TRACK|MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(size_t)&m_MCIStatus);
		if (iErr)
		{
			MCIErrorMessageBox (iErr);
			return FALSE;
		}
		nTrackLength = (DWORD)(MCI_MSF_MINUTE(m_MCIStatus.dwReturn)*60 + MCI_MSF_SECOND(m_MCIStatus.dwReturn));
		m_nTracks[i].Length = nTrackLength;

		m_MCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
		iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_STATUS, MCI_TRACK|MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(size_t)&m_MCIStatus);
		if (iErr)
		{
			MCIErrorMessageBox (iErr);
			return FALSE;
		}
		m_nTracks[i].IsAudio = (m_MCIStatus.dwReturn == MCI_CDA_TRACK_AUDIO);
	}

	return TRUE;
}


// ------------
// CD_TotalTime
// returns total time for all audio tracks in seconds
// ------------
static UINT CD_TotalTime(VOID)
{
	UINT nTotalLength = 0;
	INT nTrack;
	for (nTrack = 0; nTrack < m_nTracksCount; nTrack++)
	{
		if (m_nTracks[nTrack].IsAudio)
			nTotalLength += m_nTracks[nTrack].Length;
	}
	return nTotalLength;
}


//======================================================================
//                   CD AUDIO MUSIC SUBSYSTEM
//======================================================================

byte   cdaudio_started = 0;   // for system startup/shutdown

static BOOL cdPlaying = FALSE;
static  INT cdPlayTrack;         // when cdPlaying is true
static BOOL cdLooping = FALSE;
static BYTE cdRemap[MAX_CD_TRACKS];
static BOOL cdEnabled = TRUE;      // cd info available
static BOOL cdValid;             // true when last cd audio info was ok
static BOOL wasPlaying;
//static INT     cdVolume = 0;          // current cd volume (0-31)

// 0-31 like Music & Sfx, though CD hardware volume is 0-255.
consvar_t cd_volume = {"cd_volume","31",CV_SAVE,soundvolume_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// allow Update for next/loop track
// some crap cd drivers take up to
// a second for a simple 'busy' check..
// (on those Update can be disabled)
consvar_t cdUpdate  = {"cd_update","1",CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

// hour,minutes,seconds
static LPSTR hms(UINT seconds)
{
	UINT hours, minutes;
	static CHAR s[9];

	minutes = seconds / 60;
	seconds %= 60;
	hours = minutes / 60;
	minutes %= 60;
	if (hours > 0)
		sprintf (s, "%d:%02d:%02d", hours, minutes, seconds);
	else
		sprintf (s, "%2d:%02d", minutes, seconds);
	return s;
}

static void Command_Cd_f(void)
{
	LPCSTR    s;
	int       i,j;

	if (!cdaudio_started)
		return;

	if (COM_Argc()<2)
	{
		CONS_Printf ("cd [on] [off] [remap] [reset] [open]\n"
		             "   [info] [play <track>] [loop <track>]\n"
		             "   [stop] [resume]\n");
		return;
	}

	s = COM_Argv(1);

	// activate cd music
	if (!strncmp(s,"on",2))
	{
		cdEnabled = TRUE;
		return;
	}

	// stop/deactivate cd music
	if (!strncmp(s,"off",3))
	{
		if (cdPlaying)
			I_StopCD ();
		cdEnabled = FALSE;
		return;
	}

	// remap tracks
	if (!strncmp(s,"remap",5))
	{
		i = (int)COM_Argc() - 2;
		if (i <= 0)
		{
			CONS_Printf ("CD tracks remapped in that order :\n");
			for (j = 1; j < MAX_CD_TRACKS; j++)
				if (cdRemap[j] != j)
					CONS_Printf (" %2d -> %2d\n", j, cdRemap[j]);
			return;
		}
		for (j = 1; j <= i; j++)
			cdRemap[j] = (byte)atoi (COM_Argv (j+1));
		return;
	}

	// reset the CD driver, useful on some odd cd's
	if (!strncmp(s,"reset",5))
	{
		cdEnabled = TRUE;
		if (cdPlaying)
			I_StopCD ();
		for (i = 0; i < MAX_CD_TRACKS; i++)
			cdRemap[i] = (byte)i;
		CD_Reset();
		cdValid = CD_ReadTrackInfo();
		return;
	}

	// any other command is not allowed until we could retrieve cd information
	if (!cdValid)
	{
		CONS_Printf ("CD is not ready.\n");
		return;
	}

	/* faB: not with MCI, didn't find it, useless anyway
	if (!strncmp(s,"open",4))
	{
		if (cdPlaying)
			I_StopCD ();
		bcd_open_door();
		cdValid = FALSE;
		return;
	}*/

	if (!strncmp(s,"info",4))
	{
		if (!CD_ReadTrackInfo())
		{
			cdValid = FALSE;
			return;
		}

		cdValid = TRUE;

		if (m_nTracksCount <= 0)
			CONS_Printf ("No audio tracks\n");
		else
		{
			// display list of tracks
			// highlight current playing track
			for (i = 0; i < m_nTracksCount; i++)
			{
				CONS_Printf("%s%2d. %s  %s\n",
				            cdPlaying && (cdPlayTrack == i) ? "\2 " : " ",
				            i+1, m_nTracks[i].IsAudio ? "audio" : "data ",
				            hms(m_nTracks[i].Length));
			}
			CONS_Printf ("\2Total time : %s\n", hms(CD_TotalTime()));
		}
		if (cdPlaying)
		{
			CONS_Printf ("%s track : %d\n", cdLooping ? "looping" : "playing",
			             cdPlayTrack);
		}
		return;
	}

	if (!strncmp(s,"play",4))
	{
		I_PlayCD (atoi (COM_Argv (2)), false);
		return;
	}

	if (!strncmp(s,"stop",4))
	{
		I_StopCD ();
		return;
	}

	if (!strncmp(s,"loop",4))
	{
		I_PlayCD(atoi (COM_Argv (2)), true);
		return;
	}

	if (!strncmp(s,"resume",4))
	{
		I_ResumeCD ();
		return;
	}

	CONS_Printf ("cd command '%s' unknown\n", s);
}


// ------------
// I_ShutdownCD
// Shutdown CD Audio subsystem, release whatever was allocated
// ------------
void I_ShutdownCD(void)
{
	MCIERROR    iErr;

	if (!cdaudio_started)
		return;

	CONS_Printf ("I_ShutdownCD()\n");

	I_StopCD();

	// closes MCI CD
	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_CLOSE, 0, 0);
	if (iErr)
		MCIErrorMessageBox (iErr);
}


// --------
// I_InitCD
// Init CD Audio subsystem
// --------
void I_InitCD(void)
{
	MCI_SET_PARMS   mciSet;
	MCIERROR    iErr;
	int         i;

	// We don't have an open device yet
	m_MCIOpen.wDeviceID = 0;
	m_nTracksCount = 0;

	cdaudio_started = FALSE;

	m_MCIOpen.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
	iErr = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID, (DWORD)(size_t)&m_MCIOpen);
	if (iErr)
	{
		MCIErrorMessageBox (iErr);
		return;
	}

	// Set the time format to track/minute/second/frame (TMSF).
	mciSet.dwTimeFormat = MCI_FORMAT_TMSF;
	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(size_t)&mciSet);
	if (iErr)
	{
		MCIErrorMessageBox (iErr);
		mciSendCommand(m_MCIOpen.wDeviceID, MCI_CLOSE, 0, 0);
		return;
	}

	I_AddExitFunc (I_ShutdownCD);
	cdaudio_started = true;

	CONS_Printf ("I_InitCD: CD Audio started\n");

	// last saved in config.cfg
	i = cd_volume.value;
	//I_SetVolumeCD (0);   // initialize to 0 for some odd cd drivers
	I_SetVolumeCD (i);   // now set the last saved volume

	for (i = 0; i < MAX_CD_TRACKS; i++)
		cdRemap[i] = (byte)i;

	if (!CD_ReadTrackInfo())
	{
		CONS_Printf("\2I_InitCD: no CD in player.\n");
		cdEnabled = FALSE;
		cdValid = FALSE;
	}
	else
	{
		cdEnabled = TRUE;
		cdValid = TRUE;
	}

	COM_AddCommand ("cd", Command_Cd_f);
}



// loop/go to next track when track is finished (if cd_update var is true)
// update the volume when it has changed (from console/menu)
void I_UpdateCD(void)
{
		/// \todo check for cd change and restart music ?
}


//
void I_PlayCD(int nTrack, boolean bLooping)
{
	MCI_PLAY_PARMS  mciPlay;
	MCIERROR        iErr;

	if (!cdaudio_started || !cdEnabled)
		return;

	//faB: try again if it didn't work (just free the user of typing 'cd reset' command)
	if (!cdValid)
		cdValid = CD_ReadTrackInfo();
	if (!cdValid)
		return;

	// tracks start at 0 in the code..
	nTrack--;
	if (nTrack < 0 || nTrack >= m_nTracksCount)
		nTrack = nTrack % m_nTracksCount;

	nTrack = cdRemap[nTrack];

	if (cdPlaying)
	{
		if (cdPlayTrack == nTrack)
			return;
		I_StopCD ();
	}

	cdPlayTrack = nTrack;

	if (!m_nTracks[nTrack].IsAudio)
	{
		//CONS_Printf ("\2CD Play: not an audio track\n"); // Tails 03-25-2001
		return;
	}

	cdLooping = bLooping;

	//faB: stop MIDI music, MIDI music will restart if volume is upped later
	cv_digmusicvolume.value = 0;
	cv_midimusicvolume.value = 0;
	I_StopSong (0);

	//faB: I don't use the notify message, I'm trying to minimize the delay
	mciPlay.dwCallback = (DWORD)((size_t)hWndMain);
	mciPlay.dwFrom = MCI_MAKE_TMSF(nTrack+1, 0, 0, 0);
	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_PLAY, MCI_FROM|MCI_NOTIFY, (DWORD)(size_t)&mciPlay);
	if (iErr)
	{
		MCIErrorMessageBox (iErr);
		cdValid = FALSE;
		cdPlaying = FALSE;
		return;
	}

	cdPlaying = TRUE;
}


// pause cd music
void I_StopCD(void)
{
	MCIERROR    iErr;

	if (!cdaudio_started || !cdEnabled)
		return;

	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_PAUSE, MCI_WAIT, 0);
	if (iErr)
		MCIErrorMessageBox (iErr);
	else
	{
		wasPlaying = cdPlaying;
		cdPlaying = FALSE;
	}
}


// continue after a pause
void I_ResumeCD(void)
{
	MCIERROR    iErr;

	if (!cdaudio_started || !cdEnabled)
		return;

	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	iErr = mciSendCommand(m_MCIOpen.wDeviceID, MCI_RESUME, MCI_WAIT, 0);
	if (iErr)
		MCIErrorMessageBox (iErr);
	else
		cdPlaying = TRUE;
}


// volume : logical cd audio volume 0-31 (hardware is 0-255)
boolean I_SetVolumeCD (int volume)
{
	UNREFERENCED_PARAMETER(volume);
	return false;
}
