# Installing BadCBA on PS3

## Error codes

| Code | Meaning | Fix |
|------|---------|-----|
| **80029564** | Invalid / fake PKG | Do **not** use Package Manager on stub PKG. Use folder install. |
| **80029533** | EBOOT.BIN not a valid SELF (raw ELF) | Rebuild with **`fself -n`**. New builds refuse ELF as EBOOT. |

## Recommended: folder install (CFW / HEN)

1. Download **`BCBA00001_folder.zip`** from Releases / Actions
2. Extract → folder `BCBA00001`
3. multiMAN / IrisMAN / FTP → copy to **`/dev_hdd0/game/BCBA00001/`**
4. Structure must be:

```
/dev_hdd0/game/BCBA00001/
├── PARAM.SFO
├── ICON0.PNG
└── USRDIR/
    ├── EBOOT.BIN    ← must be SELF, not ELF
    ├── version.dat
    └── error-cba.wav  (optional)
```

5. Reboot or refresh XMB → Game → BadCBA

### HEN
Enable HEN before starting homebrew if required on your firmware.

## Rebuild (devs)

```bash
export PS3DEV=/usr/local/ps3dev
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin
make clean && make && make folder-install
```

Requires **`fself`** from ps3dev. Without it, `make` fails instead of shipping a broken EBOOT.
