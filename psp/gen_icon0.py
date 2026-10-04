#!/usr/bin/env python3
"""Generate PSP ICON0.PNG (144x80) for BadCBA – pure Python PNG."""
import os
import struct
import zlib

W, H = 144, 80
ACCENT = (108, 92, 231)
ACCENT2 = (0, 206, 201)
BG1 = (15, 12, 30)
BG2 = (30, 20, 60)

def chunk(tag, data):
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

def lerp(c1, c2, t):
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(3))

def main():
    rows = []
    for y in range(H):
        t = y / max(H - 1, 1)
        col = lerp(BG1, BG2, t)
        row = bytearray()
        for x in range(W):
            r, g, b = col
            if y < 3 or y >= H - 3:
                r, g, b = ACCENT2 if y < 3 else ACCENT
            # play circle
            cx, cy, rad = 36, 40, 22
            if (x - cx) ** 2 + (y - cy) ** 2 <= rad * rad:
                r, g, b = ACCENT
            if (x - cx) ** 2 + (y - cy) ** 2 <= (rad - 8) ** 2:
                r, g, b = (20, 15, 40)
            # triangle
            if 28 <= x <= 48 and 28 <= y <= 52:
                # simple right-pointing triangle band
                if x >= 30 + (abs(y - 40) // 2) and x <= 46:
                    r, g, b = ACCENT2
            # title bar
            if 60 <= x <= 130 and 28 <= y <= 42:
                r, g, b = ACCENT2
            if 60 <= x <= 110 and 48 <= y <= 56:
                r, g, b = (200, 200, 220)
            row.extend((r, g, b, 255))
        rows.append(b"\x00" + bytes(row))

    raw = b"".join(rows)
    ihdr = struct.pack(">IIBBBBB", W, H, 8, 6, 0, 0, 0)
    data = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ICON0.PNG")
    with open(path, "wb") as f:
        f.write(data)
    print("Wrote", path, len(data), "bytes")

if __name__ == "__main__":
    main()
