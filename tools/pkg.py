#!/usr/bin/env python3
"""
Minimal PS3 PKG builder (homebrew / debug style).
Creates a simple package from a directory containing PARAM.SFO and USRDIR/.
"""
import argparse
import os
import struct
import hashlib
import sys

def collect_files(root):
    files = []
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            full = os.path.join(dirpath, name)
            rel = os.path.relpath(full, root).replace("\\", "/")
            files.append((rel, full))
    files.sort(key=lambda x: x[0])
    return files

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--contentid", default="UP0001-BCBA00001_00-0000000000000000")
    ap.add_argument("pkgdir")
    ap.add_argument("output")
    args = ap.parse_args()

    pkgdir = args.pkgdir.rstrip("/")
    if not os.path.isdir(pkgdir):
        print(f"Not a directory: {pkgdir}", file=sys.stderr)
        sys.exit(1)

    files = collect_files(pkgdir)
    if not files:
        print("No files in package dir", file=sys.stderr)
        sys.exit(1)

    # Build file table + data
    # This is a simplified PKG format sufficient for many CFW installers.
    content_id = args.contentid.encode("ascii")
    content_id = content_id[:0x30].ljust(0x30, b"\x00")

    file_entries = []
    data_blob = b""
    for rel, full in files:
        with open(full, "rb") as f:
            raw = f.read()
        name_b = rel.encode("utf-8") + b"\x00"
        # align name to 4
        while len(name_b) % 4:
            name_b += b"\x00"
        offset = len(data_blob)
        file_entries.append((name_b, offset, len(raw)))
        data_blob += raw
        # align data to 16
        pad = (16 - (len(data_blob) % 16)) % 16
        data_blob += b"\x00" * pad

    # Header (simplified)
    # magic, type, pkg_info_offset, ...
    magic = b"\x7fPKG"
    pkg_type = struct.pack(">I", 0x00000001)  # retail-ish
    pkg_info_off = struct.pack(">I", 0xC0)
    unk = struct.pack(">I", 0)
    head_size = struct.pack(">I", 0xC0)
    item_count = struct.pack(">I", len(file_entries))
    total_size_placeholder = b"\x00" * 8
    data_off_placeholder = b"\x00" * 8
    data_size_placeholder = b"\x00" * 8

    header = (
        magic + pkg_type + pkg_info_off + unk +
        head_size + item_count +
        total_size_placeholder + data_off_placeholder + data_size_placeholder +
        content_id +
        b"\x00" * (0xC0 - 0x40 - 0x30)  # pad to 0xC0 roughly
    )
    # Ensure header length 0xC0
    header = header[:0xC0].ljust(0xC0, b"\x00")

    # File metadata block
    meta = b""
    name_table = b""
    name_off = 0
    for name_b, offset, size in file_entries:
        # flags, name_offset, name_len not used simply – store offset/size
        meta += struct.pack(">IIII", 0, name_off, offset, size)
        name_table += name_b
        name_off += len(name_b)

    while len(name_table) % 16:
        name_table += b"\x00"

    body = header + meta + name_table + data_blob

    # Fill size fields at known offsets (best-effort)
    total = len(body)
    body = bytearray(body)
    struct.pack_into(">Q", body, 0x18, total)
    struct.pack_into(">Q", body, 0x20, 0xC0 + len(meta) + len(name_table))
    struct.pack_into(">Q", body, 0x28, len(data_blob))

    with open(args.output, "wb") as f:
        f.write(body)

    print(f"Wrote {args.output} ({total} bytes, {len(file_entries)} files)")
    print("Note: simplified PKG – install with CFW Package Manager / multiMAN.")

if __name__ == "__main__":
    main()
