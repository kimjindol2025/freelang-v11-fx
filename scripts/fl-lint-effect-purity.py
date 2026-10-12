#!/usr/bin/env python3
"""Phase1 ⑤ — @effect pure vs net/db builtin calls (FX-BUILTIN-EFFECTS.airc).

Prints fl-lint-style warning lines to stdout:
  줄 N [effect-purity] ...
Exit 0 always (warn-only).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path


def load_net_db(airc: Path) -> tuple[set[str], set[str]]:
    text = airc.read_text(encoding="utf-8")
    blocks = re.split(r"\n@builtin\b", text)[1:]
    net: set[str] = set()
    db: set[str] = set()
    for b in blocks:
        names_m = re.search(r"fl_names:\s*\[([^\]]+)\]", b)
        eff_m = re.search(r"@effect:\s*(\w+)", b)
        if not names_m or not eff_m:
            continue
        names = [x.strip().strip(",") for x in names_m.group(1).split(",")]
        names = [x for x in names if x]
        eff = eff_m.group(1)
        for n in names:
            if eff == "net":
                net.add(n)
            elif eff == "db":
                db.add(n)
    return net, db


ANNO_RE = re.compile(
    r"^\s*;;\s*@in\s+\S.*\s+@out\s+\S.*\s+@effect\s+(pure|io|net|db|process)\s*$"
)
DEFN_RE = re.compile(r"^\s*\(defn\s+(\S+)")
CALL_RE = re.compile(r"\(([A-Za-z_?!][A-Za-z0-9_?!*/+-]*)")


def collect_sexpr(lines: list[str], start: int) -> tuple[str, int, int]:
    """Return (text, start_lineno_1based, next_index) for sexpr starting at start."""
    buf: list[str] = []
    depth = 0
    started = False
    i = start
    while i < len(lines):
        line = lines[i]
        # strip ; comments outside strings (rough)
        cleaned = []
        in_str = False
        esc = False
        for ch in line:
            if esc:
                cleaned.append(ch)
                esc = False
                continue
            if ch == "\\" and in_str:
                cleaned.append(ch)
                esc = True
                continue
            if ch == '"':
                in_str = not in_str
                cleaned.append(ch)
                continue
            if ch == ";" and not in_str:
                break
            cleaned.append(ch)
        s = "".join(cleaned)
        buf.append(s)
        for ch in s:
            if ch == "(":
                depth += 1
                started = True
            elif ch == ")":
                depth -= 1
        i += 1
        if started and depth <= 0:
            break
    return "\n".join(buf), start + 1, i


def check_file(path: Path, airc: Path) -> list[str]:
    net, db = load_net_db(airc)
    bad = net | db
    lines = path.read_text(encoding="utf-8").splitlines()
    warnings: list[str] = []
    pending: str | None = None
    i = 0
    while i < len(lines):
        line = lines[i]
        am = ANNO_RE.match(line)
        if am:
            pending = am.group(1)
            i += 1
            continue
        if re.match(r"^\s*$", line) or re.match(r"^\s*;", line):
            i += 1
            continue
        dm = DEFN_RE.match(line)
        if dm:
            name = dm.group(1)
            body, lineno, next_i = collect_sexpr(lines, i)
            if pending == "pure":
                found: list[str] = []
                for m in CALL_RE.finditer(body):
                    op = m.group(1)
                    if op in ("defn", "define", "let", "do", "if", "cond", "fn", "when", "unless"):
                        continue
                    if op == name:
                        continue
                    if op in bad and op not in found:
                        found.append(op)
                for op in found:
                    kind = "net" if op in net else "db"
                    warnings.append(
                        f"  줄 {lineno} [effect-purity] (defn {name}) "
                        f"@effect pure 인데 {kind} 빌트인 ({op}) 호출 — "
                        f"docs/FX-BUILTIN-EFFECTS.airc"
                    )
            pending = None
            i = next_i
            continue
        # other top-level form clears pending annotation
        pending = None
        i += 1
    return warnings


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: fl-lint-effect-purity.py <file.fl> <FX-BUILTIN-EFFECTS.airc>", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    airc = Path(sys.argv[2])
    for w in check_file(path, airc):
        print(w)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
