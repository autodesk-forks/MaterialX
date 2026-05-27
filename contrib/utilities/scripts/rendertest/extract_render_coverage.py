#!/usr/bin/env python3
"""
Extract per-element render coverage from MaterialX render logs.

Produces a sorted text manifest for diffing legacy renderDocuments.py runs
against pytest (or other runners):

    relative/path/file.mtlx:element_name<TAB>STATUS

STATUS values (legacy / renderDocuments log):
    PASS    - Saved rendered image
    FAIL    - Shader generation or render failed
    SKIP    - No surface shader in material (logged, not rendered)

Optional MaterialXTest C++ log format (genglsl_render_log.txt):
    RENDER  - Element appeared in a validation run (no pass/fail in log)

Usage:
    python extract_render_coverage.py render_log.txt -o legacy_coverage.txt
    python extract_render_coverage.py render_log.txt --compare pytest_coverage.txt
"""
from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


class Status(str, Enum):
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    RENDER = "RENDER"  # MaterialXTest log: element exercised, outcome not in log
    NO_ELEMENTS = "NO_ELEMENTS"  # File processed but nothing renderable


# renderDocuments.py / mxrenderer.performRender
_RE_RENDER_FILE = re.compile(
    r"^Render file:(.+?)\.\s*SearchPath:",
    re.IGNORECASE,
)
_RE_FINISHED_FILE = re.compile(
    r'^--- Finished rendering file "(.+?)"',
    re.IGNORECASE,
)
_RE_RENDER_NODE = re.compile(
    r"^------- Render Node:\s*(.+?)\s*--------",
    re.IGNORECASE,
)
_RE_SAVED_IMAGE = re.compile(
    r"^- Saved rendered image to:",
    re.IGNORECASE,
)
_RE_FAIL_SHADER = re.compile(
    r'^- Failed to generate shader for node:\s*"(.+?)"\.',
    re.IGNORECASE,
)
_RE_FAIL_RENDER = re.compile(
    r"^- Failed to render, Errors:",
    re.IGNORECASE,
)
_RE_WARN_NO_SHADER = re.compile(
    r"No surface shader found in material:\s*\"(.+?)\"",
    re.IGNORECASE,
)

# MaterialXTest C++ (genglsl_render_log.txt)
_RE_MTLX_FILENAME = re.compile(
    r"^MTLX Filename:\s*(.+)$",
    re.IGNORECASE,
)
_RE_RUN_ELEMENT = re.compile(
    r"^-+\s*Run GLSL validation with element:\s*(.+?)\s*-+$",
    re.IGNORECASE,
)

_PATH_ANCHORS = (
    "resources/Materials/",
    "contrib/adsk/resources/Materials/",
)


def normalize_mtlx_path(raw_path: str) -> str:
    """Normalize to forward slashes and a stable repo-relative Materials path."""
    path = raw_path.strip().strip('"').replace("\\", "/")
    lower = path.lower()
    for anchor in _PATH_ANCHORS:
        idx = lower.find(anchor.lower())
        if idx >= 0:
            return path[idx:]
    # Fallback: path relative to last Materials segment
    parts = path.split("/")
    for i, part in enumerate(parts):
        if part.lower() == "materials" and i + 1 < len(parts):
            return "/".join(parts[i - 1 :] if i > 0 and parts[i - 1] == "resources" else parts[i:])
    return path


def make_key(mtlx_path: str, element: str) -> str:
    return f"{normalize_mtlx_path(mtlx_path)}:{element}"


@dataclass
class _ParseState:
    current_file: Optional[str] = None
    current_element: Optional[str] = None
    element_open: bool = False


def _close_element(
    entries: Dict[str, Status],
    state: _ParseState,
    default_if_open: Status = Status.FAIL,
) -> None:
    if state.current_file and state.current_element and state.element_open:
        key = make_key(state.current_file, state.current_element)
        if key not in entries:
            entries[key] = default_if_open
    state.current_element = None
    state.element_open = False


def parse_legacy_log(lines: Iterable[str]) -> Dict[str, Status]:
    """Parse render_log.txt from renderDocuments.py."""
    entries: Dict[str, Status] = {}
    state = _ParseState()
    files_with_elements: set[str] = set()

    for line in lines:
        line = line.rstrip("\n\r")

        m = _RE_RENDER_FILE.match(line)
        if m:
            _close_element(entries, state)
            state.current_file = m.group(1)
            state.current_element = None
            state.element_open = False
            continue

        m = _RE_FINISHED_FILE.match(line)
        if m:
            _close_element(entries, state)
            state.current_file = m.group(1)
            continue

        m = _RE_RENDER_NODE.match(line)
        if m:
            _close_element(entries, state)
            state.current_element = m.group(1)
            state.element_open = True
            if state.current_file:
                files_with_elements.add(normalize_mtlx_path(state.current_file))
            continue

        if _RE_SAVED_IMAGE.match(line):
            if state.current_file and state.current_element:
                key = make_key(state.current_file, state.current_element)
                entries[key] = Status.PASS
                state.element_open = False
            continue

        m = _RE_FAIL_SHADER.match(line)
        if m:
            elem = m.group(1)
            if state.current_file:
                key = make_key(state.current_file, elem)
                entries[key] = Status.FAIL
            state.current_element = elem
            state.element_open = False
            continue

        if _RE_FAIL_RENDER.match(line):
            if state.current_file and state.current_element:
                key = make_key(state.current_file, state.current_element)
                entries[key] = Status.FAIL
                state.element_open = False
            continue

        m = _RE_WARN_NO_SHADER.search(line)
        if m:
            elem = m.group(1)
            if state.current_file:
                key = make_key(state.current_file, elem)
                entries[key] = Status.SKIP
            continue

    _close_element(entries, state)

    return entries


