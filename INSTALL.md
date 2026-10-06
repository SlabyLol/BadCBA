# Installing BadCBA on PS3

## Error codes

| Code | Meaning | What to do |
|------|---------|------------|
| **80029564** | Invalid PKG | Use **folder install**, not Package Manager |
| **80029533** | EBOOT is raw ELF | Need build with `fself -n` |
| **80010009** | SELF / version / HEN issue | See below |

## Folder install (recommended)

1. Download **`BCBA00001_folder.zip`**
2. Extract → `BCBA00001/`
3. Copy to **`/dev_hdd0/game/BCBA00001/`** (multiMAN / IrisMAN / FTP)
4. Must look like:

```
/dev_hdd0/game/BCBA00001/
├── PARAM.SFO
├── ICON0.PNG
└── USRDIR/
    └── EBOOT.BIN
```

## Error 80010009 – checklist

1. **HEN / CFW aktiv?**  
   Auf HEN: zuerst **Enable HEN**, dann BadCBA starten.

2. **Alten Ordner löschen** und neu kopieren (kompletter `BCBA00001`).

3. **Start über multiMAN**  
   multiMAN → browse to `/dev_hdd0/game/BCBA00001/USRDIR/EBOOT.BIN`  
   or use **app_home** if you use webMAN MOD.

4. **PARAM.SFO**  
   `PS3_SYSTEM_VER` is set to `03.4100` (homebrew-friendly).  
   Rebuild if you still have an old SFO with 04.80.

5. **QA Flag** (CFW / Rebug Toolbox): optional enable for homebrew.

## Rebuild

```bash
export PS3DEV=/usr/local/ps3dev
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin
make clean && make && make folder-install
```

Requires **fself** from ps3dev.
