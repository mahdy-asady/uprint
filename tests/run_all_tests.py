#!/usr/bin/env python3
import os
import io
import sys
import subprocess
from pathlib import Path
import re

APP_NAME = "bin/test_app"
DB_NAME = "uprint.db"

EXPECTED_DB_NAME = "expected.db"
EXPECTED_OUTPUT_NAME = "expected.output"


# Color definitions
RED_LIGHT = '\033[91m'
GREEN_LIGHT = '\033[92m'
YELLOW_LIGHT = '\033[93m'
BLUE_LIGHT = '\033[94m'
PURPLE_LIGHT = '\033[95m'
CYAN_LIGHT = '\033[96m'
WHITE_LIGHT = '\033[97m'

BLACK = '\033[30m'
RED = '\033[31m'
GREEN = '\033[32m'
YELLOW = '\033[33m'
BLUE = '\033[34m'
PURPLE = '\033[35m'
CYAN = '\033[36m'
WHITE = '\033[37m'

RESET = '\033[0m' # Resets terminal text back to default
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

ERROR = RED_LIGHT + BOLD
WARNING = YELLOW_LIGHT
INFO = BLUE_LIGHT
SUCCESS = GREEN_LIGHT + BOLD


# Print str in middle of console
SCREEN_WIDTH = os.get_terminal_size().columns
ANSI_PATTERN = re.compile(r'(?:\x1B|\033)\[[0-9;]*m')
def center(text: str = "", fill: str = "-") -> str:
    output = ""
    clean_text = ANSI_PATTERN.sub('', text)
    ln = len(clean_text)
    fill_width = SCREEN_WIDTH - ln
    if fill_width > 0:
        output += fill * (fill_width // 2)
        output += text
        output += fill * ((fill_width // 2) + (fill_width % 2))
    else:
        output += text
    return output

#  Verifies actual_bytes against expected_file line-by-line.
def verify_binary_stream(expected_file: Path, actual_bytes: bytes) -> tuple[bool, str]:
    buffer = io.StringIO()
    def log(msg: str):
            buffer.write(msg + "\n")

    byte_offset = 0
    failed = False

    with open(expected_file, "r", encoding="utf-8") as f:
        test_number = 0
        test_name = expected_file.parent.name
        for line_num, line in enumerate(f, start=1):
            # 1. Strip comments and whitespace
            clean_line = line.split("#", 1)[0].strip()
            if not clean_line:
                continue

            print(f">>> {test_name} - [{test_number}]", end="", flush=True)

            # 2. Normalize hex string
            expected_hex = clean_line.replace(" ", "").upper()

            # Validate hex length safety
            if len(expected_hex) % 2 != 0:
                log(f"❌ Syntax Error in {expected_file.name}:{line_num}")
                log(f"   Hex string has odd length: '{clean_line}'")
                print(f"{ERROR} FAILED{RESET}")
                failed = True
                continue

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

                log(f"❌ Mismatch in {expected_file.name} at line {line_num} (Byte offset {byte_offset}):")
                log(f"   Expected: {expected_formatted}")
                log(f"   Actual:   {actual_formatted}")
                print(f"{ERROR} FAILED{RESET}")
                failed = True
                continue

            byte_offset += chunk_len

            test_number += 1
            print(f"{SUCCESS} PASSED{RESET}")

    print(f">>> {test_name} - TERMINATION", end="", flush=True)
    # 5. Verify no trailing unconsumed bytes remain
    if byte_offset < len(actual_bytes):
        extra_bytes = actual_bytes[byte_offset:].hex(' ').upper()
        log(f"❌ Stream length mismatch in {expected_file.name}:")
        log(f"   Expected stream to end at byte {byte_offset}, but {len(actual_bytes) - byte_offset} extra bytes were emitted:")
        log(f"   Extra payload: {extra_bytes}")
        print(f"{ERROR} FAILED{RESET}")
        failed = True
    print(f"{SUCCESS} PASSED{RESET}")

    return not failed, buffer.getvalue()


# Run a single test
def run_test(test_dir: Path) -> tuple[bool, str]:
    buffer = io.StringIO()
    def log(msg: str):
        buffer.write(msg + "\n")

    # 1. Clean & Compile
    log(f"{WARNING}[Step 1: Build]{RESET}")
    
    # 1. Run make clean & make build
    print(f">>> {test_dir.name} - BUILD", end="", flush=True)
    clean_res = subprocess.run(["make", "-C", str(test_dir), "clean"], capture_output=True)
    build_res = subprocess.run(["make", "-C", str(test_dir), "all"], capture_output=True)
    
    log(center(f" {BOLD}stdout{RESET} ", "-"))
    log(build_res.stdout.decode())
    log(center(f" {BOLD}stderr{RESET} ", "-"))
    log(build_res.stderr.decode())

    if build_res.returncode != 0:
        print(f"{ERROR} FAILED{RESET}")
        log("\n❌ Compilation failed!")
        return False, buffer.getvalue()

    print(f"{SUCCESS} PASSED{RESET}")


    # 2. Assert Database Generated
    print(f">>> {test_dir.name} - CHECK DB", end="", flush=True)
    log(center())
    log(f"{WARNING}[Step 2: Check Database]{RESET}")
    db_file = test_dir / DB_NAME
    expected_db = test_dir / EXPECTED_DB_NAME
    
    if not db_file.exists():
        print(f"{ERROR} FAILED{RESET}")
        log(f"❌ Error: {DB_NAME} was not generated!")
        return False, buffer.getvalue()

    db_diff = subprocess.run(["diff", "-u", str(expected_db), str(db_file)], capture_output=True)
    if db_diff.returncode != 0:
        print(f"{ERROR} FAILED{RESET}")
        log(f"❌ Database Mismatch:\n{db_diff.stdout.decode()}")
        return False, buffer.getvalue()

    log(f"✅ {GREEN_LIGHT}Database OK{RESET}")
    print(f"{SUCCESS} PASSED{RESET}")

    # 3. Execute test application
    print(f">>> {test_dir.name} - EXECUTE", end="", flush=True)
    log(center())
    log(f"{WARNING}[Step 3: Execution]{RESET}")

    exec_path = test_dir / APP_NAME
    if not exec_path.exists():
        print(f"{ERROR} FAILED{RESET}")
        log("❌ Executable binary not found!")
        return False, buffer.getvalue()

    exec_res = subprocess.run([str(exec_path)], capture_output=True)

    log(f"App Exit Code: {exec_res.returncode}")

    if exec_res.returncode != 0:
        print(f"{ERROR} FAILED{RESET}")
        log(f"❌ Execution Failed:\n{exec_res.stderr.decode()}")
        return False, buffer.getvalue()

    print(f"{SUCCESS} PASSED{RESET}")

    # 4. Verify Output
    log(center())
    log(f"{WARNING}[Step 4: Verify Output]{RESET}")

    expected_output = test_dir / EXPECTED_OUTPUT_NAME

    if not expected_output.exists():
        print(f"{ERROR} FAILED{RESET}")
        log(f"❌ Missing expected output file: {expected_output}")
        return False, buffer.getvalue()

    verify_ret, verify_log = verify_binary_stream(expected_output, exec_res.stdout)
    if not verify_ret:
        print(f"{ERROR} FAILED{RESET}")
        log(f"❌ Expected output mismatch:\n{verify_log}")
        return False, buffer.getvalue()

    log(f"✅ {GREEN_LIGHT}Output OK{RESET}")

    # 5. Success
    return True, buffer.getvalue()

# Loop through all test directories and run tests
def main():
    test_root = Path(__file__).parent
    test_dirs = [d for d in test_root.iterdir() if d.is_dir() and (d / "makefile").exists()]
    test_count = len(test_dirs)
    print(f"Running {test_count} test(s)...")

    results = []
    for test_dir in sorted(test_dirs):
        ret, out = run_test(test_dir)
        results.append((test_dir.name, ret, out))

    print("\n" * 3)
    count_success = sum(1 for _, ret, _ in results if ret)
    count_failure = sum(1 for _, ret, _ in results if not ret)

    if count_failure > 0:
        for test_name, _, log_content in results:
            print(center(fill=f"{WHITE_LIGHT}={RESET}"))
            print(center(f"{BOLD}{test_name} {PURPLE}OUTPUT DETAILS:{RESET}", " "))
            print(center(fill=f"{WHITE_LIGHT}={RESET}"))
            print(log_content)

    dash_color = RED if count_failure > 0 else GREEN
    
    print(center(f"  {ERROR}{count_failure} failed{RESET}, {SUCCESS}{count_success} passed  {RESET}", f"{dash_color}="))
    sys.exit(count_failure != 0)

if __name__ == "__main__":
    main()
