# BadCBA

**Bad Custom Boot Audio** – Create custom PS3 coldboot sounds directly on your console.

BadCBA is a full PS3 homebrew application (installable as PKG) that lets you select an MP3 from USB, convert it into a proper PS3 coldboot audio package, and install or export it – all from a clean on-console GUI.

## Features

- **On-console GUI** – Native PS3 interface, controller navigation
- **USB File Browser** – Browse any USB drive and select `.mp3` files
- **MP3 Preview** – Play the selected track before conversion (limited by available audio libs)
- **Audio Conversion Pipeline**
  - Trim to safe coldboot length (max ~8 seconds recommended)
  - Fade in / Fade out
  - Volume adjustment
  - Stereo / Multi-channel AC3 output (`coldboot_stereo.ac3` + `coldboot_multi.ac3`)
- **PKG Builder** – Generate a ready-to-install coldboot package
- **Safe Install Options**
  - Export files to USB for manual installation via multiMAN / IrisMAN
  - Create an installer PKG that copies the files to the correct locations
- **Extensive Settings**
  - Max duration (1–8 s)
  - Fade duration
  - Volume (0–100 %)
  - Sample rate handling
  - Content ID / Title ID for generated PKGs
  - Custom package name & version
  - Auto-create coldboot.raf placeholder support
  - Backup existing coldboot files before overwrite
- **GitHub Actions** – Automatic builds of the `.pkg` on every release / tag

## Requirements

- PS3 with CFW or PS3HEN
- USB drive (FAT32) containing your `.mp3` files
- multiMAN / IrisMAN / webMAN recommended for flash write access (if using direct install)

## Installation

1. Download the latest `BadCBA.pkg` from the [Releases](https://github.com/SlabyLol/BadCBA/releases) page.
2. Copy it to a USB drive.
3. On the PS3 go to **Package Manager → Install Package Files** and install BadCBA.
4. Launch **BadCBA** from the XMB under Game.

## How to use

1. Insert a USB stick with your MP3 files.
2. Open BadCBA.
3. Navigate to **Select MP3 from USB**.
4. Choose your file.
5. Adjust settings (duration, fade, volume, etc.).
6. Press **Convert & Create PKG** or **Export to USB**.
7. Install the generated package or copy the AC3 files manually.

## Coldboot Files Explained

A custom coldboot usually consists of:

| File                  | Purpose                          |
|-----------------------|----------------------------------|
| `coldboot_stereo.ac3` | Main stereo boot sound           |
| `coldboot_multi.ac3`  | Multi-channel version            |
| `coldboot.raf`        | Boot animation (logo)            |

BadCBA focuses on the audio part and can generate the two AC3 files + an installer PKG. The `.raf` can be provided separately or left as original.

**Important:** Writing to `/dev_flash` / `/dev_blind` can brick your console if done incorrectly. Always keep a backup and prefer the safer “Export to USB + manual copy” method.

## Building from Source

### Prerequisites

- Linux (recommended) or WSL
- [ps3toolchain](https://github.com/ps3dev/ps3toolchain)
- [PSL1GHT](https://github.com/ps3dev/PSL1GHT)

```bash
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PSL1GHT/host/bin
```

### Build

```bash
git clone https://github.com/SlabyLol/BadCBA.git
cd BadCBA
make
make pkg
```

The resulting `BadCBA.pkg` will be in the project root.

## Project Structure

```
BadCBA/
├── source/
│   ├── main.c              # Entry point & main loop
│   ├── gui.c / gui.h       # GUI rendering & menus
│   ├── filebrowser.c       # USB / HDD file browser
│   ├── audio.c             # MP3 handling & conversion stubs
│   ├── pkg.c               # PKG generation helpers
│   └── settings.c          # Persistent settings
├── include/
├── data/                   # Icons, fonts, default assets
├── Makefile
├── sfo.xml                 # PARAM.SFO template
├── .github/workflows/      # CI for automatic PKG builds
└── README.md
```

## GitHub Actions

Every push to `main` and every tag triggers a full build using a pre-configured PS3 toolchain Docker image. The resulting `.pkg` is uploaded as a release artifact.

## Disclaimer

- This is homebrew for CFW / HEN only.
- Incorrect use of flash write tools can permanently damage your console.
- Use at your own risk.
- BadCBA does not contain any copyrighted Sony files.

## Credits

- PSL1GHT / ps3dev community
- multiMAN / IrisMAN / webMAN authors
- Everyone who documented the coldboot AC3 / RAF formats

## License

MIT License – see [LICENSE](LICENSE)
