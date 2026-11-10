import atexit
import os
import select
import signal
import struct
import sys
import termios
import time

import tt_umd

_DIR = os.path.dirname(os.path.abspath(__file__))

DOOM_ELF = os.path.join(_DIR, "src/bh_doom")

TILE_X = 1
TILE_Y = 2

NOC_BASE = 0x10000000
NOC_CHUNK = 0x100000
NOC_L1_ADDR = 0x10000
NOC_TILES = [(2, 2), (3, 2), (4, 2), (5, 2)]

SCREEN_WIDTH = 320
SCREEN_HEIGHT = 200

GPU_ADDR1 = 0x0001F100
GPU_ADDR2 = 0x0002EB00
KB_ADDR = 0x0001F000
BUFFER_SWITCH_ADDR = 0x0001F004
FRAME_ACK_ADDR = 0x0001F008

KB_ESC = 0x80
KB_SPACE = 0x40
KB_D = 0x20
KB_S = 0x10
KB_A = 0x08
KB_W = 0x04
KB_SHIFT = 0x02
KB_CTRL = 0x01

KEY_HOLD = 0.1

SOFT_RESET_0 = 0xFFB121B0
RESET_ALL = 0x47800
RESET_RUN_BRISC = 0x47000

rgb_r = [((i & 0x07) >> 0) * 36 for i in range(256)]
rgb_g = [((i & 0x38) >> 3) * 36 for i in range(256)]
rgb_b = [((i & 0xC0) >> 6) * 85 for i in range(256)]


_dev = None


def _get_dev():
    global _dev
    if _dev is None:
        _dev = tt_umd.TTDevice.create(0)
        _dev.init_tt_device()
    return _dev


def tile_write(addr, data, x=TILE_X, y=TILE_Y):
    _get_dev().noc_write(x, y, addr, bytes(data))


def tile_read(addr, size):
    return b"".join(_get_dev().noc_read(TILE_X, TILE_Y, addr + off, min(2048, size - off)) for off in range(0, size, 2048))


def set_reset(value):
    for x, y in [(TILE_X, TILE_Y)] + NOC_TILES:
        _get_dev().noc_write32(x, y, SOFT_RESET_0, value)


def parse_elf_segments(path):
    with open(path, "rb") as f:
        ehdr = f.read(52)
        if ehdr[:4] != b"\x7fELF":
            raise ValueError("Not an ELF file")
        if ehdr[4] != 1:
            raise ValueError("Not a 32-bit ELF")

        e_entry = struct.unpack_from("<I", ehdr, 24)[0]
        e_phoff = struct.unpack_from("<I", ehdr, 28)[0]
        e_phentsize = struct.unpack_from("<H", ehdr, 42)[0]
        e_phnum = struct.unpack_from("<H", ehdr, 44)[0]

        segments = []
        for i in range(e_phnum):
            f.seek(e_phoff + i * e_phentsize)
            phdr = f.read(e_phentsize)
            p_type = struct.unpack_from("<I", phdr, 0)[0]
            if p_type != 1:
                continue
            p_offset = struct.unpack_from("<I", phdr, 4)[0]
            p_vaddr = struct.unpack_from("<I", phdr, 8)[0]
            p_filesz = struct.unpack_from("<I", phdr, 16)[0]
            p_memsz = struct.unpack_from("<I", phdr, 20)[0]

            f_pos = f.tell()
            f.seek(p_offset)
            data = f.read(p_filesz)
            f.seek(f_pos)

            if p_memsz > p_filesz:
                data += b"\x00" * (p_memsz - p_filesz)

            segments.append((p_vaddr, data))

        return e_entry, segments