def parse_materialxtest_log(lines: Iterable[str]) -> Dict[str, Status]:
    """Parse genglsl_render_log.txt from MaterialXTest C++."""
    entries: Dict[str, Status] = {}
    current_file: Optional[str] = None

    for line in lines:
        line = line.rstrip("\n\r")

        m = _RE_MTLX_FILENAME.match(line)
        if m:
            current_file = m.group(1)
            continue

        m = _RE_RUN_ELEMENT.match(line)
        if m and current_file:
            element = m.group(1)
            key = make_key(current_file, element)
            entries[key] = Status.RENDER

    return entries


def detect_log_format(lines: List[str]) -> str:
    for line in lines[:200]:
        if _RE_RENDER_FILE.match(line):
            return "legacy"
        if _RE_MTLX_FILENAME.match(line):
            return "materialxtest"
    return "legacy"


def parse_log(path: Path, log_format: str) -> Dict[str, Status]:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    fmt = log_format
    if fmt == "auto":
        fmt = detect_log_format(lines)
    if fmt == "legacy":
        return parse_legacy_log(lines)
    if fmt == "materialxtest":
        return parse_materialxtest_log(lines)
    raise ValueError(f"Unknown log format: {fmt}")


def write_manifest(entries: Dict[str, Status], output: Path) -> None:
    lines = [f"{key}\t{status.value}" for key, status in sorted(entries.items())]
    output.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")


def compare_manifests(
    left: Dict[str, Status],
    right: Dict[str, Status],
    left_name: str,
    right_name: str,
) -> int:
    """Print diff summary. Return exit code (0 if no mismatches)."""
    only_left = sorted(set(left) - set(right))
    only_right = sorted(set(right) - set(left))
    common = sorted(set(left) & set(right))

    status_mismatch: List[Tuple[str, Status, Status]] = []
    for key in common:
        if left[key] != right[key]:
            status_mismatch.append((key, left[key], right[key]))

    print(f"=== Coverage comparison: {left_name} vs {right_name} ===")
    print(f"  {left_name}: {len(left)} entries")
    print(f"  {right_name}: {len(right)} entries")
    print(f"  Only in {left_name}: {len(only_left)}")
    print(f"  Only in {right_name}: {len(only_right)}")
    print(f"  Status mismatches: {len(status_mismatch)}")

    if only_left:
        print(f"\n--- Only in {left_name} (first 20) ---")
        for key in only_left[:20]:
            print(f"  {key}\t{left[key].value}")
        if len(only_left) > 20:
            print(f"  ... and {len(only_left) - 20} more")

    if only_right:
        print(f"\n--- Only in {right_name} (first 20) ---")
        for key in only_right[:20]:
            print(f"  {key}\t{right[key].value}")
        if len(only_right) > 20:
            print(f"  ... and {len(only_right) - 20} more")

    if status_mismatch:
        print("\n--- Status mismatches (first 20) ---")
        for key, lstat, rstat in status_mismatch[:20]:
            print(f"  {key}\t{left_name}={lstat.value}\t{right_name}={rstat.value}")
        if len(status_mismatch) > 20:
            print(f"  ... and {len(status_mismatch) - 20} more")

    if only_left or only_right or status_mismatch:
        return 1
    print("\nManifests match.")
    return 0


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Extract render coverage manifest from MaterialX render logs.",
    )
    parser.add_argument(
        "log_file",
        type=Path,
        nargs="?",
        help="Input log (render_log.txt or genglsl_render_log.txt)",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Write sorted manifest (file.mtlx:element<TAB>STATUS)",
    )
    parser.add_argument(
        "--format",
        choices=("auto", "legacy", "materialxtest"),
        default="auto",
        help="Log format (default: auto-detect)",
    )
    parser.add_argument(
        "--compare",
        type=Path,
        metavar="OTHER_MANIFEST",
        help="Compare extracted or existing manifest against another file",
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        help="Use an existing manifest instead of parsing log_file",
    )
    args = parser.parse_args(argv)

    if args.manifest:
        entries = _read_manifest(args.manifest)
        left_name = str(args.manifest)
    elif args.log_file:
        if not args.log_file.is_file():
            print(f"Error: log file not found: {args.log_file}", file=sys.stderr)
            return 2
        entries = parse_log(args.log_file, args.format)
        left_name = str(args.log_file)
    else:
        parser.error("Provide log_file and/or --manifest")

    if args.output:
        write_manifest(entries, args.output)
        print(f"Wrote {len(entries)} entries to {args.output}")

    if args.compare:
        if not args.compare.is_file():
            print(f"Error: compare file not found: {args.compare}", file=sys.stderr)
            return 2
        right = _read_manifest(args.compare)
        return compare_manifests(entries, right, left_name, str(args.compare))

    if not args.output and not args.compare:
        for key, status in sorted(entries.items()):
            print(f"{key}\t{status.value}")

    return 0


def _read_manifest(path: Path) -> Dict[str, Status]:
    entries: Dict[str, Status] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "\t" not in line:
            print(f"Warning: skipping malformed line: {line}", file=sys.stderr)
            continue
        key, status_str = line.split("\t", 1)
        try:
            entries[key] = Status(status_str)
        except ValueError:
            print(f"Warning: unknown status {status_str!r} for {key}", file=sys.stderr)
    return entries


if __name__ == "__main__":
    sys.exit(main())
