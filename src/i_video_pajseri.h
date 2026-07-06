#ifndef I_VIDEO_PAJSERI
#define I_VIDEO_PAJSERI

#include "doomtype.h"

#define LPALETTE (byte*)0x00031c00

#define BUFFER_SWITCH 0x1F004

#define GPU_ADDR1 0x00020000
#define GPU_ADDR2 0x00024000

#define FRAMEBUFFER ((*((byte*)BUFFER_SWITCH)) ? ((byte*)GPU_ADDR2) : ((byte*)GPU_ADDR1))

#endif
