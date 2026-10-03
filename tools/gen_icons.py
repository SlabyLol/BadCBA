#!/usr/bin/env python3
"""
Generate ICON0.PNG (320x176) and PIC1.PNG (1920x1080) for BadCBA.
Pure Python, no external dependencies required.
"""

import struct
import zlib
import os

def write_png(filename, width, height, pixels):
    """pixels: list of (r,g,b,a) tuples row-major"""
    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)

    raw = b""
    for y in range(height):
        raw += b"\x00"  # filter none
        for x in range(width):
            r, g, b, a = pixels[y * width + x]
            raw += bytes((r, g, b, a))

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", ihdr)
    data += chunk(b"IDAT", zlib.compress(raw, 9))
    data += chunk(b"IEND", b"")

    os.makedirs(os.path.dirname(filename) or ".", exist_ok=True)
    with open(filename, "wb") as f:
        f.write(data)
    print(f"Wrote {filename} ({width}x{height})")


def lerp(a, b, t):
    return int(a + (b - a) * t)


def gen_icon(path):
    w, h = 320, 176
    pixels = []
    for y in range(h):
        for x in range(w):
            # Dark purple gradient background
            t = y / h
            r = lerp(10, 40, t)
            g = lerp(10, 20, t)
            b = lerp(25, 60, t)

            # Accent bar at top
            if y < 6:
                r, g, b = 108, 92, 231

            # Simple "B" letter shape in the center
            cx, cy = w // 2, h // 2
            # Vertical bar of B
            if 90 < x < 110 and 40 < y < 136:
                r, g, b = 0, 206, 201
            # Top loop
            if 110 <= x < 160 and 40 < y < 80:
                if (x - 110) ** 2 / 50**2 + (y - 60) ** 2 / 25**2 < 1:
                    r, g, b = 0, 206, 201
            # Bottom loop
            if 110 <= x < 170 and 90 < y < 136:
                if (x - 110) ** 2 / 55**2 + (y - 113) ** 2 / 28**2 < 1:
                    r, g, b = 0, 206, 201

            pixels.append((r, g, b, 255))
    write_png(path, w, h, pixels)


def gen_background(path):
    w, h = 1920, 1080
    pixels = []
    for y in range(h):
        for x in range(w):
            # Smooth dark gradient
            t = y / h
            r = lerp(8, 25, t)
            g = lerp(8, 15, t)
            b = lerp(18, 45, t)

            # Subtle diagonal accent line
            if abs((x / w) - (y / h)) < 0.003:
                r, g, b = 108, 92, 231

            # Soft glow in center
            dx = (x - w/2) / (w/2)
            dy = (y - h/2) / (h/2)
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 0.6:
                glow = (0.6 - dist) * 30
                r = min(255, int(r + glow * 0.4))
                g = min(255, int(g + glow * 0.3))
                b = min(255, int(b + glow * 0.8))

            pixels.append((r, g, b, 255))
    write_png(path, w, h, pixels)


if __name__ == "__main__":
    gen_icon("data/ICON0.PNG")
    gen_background("data/PIC1.PNG")
    print("Icons generated successfully.")
