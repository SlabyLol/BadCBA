#!/usr/bin/env python3
"""Generate real BadCBA PNG assets (no external deps)."""
import os
import struct
import zlib

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
DATA = os.path.join(ROOT, "data")
ICONS = os.path.join(DATA, "icons")

ACCENT = (108, 92, 231)
ACCENT2 = (0, 206, 201)
TEXT = (238, 238, 245)
PANEL = (20, 20, 31)
RED = (255, 51, 51)
GREEN = (0, 184, 148)
BG = (10, 10, 18)

def _chunk(tag, data):
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

def write_png(path, w, h, rows):
    raw = b"".join(b"\x00" + r for r in rows)
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)
    out = b"\x89PNG\r\n\x1a\n" + _chunk(b"IHDR", ihdr) + _chunk(b"IDAT", zlib.compress(raw, 9)) + _chunk(b"IEND", b"")
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "wb") as f:
        f.write(out)
    print("Wrote", path, w, "x", h, len(out), "bytes")

def px(r, g, b, a=255):
    return bytes((r & 255, g & 255, b & 255, a & 255))

def lerp(c1, c2, t):
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(3))

def gradient(w, h, c1, c2):
    rows = []
    for y in range(h):
        col = lerp(c1, c2, y / max(h - 1, 1))
        rows.append(px(*col) * w)
    return rows

def fill_rect(rows, w, x0, y0, x1, y1, color):
    c = px(*color) if len(color) == 4 else px(*color, 255)
    for y in range(max(0, y0), min(len(rows), y1)):
        row = bytearray(rows[y])
        for x in range(max(0, x0), min(w, x1)):
            i = x * 4
            row[i:i+4] = c
        rows[y] = bytes(row)

def fill_circle(rows, w, cx, cy, r, color):
    c = px(*color) if len(color) == 4 else px(*color, 255)
    r2 = r * r
    for y in range(max(0, cy - r), min(len(rows), cy + r + 1)):
        row = bytearray(rows[y])
        for x in range(max(0, cx - r), min(w, cx + r + 1)):
            if (x - cx) ** 2 + (y - cy) ** 2 <= r2:
                i = x * 4
                row[i:i+4] = c
        rows[y] = bytes(row)

def fill_tri(rows, w, pts, color):
    c = px(*color) if len(color) == 4 else px(*color, 255)
    xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
    minx, maxx = max(0, min(xs)), min(w - 1, max(xs))
    miny, maxy = max(0, min(ys)), min(len(rows) - 1, max(ys))
    x0, y0 = pts[0]; x1, y1 = pts[1]; x2, y2 = pts[2]
    denom = ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2))
    if abs(denom) < 1e-6:
        return
    for y in range(miny, maxy + 1):
        row = bytearray(rows[y])
        for x in range(minx, maxx + 1):
            a = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / denom
            b = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / denom
            if a >= 0 and b >= 0 and (1 - a - b) >= 0:
                i = x * 4
                row[i:i+4] = c
        rows[y] = bytes(row)

def gen_icon0():
    w, h = 320, 176
    rows = gradient(w, h, (15, 12, 30), (30, 20, 60))
    fill_rect(rows, w, 0, 0, w, 6, ACCENT2)
    fill_rect(rows, w, 0, h - 6, w, h, ACCENT)
    fill_circle(rows, w, 60, 88, 40, ACCENT)
    fill_circle(rows, w, 60, 88, 28, (20, 15, 40))
    fill_tri(rows, w, [(48, 68), (48, 108), (85, 88)], ACCENT2)
    fill_rect(rows, w, 120, 55, 280, 78, ACCENT2)
    fill_rect(rows, w, 120, 95, 250, 110, TEXT)
    write_png(os.path.join(DATA, "ICON0.PNG"), w, h, rows)

def gen_pic1():
    w, h = 1920, 1080
    rows = gradient(w, h, (8, 8, 16), (25, 18, 45))
    for y in range(0, h, 40):
        row = bytearray(rows[y])
        for x in range(0, w, 40):
            row[x*4:x*4+3] = bytes((40, 35, 70))
        rows[y] = bytes(row)
    fill_circle(rows, w, 300, 220, 200, (50, 40, 100, 90))
    fill_circle(rows, w, 1600, 850, 250, (30, 60, 80, 80))
    fill_rect(rows, w, 120, 400, 720, 490, ACCENT2)
    fill_rect(rows, w, 120, 520, 950, 560, TEXT)
    fill_rect(rows, w, 120, 640, 450, 648, ACCENT)
    write_png(os.path.join(DATA, "PIC1.PNG"), w, h, rows)

