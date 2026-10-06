# Installing BadCBA on PS3

## Error 80029564

This error means the **Package Manager rejected the PKG** (invalid or incomplete package format).

Our CI prefers the official **ps3dev `pkg.py` / `sfo.py` / `fself`**. If those tools are missing, the fallback PKG is **not** installable via XMB Package Manager.

### Fix options

### A) Folder install (recommended on CFW)

Does **not** need a PKG:

1. On PC, create folder structure:

```
BCBA00001/
├── PARAM.SFO
├── ICON0.PNG
└── USRDIR/
    ├── EBOOT.BIN
    ├── version.dat
    └── error-cba.wav   (optional)
```

2. Copy `BCBA00001` to USB.
3. On PS3 with **multiMAN / IrisMAN / File Manager**:
   - Copy to `/dev_hdd0/game/BCBA00001/`
4. Refresh XMB (or reboot) → **BadCBA** under Game.

### B) Install PKG with homebrew tools

- Put `BadCBA.pkg` on USB
- Install with **multiMAN → mmCM → Install PKG** or **IrisMAN**
- Not the official Sony Package Manager if the PKG is unsigned/debug

### C) Rebuild with full toolchain

```bash
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin
make clean && make && make pkg
```

Needs working `fself` / `make_self_npdrm`, `sfo.py`, `pkg.py` from ps3dev.

## HEN users

Enable **HAN / HEN** before installing packages if required on your firmware.
