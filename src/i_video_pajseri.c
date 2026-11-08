#include "d_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "i_video_pajseri.h"

byte lpalette[256];

void I_InitGraphics(void)
{
}

void I_ShutdownGraphics(void)
{
}

void I_SetPalette(byte *palette)
{
	for(int i = 0; i < 256; i++) {
		uint8_t b = (palette[i * 3 + 2] & 0xc0);
		uint8_t g = (palette[i * 3 + 1] & 0xe0) >> 2;
		uint8_t r = (palette[i * 3 + 0] & 0xe0) >> 5;
		lpalette[i] = b | g | r;
	}
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
	byte* buffer = (byte*)BUFFER_SWITCH;
	*buffer = *buffer ? (byte)0 : (byte)1;
}

void I_StartFrame()
{
}

#define KEY_ADDR 0x1F000UL
#define ESC 0x80
#define SPACE 0x40
#define D 0x20
#define S 0x10
#define A 0x08
#define W 0x04
#define SHIFT 0x02
#define CTRL 0x01

uint8_t kb_status = 0;

void I_StartTic()
{
	volatile uint8_t* keyboard = (uint8_t*)KEY_ADDR;
	uint8_t key = *keyboard;
	if (ESC & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_PAUSE;
		D_PostEvent(&event);
		kb_status |= ESC;
	} else if (kb_status & ESC) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_PAUSE;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~ESC;
	}

	if (SPACE & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = ' ';
		D_PostEvent(&event);
		kb_status |= SPACE;
	} else if (kb_status & SPACE) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = ' ';
		D_PostEvent(&event);
		kb_status &= (uint8_t)~SPACE;
	}

	if (D & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_RIGHTARROW;
		D_PostEvent(&event);
		kb_status |= D;
	} else if (kb_status & D) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_RIGHTARROW;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~D;
	}

	if (S & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_DOWNARROW;
		D_PostEvent(&event);
		kb_status |= S;
	} else if (kb_status & S) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_DOWNARROW;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~S;
	}

	if (A & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_LEFTARROW;
		D_PostEvent(&event);
		kb_status |= A;
	} else if (kb_status & A) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_LEFTARROW;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~A;
	}

	if (W & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_UPARROW;
		D_PostEvent(&event);
		kb_status |= W;
	} else if (kb_status & W) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_UPARROW;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~W;
	}

	if (SHIFT & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_ENTER;
		D_PostEvent(&event);
		kb_status |= SHIFT;
	} else if (kb_status & SHIFT) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_ENTER;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~SHIFT;
	}

	if (CTRL & key) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_RCTRL;
		D_PostEvent(&event);
		kb_status |= CTRL;
	} else if (kb_status & CTRL) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = KEY_RCTRL;
		D_PostEvent(&event);
		kb_status &= (uint8_t)~CTRL;
	}
}
