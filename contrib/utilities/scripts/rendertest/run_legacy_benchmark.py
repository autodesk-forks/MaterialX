#!/usr/bin/env python3
"""
Benchmark wrapper for the legacy MaterialX renderDocuments.py script.

This script:
1. Discovers and runs renderDocuments.py sequentially on the complete set of
   standard and Autodesk materials (or on an explicitly specified path).
2. Measures high-precision wall-clock execution time for the rendering run(s).
3. Parses the generated render logs to extract a consolidated sorted coverage manifest.
4. Reports total execution time, element-level success counts, and saves the manifest.

Usage:
    # Run benchmark on the complete test suite (TestSuite + Examples + ADSK)
    python contrib/utilities/scripts/rendertest/run_legacy_benchmark.py

    # Run benchmark on a specific file or folder
    python contrib/utilities/scripts/rendertest/run_legacy_benchmark.py contrib/adsk/resources/Materials/Examples/Fusion/metal.mtlx
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
import time
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Dict, Iterable, List, Optional


class Status(str, Enum):
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"


# renderDocuments.py / mxrenderer.performRender log regexes
_RE_RENDER_FILE = re.compile(r"^Render file:(.+?)\.\s*SearchPath:", re.IGNORECASE)
_RE_FINISHED_FILE = re.compile(r'^--- Finished rendering file "(.+?)"', re.IGNORECASE)
_RE_RENDER_NODE = re.compile(r"^------- Render Node:\s*(.+?)\s*--------", re.IGNORECASE)
_RE_SAVED_IMAGE = re.compile(r"^- Saved rendered image to:", re.IGNORECASE)
_RE_FAIL_SHADER = re.compile(r'^- Failed to generate shader for node:\s*"(.+?)"\.', re.IGNORECASE)
_RE_FAIL_RENDER = re.compile(r"^- Failed to render, Errors:", re.IGNORECASE)
_RE_WARN_NO_SHADER = re.compile(r"No surface shader found in material:\s*\"(.+?)\"", re.IGNORECASE)

_PATH_ANCHORS = (
    "contrib/adsk/resources/Materials/",
    "resources/Materials/",
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


def write_manifest(entries: Dict[str, Status], output: Path) -> None:
    lines = [f"{key}\t{status.value}" for key, status in sorted(entries.items())]
    output.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run, time, and analyze legacy renderDocuments.py execution.",
    )
    parser.add_argument(
        "input_path",
        type=str,
        nargs="?",
        default=None,
        help="Path to mtlx file or folder to render. If not specified, runs standard & ADSK suites.",
    )
    parser.add_argument(
        "--outputPath",
        type=str,
        default="contrib/renders",
        help="Directory to save rendered images (default: contrib/renders)",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent.parent.parent.parent
    
    # Define directories to run
    if args.input_path:
        target_paths = [Path(args.input_path)]
    else:
        # Default suite runs standard materials AND ADSK materials to match pytest's entire scope
        target_paths = [
            Path("resources/Materials/TestSuite"),
            Path("resources/Materials/Examples"),
            Path("contrib/adsk/resources/Materials"),
        ]

    # Create directories
    output_path = repo_root / args.outputPath
    output_path.mkdir(parents=True, exist_ok=True)

    render_docs_script = Path(__file__).resolve().parent / "renderDocuments.py"
    geometry_path = repo_root / "resources" / "Geometry" / "sphere.obj"
    radiance_path = repo_root / "resources" / "Lights" / "san_giuseppe_bridge.hdr"
    irradiance_path = repo_root / "resources" / "Lights" / "irradiance" / "san_giuseppe_bridge.hdr"

    # We want render_log.txt to land in contrib/ for neatness
    contrib_dir = repo_root / "contrib"
    contrib_dir.mkdir(parents=True, exist_ok=True)
    log_file = contrib_dir / "render_log.txt"

    accumulated_entries: Dict[str, Status] = {}
    total_elapsed_time = 0.0

    print("=== MaterialX Legacy Benchmark Wrapper ===")
    
    active_paths = [repo_root / p for p in target_paths if (repo_root / p).exists()]
    if not active_paths:
        print("Error: No valid target material directories found.", file=sys.stderr)
        return 1

    for idx, path in enumerate(active_paths, start=1):
        path_str = str(path)
        print(f"\n[{idx}/{len(active_paths)}] Rendering path: {path.relative_to(repo_root)}")
        
        # Position input_path first in arguments to avoid argparse consuming it as part of --library/--path lists
        cmd = [
            sys.executable,
            str(render_docs_script),
            path_str,
            "--outputPath",
            str(output_path),
            "--geometryPath",
            str(geometry_path),
            "--radiancePath",
            str(radiance_path),
            "--irradiancePath",
            str(irradiance_path),
            "--size",
            "512",
        ]

        # Automatically load adsklib for Autodesk materials if relevant
        if "adsk" in path_str.lower():
            adsk_lib_path = repo_root / "contrib" / "adsk" / "libraries"
            if adsk_lib_path.exists():
                cmd.extend([
                    "--path",
                    str(adsk_lib_path),
                    "--library",
                    "adsklib"
                ])

        # Clean the previous log file so we only parse this run's results
        if log_file.exists():
            log_file.unlink()

        start_time = time.perf_counter()
        process = subprocess.run(cmd, cwd=str(contrib_dir))
        elapsed_time = time.perf_counter() - start_time
        total_elapsed_time += elapsed_time

        if process.returncode != 0:
            print(f"Warning: renderDocuments.py failed on {path.name} with return code {process.returncode}", file=sys.stderr)
            continue

        if log_file.exists():
            run_entries = parse_legacy_log(log_file.read_text(encoding="utf-8", errors="replace").splitlines())
            accumulated_entries.update(run_entries)
            print(f"  Completed in {elapsed_time:.3f} seconds ({len(run_entries)} elements found)")
        else:
            print(f"  Warning: No render log generated at {log_file}")

    print(f"\nLegacy render benchmark runs complete!")
    print(f"----------------------------------------")
    print(f"Total Execution Time: {total_elapsed_time:.3f} seconds")
    print(f"----------------------------------------")

    # Count passes/fails/skips
    passes = sum(1 for status in accumulated_entries.values() if status == Status.PASS)
    fails = sum(1 for status in accumulated_entries.values() if status == Status.FAIL)
    skips = sum(1 for status in accumulated_entries.values() if status == Status.SKIP)

    print(f"Consolidated Coverage Summary:")
    print(f"  Total Elements Discovered/Attempted: {len(accumulated_entries)}")
    print(f"  Passed (Rendered Successfully):    {passes}")
    print(f"  Failed:                            {fails}")
    print(f"  Skipped (No Surface Shader):       {skips}")
    print(f"----------------------------------------")

    # Write consolidated legacy_coverage.txt manifest inside contrib/
    manifest_file = contrib_dir / "legacy_coverage.txt"
    write_manifest(accumulated_entries, manifest_file)
    print(f"Consolidated coverage manifest written to: {manifest_file.relative_to(repo_root)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
