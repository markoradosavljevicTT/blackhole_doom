#ifndef I_VIDEO_PAJSERI
#define I_VIDEO_PAJSERI

#include "doomtype.h"

extern unsigned char lpalette[256];
#define LPALETTE lpalette

#define BUFFER_SWITCH 0x1F004

#define GPU_ADDR1 0x0001F100
#define GPU_ADDR2 0x0002EB00

#define FRAMEBUFFER ((*((byte*)BUFFER_SWITCH)) ? ((byte*)GPU_ADDR2) : ((byte*)GPU_ADDR1))

#endif
