#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

APP_NAME = "bin/test_app"
DB_NAME = "uprint.db"

EXPECTED_DB_NAME = "expected.db"
EXPECTED_OUTPUT_NAME = "expected.output"



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

    expected_output_content = expected_output.read_bytes()

    # Check if execution output is same as expected output
    if exec_res.stdout != expected_output_content:
        print(f"❌ Output Mismatch:")
        print(f"❌ Output:         {exec_res.stdout.hex(' ').upper()}")
        print(f"❌ Expected Output:{expected_output_content.hex(' ').upper()}")
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
