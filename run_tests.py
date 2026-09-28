"""
run_tests.py -> drive drvm against every .class file in extracted/ and classify the outcome, then print a summary report.

Usage:
    python3 run_tests.py
    python3 run_tests.py --drvm build/drvm --extracted extracted
    python3 run_tests.py --timeout 5 --verbose
    python3 run_tests.py --pattern 'd.class'   # only run files matching a glob
    python3 run_tests.py --report report.json  # also dump machine-readable results
"""

import argparse
import json
import re
import subprocess
import sys
import time
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Optional



FAILURE_PATTERNS = [
    ("native_missing", r"not found \(no bytecode, no native implementation\)"),
    ("unimplemented_opcode", r"Unimplemented opcode"),
    ("null_pointer", r"NullPointerException"),
    ("class_cast", r"ClassCastException"),
    ("array_index_oob", r"ArrayIndexOutOfBoundsException"),
    ("negative_array_size", r"NegativeArraySizeException"),
    ("arithmetic", r"ArithmeticException"),
    ("stack_underflow", r"Operand stack underflow"),
    ("class_load_failure", r"failed to load class"),
    ("method_not_found", r"method .* not found"),
    ("field_not_found", r"field .* not found"),
    ("bad_variant_access", r"bad_variant_access|not of an expected type"),
    ("fell_off_end", r"Fell off end of method"),
    ("invalid_class_file", r"Invalid Java class file"),
    ("segfault", r"Segmentation fault|SIGSEGV"),
    ("abort", r"SIGABRT|abort\(\)"),
]




@dataclass
class RunResult:
    class_file: str
    returncode: Optional[int]
    duration_s: float
    timed_out: bool
    category: str
    matched_line: Optional[str] = None
    stdout_tail: str = ""
    stderr_tail: str = ""

    def to_dict(self):
        return asdict(self)


def classify(returncode: Optional[int], timed_out: bool, combined_output: str) -> tuple[str, Optional[str]]:
    if timed_out:
        return "timeout", None

    for category, pattern in FAILURE_PATTERNS:
        m = re.search(pattern, combined_output, re.IGNORECASE)
        if m:
            line = next((l for l in combined_output.splitlines() if m.group(0) in l), m.group(0))
            return category, line.strip()

    if returncode == 0:
        return "clean_exit", None

    if returncode is not None and returncode < 0:
        return f"killed_by_signal_{-returncode}", None

    return "nonzero_exit_unclassified", None


def run_one(drvm: Path, class_path: Path, timeout: float, tail_lines: int) -> RunResult:
    start = time.monotonic()
    timed_out = False
    stdout = ""
    stderr = ""
    returncode: Optional[int] = None

    try:
        proc = subprocess.run(
            [str(drvm), str(class_path), "run"],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
        )
        stdout = proc.stdout
        stderr = proc.stderr
        returncode = proc.returncode
    except subprocess.TimeoutExpired as e:
        timed_out = True
        stdout = (e.stdout or "") if isinstance(e.stdout, str) else (e.stdout or b"").decode(errors="replace")
        stderr = (e.stderr or "") if isinstance(e.stderr, str) else (e.stderr or b"").decode(errors="replace")

    duration = time.monotonic() - start
    combined = stdout + "\n" + stderr
    category, matched_line = classify(returncode, timed_out, combined)

    def tail(text: str) -> str:
        lines = text.splitlines()
        return "\n".join(lines[-tail_lines:]) if lines else ""

    return RunResult(
        class_file=class_path.name,
        returncode=returncode,
        duration_s=round(duration, 3),
        timed_out=timed_out,
        category=category,
        matched_line=matched_line,
        stdout_tail=tail(stdout),
        stderr_tail=tail(stderr),
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--drvm", default="build/drvm", help="path to the drvm binary")
    parser.add_argument("--extracted", default="extracted", help="directory containing .class files")
    parser.add_argument("--pattern", default="*.class", help="glob pattern for which files to run")
    parser.add_argument("--timeout", type=float, default=10.0, help="per-file timeout in seconds")
    parser.add_argument("--tail-lines", type=int, default=6, help="lines of stdout/stderr to keep per result")
    parser.add_argument("--verbose", action="store_true", help="print full tail for every file, not just failures")
    parser.add_argument("--report", type=Path, default=None, help="write machine-readable JSON report to this path")
    args = parser.parse_args()

    drvm = Path(args.drvm)
    extracted = Path(args.extracted)

    if not drvm.exists():
        print(f"error: drvm binary not found at {drvm}", file=sys.stderr)
        return 2
    if not extracted.is_dir():
        print(f"error: extracted directory not found at {extracted}", file=sys.stderr)
        return 2

    class_files = sorted(extracted.glob(args.pattern))
    if not class_files:
        print(f"error: no files matching {args.pattern!r} in {extracted}", file=sys.stderr)
        return 2

    results: list[RunResult] = []
    print(f"Running {len(class_files)} class file(s) through {drvm} (timeout={args.timeout}s each)\n")

    for class_path in class_files:
        result = run_one(drvm, class_path, args.timeout, args.tail_lines)
        results.append(result)

        status_symbol = "OK " if result.category == "clean_exit" else "!! "
        print(f"{status_symbol} {result.class_file:<20} {result.category:<28} "
              f"({result.duration_s}s, rc={result.returncode})")

        if result.matched_line:
            print(f"      -> {result.matched_line}")

        if args.verbose or result.category not in ("clean_exit",):
            if result.stdout_tail:
                print("      [stdout tail]")
                for line in result.stdout_tail.splitlines():
                    print(f"        {line}")
            if result.stderr_tail:
                print("      [stderr tail]")
                for line in result.stderr_tail.splitlines():
                    print(f"        {line}")
        print()

    print("=-" * 35)
    print("Summary")
    print("=-" * 35)

    counts: dict[str, int] = {}
    for r in results:
        counts[r.category] = counts.get(r.category, 0) + 1

    for category, count in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"  {category:<28} {count}")

    print(f"\nTotal: {len(results)}   Clean: {counts.get('clean_exit', 0)}   "
          f"Failures: {len(results) - counts.get('clean_exit', 0)}")

    if args.report:
        args.report.write_text(json.dumps([r.to_dict() for r in results], indent=2))
        print(f"\nJSON report written to {args.report}")

    return 0 if counts.get("clean_exit", 0) == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())