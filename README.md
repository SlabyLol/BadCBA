# BadCBA

**BadCustomBootAnimation** – custom boot tools for **PS3** and **PSP**.

| Platform | Folder | Output |
|----------|--------|--------|
| **PS3** | `/` (root) | `BadCBA.pkg` |
| **PSP** | [`psp/`](psp/) | `EBOOT.PBP` |

![PS3](https://img.shields.io/badge/PS3-Homebrew-purple) ![PSP](https://img.shields.io/badge/PSP-Homebrew-blue) ![License](https://img.shields.io/badge/license-MIT-blue)

## What BadCBA means

**Bad** **C**ustom **B**oot **A**nimation

---

## PS3

Native homebrew with RSX GUI. Pick **MP4** from USB, convert, install custom coldboot to flash (CFW).

- See root `source/`, `Makefile`, workflow **Build BadCBA PKG**
- Install: Package Manager → `BadCBA.pkg`

## PSP

Homebrew under [`psp/`](psp/) – Memory Stick browser, settings, prepare `ms0:/BadCBA_Boot/`.

- Workflow: **Build BadCBA PSP** (Docker `pspdev/pspdev:latest`)
- Install: `ms0:/PSP/GAME/BadCBA/EBOOT.PBP`

```bash
docker run --rm -v "$PWD:/source" -w /source/psp pspdev/pspdev:latest make
```

## License

MIT – see [LICENSE](LICENSE)
