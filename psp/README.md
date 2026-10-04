# BadCBA for PSP

**BadCustomBootAnimation** – PSP homebrew port.

## Features

- On-screen menu (debug font + GU)
- Memory Stick browser (`ms0:`)
- Select MP4 / MP3 / PMF media
- Settings: duration, volume
- Prepare boot folder: `ms0:/BadCBA_Boot/`
- No MS → red error style **(err:726)**

## Requirements

- PSP with **CFW** (or PRO / ME / Infinity)
- Memory Stick

## Install

1. Download `EBOOT.PBP` from [Releases](https://github.com/SlabyLol/BadCBA/releases)
2. Copy to `ms0:/PSP/GAME/BadCBA/EBOOT.PBP`
3. Run from the XMB under Game → Memory Stick

## Build (local)

```bash
# Using Docker
docker run --rm -v "$PWD:/source" -w /source/psp pspdev/pspdev:latest make

# Or native PSPDEV
export PSPDEV=/usr/local/pspdev
export PATH=$PATH:$PSPDEV/bin
cd psp && make
```

Output: `EBOOT.PBP`

## GitHub Actions

Workflow `.github/workflows/build-psp.yml` builds with `pspdev/pspdev:latest` and uploads `EBOOT.PBP`.
