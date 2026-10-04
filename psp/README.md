# BadCBA for PSP (full)

**BadCustomBootAnimation** – complete PSP homebrew port.

## Features

| Feature | Description |
|---------|-------------|
| Media browser | `ms0:` – MP4, MP3, PMF, AVI, AT3, WAV… |
| Settings | Duration, volume, fade, backup ON/OFF (saved) |
| Convert | Copy media + write `boot_info.txt` to `ms0:/BadCBA_Boot/` |
| Install | Copy prepared files to `ms0:/BadCBA_Boot/installed/` |
| Restore | Restore from `ms0:/PSP/GAME/BadCBA/backup/` |
| Export | `ms0:/BadCBA_Export/` |
| Errors | Full codes; **(err:726)** no MS; `error-cba.wav` loops **until app exit** |

## Install

```
ms0:/PSP/GAME/BadCBA/EBOOT.PBP
ms0:/PSP/GAME/BadCBA/error-cba.wav   (optional)
```

## Build

```bash
docker run --rm -v "$PWD:/source" -w /source/psp pspdev/pspdev:latest make
```

Optional icon: `python3 gen_icon0.py` then `make`.
