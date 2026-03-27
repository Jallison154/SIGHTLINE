"""Simple file + stderr logging for the test environment."""

from __future__ import annotations

import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional, TextIO


def utc_ts() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


class SimLogger:
    def __init__(self, name: str, log_dir: Optional[Path] = None, file: Optional[TextIO] = None):
        self.name = name
        self._file = file
        self._path: Optional[Path] = None
        if log_dir is not None and file is None:
            log_dir.mkdir(parents=True, exist_ok=True)
            safe = name.replace("/", "_").replace(" ", "_")
            self._path = log_dir / f"{safe}_{utc_ts().replace(':', '')}.log"
            self._file = self._path.open("w", encoding="utf-8")
            self._file.write(f"# SIGHTLINE sim log {utc_ts()} {name}\n")

    def log(self, line: str, *, also_stderr: bool = True) -> None:
        full = f"[{utc_ts()}] {line}\n"
        if self._file:
            self._file.write(full)
            self._file.flush()
        if also_stderr:
            sys.stderr.write(full)

    def close(self) -> None:
        if self._file and self._path:
            self._file.close()
