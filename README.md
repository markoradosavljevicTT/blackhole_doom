# Doom on Tenstorrent Blackhole

A port of **DOOM** to the **Tenstorrent Blackhole** card, running on 4 Tensix cores. This is not a game, it is a test kernel.

## Dependencies

```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install gcc-multilib gcc-12-multilib libx11-dev:i386 libxext-dev:i386 libnsl-dev:i386
```

## Building

```bash
cd src
SFPI=.../tt-metal/runtime/sfpi make
cd ..
```

## Running
Needs `tt_umd` (e.g. tt-metal's python_env):
```bash
python test_doom_bh.py
```
