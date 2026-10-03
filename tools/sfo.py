#!/usr/bin/env python3
"""Minimal PARAM.SFO writer for PS3 homebrew packages."""
import argparse
import struct
import sys
import xml.etree.ElementTree as ET

def parse_xml(path):
    tree = ET.parse(path)
    root = tree.getroot()
    entries = []
    for val in root.findall("value"):
        name = val.get("name")
        typ = val.get("type", "string")
        text = (val.text or "").strip()
        if typ == "integer":
            entries.append((name, 4, struct.pack("<I", int(text))))
        else:
            data = text.encode("utf-8") + b"\x00"
            # pad to 4
            while len(data) % 4:
                data += b"\x00"
            entries.append((name, 2, data))
    return entries

def build_sfo(entries):
    # PSF header
    magic = b"\x00PSF"
    version = struct.pack("<I", 0x101)
    key_table_start = 20  # after header
    # We'll recalculate
    keys = []
    data_blobs = []
    key_blob = b""
    for name, fmt, data in entries:
        keys.append((name, fmt, len(data), len(key_blob)))
        key_blob += name.encode("ascii") + b"\x00"
        data_blobs.append(data)

    # align key table
    while len(key_blob) % 4:
        key_blob += b"\x00"

    num = len(entries)
    index_size = num * 16
    key_table_start = 20 + index_size
    data_table_start = key_table_start + len(key_blob)

    header = magic + version + struct.pack("<III", key_table_start, data_table_start, num)

    index = b""
    data_off = 0
    for i, (name, fmt, dlen, koff) in enumerate(keys):
        # key_offset, data_fmt, data_len, data_max_len, data_offset
        max_len = dlen
        index += struct.pack("<HHIII", koff, fmt, dlen, max_len, data_off)
        data_off += dlen

    body = header + index + key_blob + b"".join(data_blobs)
    return body
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-f", "--file", required=True, help="sfo.xml")
    ap.add_argument("output", nargs="?", default="PARAM.SFO")
    ap.add_argument("--title", default=None)
    ap.add_argument("--appid", default=None)
    ap.add_argument("--appver", default=None)
    ap.add_argument("--category", default=None)
    args = ap.parse_args()

    entries = parse_xml(args.file)
    # optional overrides
    override = {}
    if args.title: override["TITLE"] = args.title
    if args.appid: override["TITLE_ID"] = args.appid
    if args.appver: override["APP_VER"] = args.appver
    if args.category: override["CATEGORY"] = args.category

    if override:
        new = []
        for name, fmt, data in entries:
            if name in override and fmt == 2:
                d = override[name].encode("utf-8") + b"\x00"
                while len(d) % 4:
                    d += b"\x00"
                new.append((name, fmt, d))
            else:
                new.append((name, fmt, data))
        entries = new

    sfo = build_sfo(entries)
    with open(args.output, "wb") as f:
        f.write(sfo)
    print(f"Wrote {args.output} ({len(sfo)} bytes)")

if __name__ == "__main__":
    main()
