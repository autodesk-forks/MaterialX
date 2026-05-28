#!/usr/bin/env python3
"""
Compare legacy_coverage.txt and pytest_coverage.txt to identify any discrepancies.
"""
import sys
from pathlib import Path
from typing import Dict, Set

def align_key(key: str) -> str:
    """Normalize keys so they can be matched regardless of backslashes or spacing."""
    return key.replace("\\", "/").strip()

def load_manifest(path: Path) -> Dict[str, str]:
    if not path.exists():
        print(f"Error: Manifest not found at {path}", file=sys.stderr)
        return {}
    
    entries = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        parts = line.split("\t")
        if len(parts) == 2:
            entries[align_key(parts[0])] = parts[1]
    return entries

def main():
    contrib_dir = Path(__file__).resolve().parent
    legacy_file = contrib_dir / "legacy_coverage.txt"
    pytest_file = contrib_dir / "pytest_coverage.txt"

    print("=== MaterialX Coverage Manifest Comparison ===")
    print(f"Legacy Manifest: {legacy_file.relative_to(legacy_file.parents[1]) if legacy_file.exists() else 'Missing'}")
    print(f"Pytest Manifest: {pytest_file.relative_to(pytest_file.parents[1]) if pytest_file.exists() else 'Missing'}\n")

    legacy = load_manifest(legacy_file)
    pytest = load_manifest(pytest_file)

    if not legacy or not pytest:
        print("Error: Both manifests must exist to perform a comparison.")
        return 1

    all_keys: Set[str] = sorted(set(legacy.keys()) | set(pytest.keys()))

    matches = 0
    discrepancies = []
    only_in_legacy = []
    only_in_pytest = []

    for key in all_keys:
        leg_status = legacy.get(key)
        py_status = pytest.get(key)

        if leg_status and py_status:
            if leg_status == py_status:
                matches += 1
            else:
                discrepancies.append((key, leg_status, py_status))
        elif leg_status:
            only_in_legacy.append((key, leg_status))
        else:
            only_in_pytest.append((key, py_status))

    print(f"Comparison Summary:")
    print(f"  Total unique elements matched:      {matches} / {len(all_keys)}")
    print(f"  Status discrepancies:               {len(discrepancies)}")
    print(f"  Elements only in Legacy:            {len(only_in_legacy)}")
    print(f"  Elements only in Pytest:            {len(only_in_pytest)}")
    print("-" * 50)

    if discrepancies:
        print("\nStatus Discrepancies (Legacy != Pytest):")
        print(f"  {'Element Key':<70} | {'Legacy':<10} | {'Pytest':<10}")
        print("  " + "-" * 96)
        for key, leg, py in sorted(discrepancies):
            print(f"  {key:<70} | {leg:<10} | {py:<10}")

    if only_in_legacy:
        print("\nElements Only in Legacy (Missing in Pytest):")
        for key, status in sorted(only_in_legacy):
            print(f"  {key:<70} | ({status})")

    if only_in_pytest:
        print("\nElements Only in Pytest (Missing in Legacy):")
        for key, status in sorted(only_in_pytest):
            print(f"  {key:<70} | ({status})")

    if not discrepancies and not only_in_legacy and not only_in_pytest:
        print("\nSUCCESS: Both manifests are 100% identical in coverage and status!")
    else:
        print("\nDiscrepancies found. Review the detailed log above.")

    return 0 if (not discrepancies and not only_in_legacy and not only_in_pytest) else 1

if __name__ == "__main__":
    sys.exit(main())
