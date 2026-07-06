# Doom on Tenstorrent Quasar

A port of **DOOM** to the **Tenstorrent Quasar** card's TRISC cores. This is not a game, it is a test kernel.

https://github.com/user-attachments/assets/bda750c4-0d4c-4dc9-bb4f-c1085881695f

## Running

Requires the [tt-metal](https://github.com/tenstorrent/tt-metal/tree/marko/doom) Python test to run on ttsim for Quasar.

## Dependencies

```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install gcc-multilib gcc-12-multilib libx11-dev:i386 libxext-dev:i386 libnsl-dev:i386
```

## Building

```bash
make
```
