# Installing BadCBA – HEN

## Must do on HEN

1. **Every time** after reboot: **Enable HEN** first, then start BadCBA.
2. Use **folder install** (not the broken old PKG).
3. New builds use **`fself` without `-n`** for the folder EBOOT (correct for HEN).

## Install steps

1. Download latest **`BCBA00001_folder.zip`** (GitHub Actions / Release)
2. Delete old folder on PS3: `/dev_hdd0/game/BCBA00001`
3. Copy new `BCBA00001/` to `/dev_hdd0/game/BCBA00001/`
4. **Enable HEN**
5. XMB → Game → BadCBA

## Still 80010009?

- HEN really enabled? (try again after Enable HEN)
- Full delete + recopy of `BCBA00001`
- Start `EBOOT.BIN` from multiMAN file browser
- Install webMAN MOD and retry

| Code | Meaning |
|------|--------|
| 80029564 | Bad PKG → folder zip only |
| 80029533 | EBOOT was ELF |
| 80010009 | HEN off / old SELF / wrong install |
