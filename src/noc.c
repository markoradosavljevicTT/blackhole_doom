#include <stdint.h>
#include "stdlib.h"

#define NOC_BASE 0x10000000
#define NOC_CHUNK 0x100000
#define NOC_L1_ADDR 0x10000

static const uint8_t noc_x[] = {2, 3, 4, 5};
#define NOC_Y 2

#define NOC_RD(off) (*(volatile uint32_t *)(0xFFB20800 + (off)))
#define NOC_RD_RESP_RECEIVED (*(volatile uint32_t *)0xFFB20208)
#define NOC_MY_XY (*(volatile uint32_t *)0xFFB20148)

static uint8_t bounce[4096 + 16] __attribute__((aligned(16)));

static void noc_read(uint32_t x, uint32_t y, uint32_t src, uint32_t dst, uint32_t len)
{
	uint32_t received = NOC_RD_RESP_RECEIVED;
	while (NOC_RD(0x40));
	NOC_RD(0x1C) = (1 << 4) | (1 << 7) | (1 << 13);
	NOC_RD(0x0C) = dst;
	NOC_RD(0x10) = 0;
	NOC_RD(0x14) = NOC_MY_XY & 0xFFF;
	NOC_RD(0x00) = src;
	NOC_RD(0x04) = 0;
	NOC_RD(0x08) = (y << 6) | x;
	NOC_RD(0x20) = len;
	NOC_RD(0x40) = 1;
	while (NOC_RD_RESP_RECEIVED == received);
}

void noc_copy(void *dst, const void *src, int len)
{
	uint32_t off = (uint32_t)src - NOC_BASE;
	uint8_t *d = dst;

	while (len > 0) {
		uint32_t local = off % NOC_CHUNK;
		uint32_t n = len < 4096 ? len : 4096;
		if (n > NOC_CHUNK - local)
			n = NOC_CHUNK - local;
		uint32_t addr = NOC_L1_ADDR + local;
		noc_read(noc_x[off / NOC_CHUNK], NOC_Y, addr & ~15, (uint32_t)bounce, ((addr & 15) + n + 15) & ~15);
		memcpy(d, bounce + (addr & 15), n);
		d += n;
		off += n;
		len -= n;
	}
}