def load_elf_to_device(elf_path):
    entry, segments = parse_elf_segments(elf_path)
    for vaddr, data in segments:
        if vaddr >= NOC_BASE:
            for off in range(0, len(data), NOC_CHUNK):
                x, y = NOC_TILES[off // NOC_CHUNK]
                tile_write(NOC_L1_ADDR, data[off : off + NOC_CHUNK], x, y)
        elif vaddr == 0:
            tile_write(0, data[:0x10])
            tile_write(0x20, data[0x20:])
        else:
            tile_write(vaddr, data)
    return entry


old_termios = None


def setup_terminal():
    global old_termios
    old_termios = termios.tcgetattr(sys.stdin)
    new = termios.tcgetattr(sys.stdin)
    new[3] &= ~(termios.ICANON | termios.ECHO)
    new[6][termios.VMIN] = 0
    new[6][termios.VTIME] = 0
    termios.tcsetattr(sys.stdin, termios.TCSANOW, new)
    sys.stdout.write("\033[2J\033[H\033[?25l\033[?1049h")
    sys.stdout.write("\033[8;%d;%dt" % (SCREEN_HEIGHT // 2, SCREEN_WIDTH))
    sys.stdout.flush()
    signal.signal(signal.SIGWINCH, lambda *_: _last_rows.clear())


def restore_terminal():
    if old_termios is not None:
        termios.tcsetattr(sys.stdin, termios.TCSANOW, old_termios)
    sys.stdout.write("\033[2J\033[H\033[?25h\033[?1049l")
    sys.stdout.flush()


def poll_keyboard():
    key_pressed = 0
    while select.select([sys.stdin], [], [], 0)[0]:
        buf = os.read(sys.stdin.fileno(), 32)
        for c in buf:
            if c == ord("w") or c == ord("W"):
                key_pressed |= KB_W
            elif c == ord("s") or c == ord("S"):
                key_pressed |= KB_S
            elif c == ord("a") or c == ord("A"):
                key_pressed |= KB_A
            elif c == ord("d") or c == ord("D"):
                key_pressed |= KB_D
            elif c == ord(" "):
                key_pressed |= KB_SPACE
            elif c == 27:
                key_pressed |= KB_ESC
            elif c == 13 or c == 10:
                key_pressed |= KB_CTRL
            elif c == ord("e") or c == ord("E"):
                key_pressed |= KB_SHIFT
            elif c == 3:
                raise KeyboardInterrupt
    return key_pressed


screen_buffer = bytearray(SCREEN_WIDTH * SCREEN_HEIGHT)


_esc_cache = {}
_last_rows = {}


def render_framebuffer(fb_data):
    parts = ["\033[?2026h"]
    if not _last_rows:
        parts.append("\033[2J")
    cur_fg = cur_bg = None
    cur_esc = None
    for y in range(0, SCREEN_HEIGHT, 2):
        row = []
        for x in range(SCREEN_WIDTH):
            top = fb_data[y * SCREEN_WIDTH + x]
            bot = fb_data[(y + 1) * SCREEN_WIDTH + x] if y + 1 < SCREEN_HEIGHT else top
            fg = (rgb_r[top], rgb_g[top], rgb_b[top])
            bg = (rgb_r[bot], rgb_g[bot], rgb_b[bot])
            if fg != cur_fg or bg != cur_bg:
                key = (fg, bg)
                esc = _esc_cache.get(key)
                if esc is None:
                    esc = "\033[38;2;%d;%d;%d;48;2;%d;%d;%dm" % (fg[0], fg[1], fg[2], bg[0], bg[1], bg[2])
                    _esc_cache[key] = esc
                cur_esc = esc
                cur_fg, cur_bg = fg, bg
                row.append(cur_esc)
            row.append("▀")
        row.append("\033[0m")
        row = "".join(row)
        if _last_rows.get(y) != row:
            _last_rows[y] = row
            parts.append("\033[%d;1H" % (y // 2 + 1) + row)
        cur_fg = cur_bg = None
    parts.append("\033[?2026l")
    os.write(sys.stdout.fileno(), "".join(parts).encode("utf-8"))


def run_doom():
    atexit.register(restore_terminal)
    is_tty = sys.stdout.isatty()

    set_reset(RESET_ALL)
    load_elf_to_device(DOOM_ELF)
    tile_write(BUFFER_SWITCH_ADDR, bytes(8))
    _get_dev().noc_write32(TILE_X, TILE_Y, SOFT_RESET_0, RESET_RUN_BRISC)

    setup_terminal()

    prev_switch = 0xFF
    key_pressed = 0
    key_time = {KB_W: 0.0, KB_S: 0.0, KB_A: 0.0, KB_D: 0.0}

    try:
        while True:
            time.sleep(0.005)

            buf_switch = tile_read(BUFFER_SWITCH_ADDR, 1)[0]

            if buf_switch != prev_switch:
                prev_switch = buf_switch
                key_pressed = 0

                read_addr = GPU_ADDR1 if buf_switch else GPU_ADDR2
                fb_data = tile_read(read_addr, SCREEN_WIDTH * SCREEN_HEIGHT)
                tile_write(FRAME_ACK_ADDR, struct.pack("<I", buf_switch))

                screen_buffer[:] = fb_data
                if is_tty:
                    render_framebuffer(screen_buffer)

            keys = poll_keyboard()
            now = time.time()
            for bit in key_time:
                if keys & bit:
                    key_time[bit] = now
            key_pressed |= keys
            held = sum(bit for bit in key_time if now - key_time[bit] < KEY_HOLD)
            tile_write(KB_ADDR, bytes([key_pressed | held]))

    except KeyboardInterrupt:
        pass
    finally:
        set_reset(RESET_ALL)
        restore_terminal()


def test_doom():
    run_doom()


if __name__ == "__main__":
    run_doom()
