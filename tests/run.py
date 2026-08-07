#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

APP_NAME = "bin/test_app"
DB_NAME = "uprint.db"

EXPECTED_DB_NAME = "expected.db"
EXPECTED_OUTPUT_NAME = "expected.output"


def verify_binary_stream(expected_file: Path, actual_bytes: bytes) -> bool:
    #  Verifies actual_bytes against expected_file line-by-line.
    byte_offset = 0

    with open(expected_file, "r", encoding="utf-8") as f:
        for line_num, line in enumerate(f, start=1):
            # 1. Strip comments and whitespace
            clean_line = line.split("#", 1)[0].strip()
            if not clean_line:
                continue

            # 2. Normalize hex string
            expected_hex = clean_line.replace(" ", "").upper()

            # Validate hex length safety
            if len(expected_hex) % 2 != 0:
                print(f"❌ Syntax Error in {expected_file.name}:{line_num}")
                print(f"   Hex string has odd length: '{clean_line}'")
                return False

            chunk_len = len(expected_hex) // 2

            # 3. Slice actual binary stream at current offset
            actual_chunk = actual_bytes[byte_offset : byte_offset + chunk_len]

            # Reformat actual chunk to match spaced uppercase format
            actual_hex = actual_chunk.hex().upper()

            # 4. Assert chunk match
            if actual_hex != expected_hex:
                # Format expected string with byte spaces for clean comparison display
                expected_formatted = " ".join(expected_hex[i:i+2] for i in range(0, len(expected_hex), 2))
                actual_formatted = " ".join(actual_hex[i:i+2] for i in range(0, len(actual_hex), 2)) if actual_hex else "<EOF>"

                print(f"❌ Mismatch in {expected_file.name} at line {line_num} (Byte offset {byte_offset}):")
                print(f"   Expected: {expected_formatted}")
                print(f"   Actual:   {actual_formatted}")
                return False

            byte_offset += chunk_len

    # 5. Verify no trailing unconsumed bytes remain
    if byte_offset < len(actual_bytes):
        extra_bytes = actual_bytes[byte_offset:].hex(' ').upper()
        print(f"❌ Stream length mismatch in {expected_file.name}:")
        print(f"   Expected stream to end at byte {byte_offset}, but {len(actual_bytes) - byte_offset} extra bytes were emitted:")
        print(f"   Extra payload: {extra_bytes}")
        return False

    return True


# Run a single test
def run_test(test_dir: Path) -> bool:
    print(f"\n>>> Running Test: {test_dir.name} <<<")
    
    # 1. Run make clean & make build
    clean_res = subprocess.run(["make", "-C", str(test_dir), "clean"], capture_output=True)
    build_res = subprocess.run(["make", "-C", str(test_dir), "all"], capture_output=True)
    
    if build_res.returncode != 0:
        print(f"❌ Compilation Failed:\n{build_res.stderr.decode()}")
        return False

    # 2. Assert Database Generated
    db_file = test_dir / DB_NAME
    expected_db = test_dir / EXPECTED_DB_NAME
    
    if not db_file.exists():
        print("❌ Error: uprint.db was not generated!")
        return False

    db_diff = subprocess.run(["diff", "-u", str(expected_db), str(db_file)], capture_output=True)
    if db_diff.returncode != 0:
        print(f"❌ Database Mismatch:\n{db_diff.stdout.decode()}")
        return False

    # 3. Execute test application
    exec_res = subprocess.run([str(test_dir / APP_NAME)], capture_output=True)
    if exec_res.returncode != 0:
        print(f"❌ Execution Failed:\n{exec_res.stderr.decode()}")
        return False

    expected_output = test_dir / EXPECTED_OUTPUT_NAME

    if not expected_output.exists():
        print(f"❌ Missing expected output file: {expected_output}")
        return False

    if not verify_binary_stream(expected_output, exec_res.stdout):
        return False

    print(f"✅ {test_dir.name} Passed!")
    return True

# Loop through all test directories and run tests
def main():
    test_root = Path(__file__).parent
    test_dirs = [d for d in test_root.iterdir() if d.is_dir() and (d / "makefile").exists()]
    test_count = len(test_dirs)
    print(f"Running {test_count} test(s)...")

    failed = []
    for test_dir in sorted(test_dirs):
        if not run_test(test_dir):
            failed.append(test_dir.name)

    print("\n" + "=" * 80)
    if failed:
        print(f"❌ Failed Tests: {', '.join(failed)}")
        sys.exit(1)
    else:
        print("🎉 ALL TESTS PASSED SUCCESSFULLY!")
        sys.exit(0)

if __name__ == "__main__":
    main()
