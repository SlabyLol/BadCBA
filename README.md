# BadCBA

**Bad Custom Boot Audio** – Create custom PS3 coldboot sounds directly on your console.

BadCBA is a full native PS3 homebrew application with a real on-screen GUI.  
Install the PKG, launch it from the XMB, select an MP3 from USB and create your custom boot sound package – all on the PS3.

![BadCBA](https://img.shields.io/badge/PS3-Homebrew-purple) ![License](https://img.shields.io/badge/license-MIT-blue)

## Features

### Real On-Screen GUI
- Dark modern theme with accent colors
- Full controller navigation (Up/Down/Left/Right + X / O)
- Smooth menus, file browser, settings, convert screen
- Bitmap font rendering directly on the RSX framebuffer

### Core Functions
- **USB File Browser** – Browse any connected USB and pick `.mp3` files
- **Settings**
  - Max duration (1–8 seconds)
  - Volume (0–100 %)
  - Fade in / fade out (0–2000 ms)
  - Backup existing coldboot files
  - Sample rate (44100 / 48000 Hz)
  - Content ID & PKG name
- **Convert & Create PKG** – Prepare coldboot audio package
- **Export to USB** – Write files + README to `/dev_usb000/BadCBA_Coldboot/`

### Technical
- Native PSL1GHT application
- Dual-buffered RSX rendering
- Proper sysutil exit handling
- Ready for real AC3 encoding libraries

## Requirements

- PS3 with **CFW** or **PS3HEN**
- USB drive (FAT32) with your MP3 files
- multiMAN / IrisMAN / webMAN recommended for advanced flash operations

## Installation

1. Download `BadCBA.pkg` from [Releases](https://github.com/SlabyLol/BadCBA/releases)
2. Copy to USB
3. On PS3: **Package Manager → Install Package Files**
4. Launch **BadCBA** under Game in the XMB

## How to use

1. Plug in USB with MP3s
2. Start BadCBA
3. Choose **Select MP3 from USB**
4. Navigate and press **X** on your file
5. Adjust **Settings** if needed
6. Go to **Convert & Create PKG** and press **X**
7. Or use **Export to USB** for manual installation

## Building

### Prerequisites

```bash
# Install ps3toolchain + PSL1GHT first
export PS3DEV=/usr/local/ps3dev   # or /opt/ps3dev
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PSL1GHT/host/bin
```

### Build commands

```bash
git clone https://github.com/SlabyLol/BadCBA.git
cd BadCBA
make          # builds BadCBA.elf
make pkg      # creates BadCBA.pkg
```

### GitHub Actions

Every push and release triggers a build workflow.  
Once a full toolchain cache is available, the workflow produces a ready-to-install `.pkg`.

## Project structure

```
BadCBA/
├── source/
│   ├── main.c          # Entry, video init, main loop
│   ├── gui.c           # Full RSX on-screen GUI + font
│   ├── filebrowser.c   # USB browser with GUI
│   ├── settings.c      # Settings + GUI
│   ├── audio.c         # Conversion & export
│   └── pkg.c           # Package helpers
├── include/
├── data/               # ICON0.PNG, PIC1.PNG
├── Makefile            # Real PSL1GHT build
├── sfo.xml
└── .github/workflows/  # CI
```

## Important safety notes

- Writing to `/dev_flash` or `/dev_blind` can brick your console
- Always keep backups of original coldboot files
- Prefer the **Export to USB** method + manual copy with multiMAN
- BadCBA itself does not write to flash by default

## Credits

- PSL1GHT / ps3dev community
- multiMAN, IrisMAN, webMAN authors
- Everyone who documented coldboot AC3 / RAF formats

## License

MIT License – see [LICENSE](LICENSE)
