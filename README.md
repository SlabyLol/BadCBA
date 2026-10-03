# BadCBA

**BadCustomBootAnimation** – Create custom PS3 coldboot animations from **MP4** directly on your console.

BadCBA is a native PS3 homebrew app with a full on-screen GUI.  
Install the PKG, pick an **MP4** from USB, convert it into a custom boot animation package – all on the PS3.

![BadCBA](https://img.shields.io/badge/PS3-Homebrew-purple) ![Input](https://img.shields.io/badge/input-MP4-cyan) ![License](https://img.shields.io/badge/license-MIT-blue)

## What BadCBA means

**Bad** **C**ustom **B**oot **A**nimation

## Features

### Real On-Screen GUI
- Dark modern theme
- Controller navigation
- Menus: Main, USB browser, Settings, Convert, About
- Bitmap font on RSX framebuffer

### Media support
- **Primary input: MP4** (also accepts MP3 / M4V)
- USB file browser for media files
- Video frames → `coldboot.raf` (boot animation)
- Audio track → `coldboot_stereo.ac3` + `coldboot_multi.ac3`

### Settings
- Max duration (1–8 s)
- Volume (0–100 %)
- Fade in/out
- Backup existing coldboot files
- Sample rate
- Content ID & PKG name

### Output
- Convert & create package structure
- Export to USB (`/dev_usb000/BadCBA_Coldboot/`)

## Requirements

- PS3 with CFW or PS3HEN
- USB (FAT32) with your **MP4** files
- multiMAN / IrisMAN / webMAN for flash operations

## Installation

1. Download `BadCBA.pkg` from [Releases](https://github.com/SlabyLol/BadCBA/releases)
2. Copy to USB → Package Manager → Install
3. Launch **BadCBA** under Game

## How to use

1. Put your MP4 on USB
2. Open BadCBA → **Select MP4 from USB**
3. Choose the file with **X**
4. Adjust **Settings** if needed
5. **Convert & Create PKG** or **Export to USB**

## Building

```bash
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin

make icons
make
make pkg
```

## Project structure

```
BadCBA/
├── source/          # main, gui, filebrowser, settings, audio, pkg, rsxutil
├── include/
├── data/            # ICON0.PNG, PIC1.PNG, version.dat
├── tools/           # gen_icons.py, sfo.py, pkg.py
├── Makefile
└── .github/workflows/
```

## Safety

- Do not write to `/dev_flash` without backups
- Prefer Export to USB + manual copy
- Use at your own risk

## License

MIT – see [LICENSE](LICENSE)
