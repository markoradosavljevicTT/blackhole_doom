// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

//static const char
//rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";



#ifndef __riscv
#include <sys/time.h>
#endif

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"


#ifdef COMBINE_SCREENS
unsigned char CombinedScreens[1];
#else
unsigned char CombinedScreens[1];
#endif


int	mb_used = 6;


void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}

unsigned char DOOMHeap[FIXED_HEAP];

int  I_GetHeapSize (void)
{
    return FIXED_HEAP;
}

byte* I_ZoneBase (int*	size)
{
    *size = FIXED_HEAP; //mb_used*1024*1024;
    return (byte *) DOOMHeap;
}


//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
#ifndef __riscv
    struct timeval	tp;
    // struct timezone	tzp;
    int			newtics;
    static int		basetime=0;
  
    gettimeofday(&tp, NULL);
    if (!basetime)
	basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
#else
    unsigned lo = *(volatile unsigned*)0xFFB121F0;
    unsigned hi = *(volatile unsigned*)0xFFB121F8;
    return ((hi << 22) | (lo >> 10)) / 22321;
#endif
}



//
// I_Init
//
void I_Init (void)
{
    ///I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();

    // crash
    while (1) {
    }
}

void I_WaitVBL(int count)
{
    //usleep (count * (1000000/700) );                                
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
    mem = CombinedScreens;
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
#ifdef __riscv
	    __asm__ __volatile__ ("ecall;");
#endif

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    I_ShutdownGraphics();
    
    // crash
    while (1) {
    }
}