def gen_pic0():
    w, h = 1000, 560
    rows = gradient(w, h, (12, 10, 28), (35, 25, 70))
    fill_rect(rows, w, 0, 0, w, 8, ACCENT2)
    fill_rect(rows, w, 60, 200, 520, 270, ACCENT2)
    fill_rect(rows, w, 60, 300, 720, 335, TEXT)
    write_png(os.path.join(DATA, "PIC0.PNG"), w, h, rows)

def gen_splash():
    w, h = 1280, 720
    rows = gradient(w, h, (8, 8, 18), (30, 20, 55))
    fill_rect(rows, w, 80, 280, 520, 360, ACCENT2)
    fill_rect(rows, w, 80, 390, 750, 425, TEXT)
    fill_rect(rows, w, 80, 500, 420, 510, ACCENT)
    write_png(os.path.join(DATA, "SPLASH.PNG"), w, h, rows)

def gen_logo(size, name):
    rows = gradient(size, size, (15, 12, 30), (40, 30, 80))
    m = size // 8
    fill_circle(rows, size, size // 2, size // 2, size // 2 - m, ACCENT)
    fill_circle(rows, size, size // 2, size // 2, size // 2 - 2 * m, (20, 15, 40))
    cx = cy = size // 2
    fill_tri(rows, size, [(cx - size // 8, cy - size // 6), (cx - size // 8, cy + size // 6), (cx + size // 6, cy)], ACCENT2)
    write_png(os.path.join(DATA, name), size, size, rows)

def menu_icon(name, drawer):
    w = h = 64
    rows = [px(0, 0, 0, 0) * w for _ in range(h)]
    fill_rect(rows, w, 4, 4, 60, 60, PANEL)
    fill_rect(rows, w, 4, 4, 60, 7, ACCENT)
    fill_rect(rows, w, 4, 57, 60, 60, ACCENT)
    drawer(rows, w)
    write_png(os.path.join(ICONS, name + ".png"), w, h, rows)

def main():
    os.makedirs(DATA, exist_ok=True)
    os.makedirs(ICONS, exist_ok=True)
    gen_icon0()
    gen_pic1()
    gen_pic0()
    gen_splash()
    gen_logo(128, "logo_128.png")
    gen_logo(256, "logo_256.png")
    gen_logo(512, "logo_512.png")
    menu_icon("usb", lambda r, w: (fill_rect(r, w, 24, 20, 40, 48, ACCENT2), fill_rect(r, w, 27, 12, 31, 20, TEXT), fill_rect(r, w, 33, 12, 37, 20, TEXT)))
    menu_icon("settings", lambda r, w: (fill_circle(r, w, 32, 32, 14, ACCENT2), fill_circle(r, w, 32, 32, 6, ACCENT)))
    menu_icon("convert", lambda r, w: (fill_tri(r, w, [(14, 32), (28, 20), (28, 44)], ACCENT2), fill_rect(r, w, 28, 28, 50, 36, ACCENT2)))
    menu_icon("flash", lambda r, w: (fill_rect(r, w, 20, 16, 44, 48, ACCENT), fill_rect(r, w, 26, 22, 38, 28, ACCENT2), fill_rect(r, w, 26, 32, 38, 42, GREEN)))
    menu_icon("restore", lambda r, w: (fill_circle(r, w, 32, 32, 14, ACCENT2 + (120,)), fill_tri(r, w, [(40, 14), (50, 22), (40, 28)], ACCENT2)))
    menu_icon("export", lambda r, w: (fill_tri(r, w, [(32, 12), (44, 28), (20, 28)], GREEN), fill_rect(r, w, 28, 28, 36, 50, GREEN)))
    menu_icon("about", lambda r, w: (fill_circle(r, w, 32, 32, 16, ACCENT2), fill_rect(r, w, 30, 28, 34, 42, BG), fill_circle(r, w, 32, 22, 3, BG)))
    menu_icon("exit", lambda r, w: (fill_rect(r, w, 18, 18, 46, 46, RED), fill_rect(r, w, 24, 30, 40, 34, TEXT)))
    menu_icon("error", lambda r, w: (fill_tri(r, w, [(32, 10), (54, 50), (10, 50)], RED), fill_rect(r, w, 30, 22, 34, 36, TEXT)))
    menu_icon("mp4", lambda r, w: (fill_rect(r, w, 12, 16, 52, 48, ACCENT), fill_tri(r, w, [(28, 24), (28, 40), (44, 32)], TEXT)))
    menu_icon("folder", lambda r, w: (fill_rect(r, w, 12, 24, 52, 48, ACCENT2), fill_rect(r, w, 12, 18, 30, 24, ACCENT)))
    print("All assets ready in", DATA)

if __name__ == "__main__":
    main()
