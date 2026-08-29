#!/usr/bin/env python3
import sys
import struct
import argparse
import re
import csv
from pathlib import Path


# Specifier character + byte size -> struct format character
SPEC_SIZE_MAP = {
    # Integers (signed)
    ('d', 1): 'b', ('d', 2): 'h', ('d', 4): 'i', ('d', 8): 'q',
    ('i', 1): 'b', ('i', 2): 'h', ('i', 4): 'i', ('i', 8): 'q',
    # Integers (unsigned) / Characters / Hex / Octal
    ('u', 1): 'B', ('u', 2): 'H', ('u', 4): 'I', ('u', 8): 'Q',
    ('x', 1): 'B', ('x', 2): 'H', ('x', 4): 'I', ('x', 8): 'Q',
    ('X', 1): 'B', ('X', 2): 'H', ('X', 4): 'I', ('X', 8): 'Q',
    ('o', 1): 'B', ('o', 2): 'H', ('o', 4): 'I', ('o', 8): 'Q',
    ('c', 1): 'B', ('c', 2): 'H', ('c', 4): 'I', ('c', 8): 'Q',
    # Floating point
                                  ('f', 4): 'f', ('f', 8): 'd',
                                  ('F', 4): 'f', ('F', 8): 'd',
                                  ('g', 4): 'f', ('g', 8): 'd',
                                  ('G', 4): 'f', ('G', 8): 'd',
                                  ('e', 4): 'f', ('e', 8): 'd',
                                  ('E', 4): 'f', ('E', 8): 'd',
}

def extract_printf_specs(fmt: str) -> list[str]:
    """Extracts printf conversion specifiers to determine signedness/floating point."""
    pattern = r'%[-+ #0]*\d*(?:\.\d+)?[hlLzjt]*([diouxXeEfFgGaAcsp%])'
    specs = []
    for match in re.finditer(pattern, fmt):
        spec = match.group(1)
        if spec != '%':  # Ignore escaped %%
            specs.append(spec)
    return specs

def load_database(db_path: Path) -> dict[int, tuple[str, struct.Struct, int]]:
    if not db_path.exists():
        sys.stderr.write(f"Error: Database file not found: {db_path}\n")
        sys.exit(1)

    records = {}
    with open(db_path, "r", newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        
        # Skip Line 1 (reserved metadata header)
        _header = next(reader, None)
        
        for record_id, row in enumerate(reader, start=1):
            if not row:
                continue

            # 1. Unescape format string and handle %p compatibility
            fmt_str = row[0].encode('utf-8').decode('unicode_escape')

            # 2. Extract argument sizes
            sizes = [int(x.strip()) for x in row[1:] if x.strip()]
            payload_size = sum(sizes)

            # 3. Pre-extract specs and build struct pattern ONCE
            specs = extract_printf_specs(fmt_str)
            pattern_parts = ["<"]
            
            for i, size in enumerate(sizes):
                spec_char = specs[i] if i < len(specs) else 'd'
                # Lookup struct char, fallback to signed integer if unknown
                char = SPEC_SIZE_MAP.get((spec_char, size), 'i' if size == 4 else 'b')
                pattern_parts.append(char)
            
            unpack_pattern = "".join(pattern_parts)
            
            # 4. Pre-compile the struct object
            precompiled_struct = struct.Struct(unpack_pattern)
            
            # Cache ready-to-use tuple: (fmt_str, compiled_struct, total_bytes)
            records[record_id] = (fmt_str, precompiled_struct, payload_size)

    return records


def decode_stream(db: dict[int, tuple[str, struct.Struct, int]], endian_prefix: str):
    stdin_raw = sys.stdin.buffer

    while True:
        # 1. Read Record ID (4 bytes)
        id_bytes = stdin_raw.read(4)
        if not id_bytes:
            break  # End of Stream

        if len(id_bytes) < 4:
            sys.stderr.write(f"Error: Incomplete record ID received ({len(id_bytes)}/4 bytes)\n")
            break

        record_id = struct.unpack(f"{endian_prefix}I", id_bytes)[0]

        entry = db.get(record_id)
        if not entry:
            sys.stderr.write(f"Error: Unknown record ID: {record_id} (0x{record_id:08X})\n")
            break

        fmt_str, unpacker, payload_size = entry
        
        # 2. Read exactly the pre-computed payload size
        payload = stdin_raw.read(payload_size)
        if len(payload) < payload_size:
            sys.stderr.write(f"Error: Truncated stream for ID {record_id}\n")
            f"Expected {payload_size} bytes, got {len(payload)} bytes.\n"
            break

        # 3. Direct unpack and string formatting
        values = unpacker.unpack(payload)
        sys.stdout.write(fmt_str % values)
        sys.stdout.flush()


def main():
    parser = argparse.ArgumentParser(description="uprint Binary Stream Decoder")
    parser.add_argument("database", help="Path to uprint.db file")

    args = parser.parse_args()
    db_path = Path(args.database)

    db = load_database(db_path)
    decode_stream(db, "<")


if __name__ == "__main__":
    main()
