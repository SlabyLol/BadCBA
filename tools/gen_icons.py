#!/usr/bin/env python3
"""
Generate ICON0.PNG (320x176) and PIC1.PNG (1920x1080) for BadCBA.
Pure Python, no external dependencies.
"""

import struct
import zlib
import os

def write_png(filename, width, height, row_func):
    """row_func(y) -> bytes of one scanline (filter byte + RGBA pixels)"""
    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)

    compressor = zlib.compressobj(9)
    compressed = b""
    for y in range(height):
        compressed += compressor.compress(row_func(y))
    compressed += compressor.flush()

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", ihdr)
    data += chunk(b"IDAT", compressed)
    data += chunk(b"IEND", b"")

    os.makedirs(os.path.dirname(filename) or ".", exist_ok=True)
    with open(filename, "wb") as f:
        f.write(data)
    print(f"Wrote {filename} ({width}x{height})")


def lerp(a, b, t):
    return int(a + (b - a) * t)


def gen_icon(path):
    w, h = 320, 176

    def row(y):
        line = bytearray()
        line.append(0)
        t = y / h
        for x in range(w):
            r = lerp(10, 40, t)
            g = lerp(10, 20, t)
            b = lerp(25, 60, t)

            if y < 6:
                r, g, b = 108, 92, 231

            if 90 < x < 110 and 40 < y < 136:
                r, g, b = 0, 206, 201
            if 110 <= x < 160 and 40 < y < 80:
                if ((x - 110) / 50) ** 2 + ((y - 60) / 25) ** 2 < 1:
                    r, g, b = 0, 206, 201
            if 110 <= x < 170 and 90 < y < 136:
                if ((x - 110) / 55) ** 2 + ((y - 113) / 28) ** 2 < 1:
                    r, g, b = 0, 206, 201

            line.extend((r, g, b, 255))
        return bytes(line)

    write_png(path, w, h, row)


def gen_background(path):
    w, h = 1920, 1080

    def row(y):
        line = bytearray()
        line.append(0)
        t = y / h
        for x in range(w):
            r = lerp(8, 25, t)
            g = lerp(8, 15, t)
            b = lerp(18, 45, t)

            if abs((x / w) - (y / h)) < 0.0025:
                r, g, b = 108, 92, 231

            dx = (x - w / 2) / (w / 2)
            dy = (y - h / 2) / (h / 2)
            dist = (dx * dx + dy * dy) ** 0.5
            if dist < 0.55:
                glow = (0.55 - dist) * 28
                r = min(255, int(r + glow * 0.4))
                g = min(255, int(g + glow * 0.3))
                b = min(255, int(b + glow * 0.85))

            line.extend((r, g, b, 255))
        return bytes(line)

    write_png(path, w, h, row)


if __name__ == "__main__":
    gen_icon("data/ICON0.PNG")
    gen_background("data/PIC1.PNG")
    print("Icons generated successfully.")
