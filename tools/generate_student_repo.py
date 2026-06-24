#!/usr/bin/env python3
"""Student starter helper for MiniRedis17.

This repository is already the student starter. The complete reference copy is
stored outside this tree at /home/scx/dev/miniredis17-reference-20260624/.
Use CMake's MINIREDIS_TASK_LEVEL option to enable staged public tests.
"""

import argparse

TASKS = [
    "Error Model and ByteBuffer",
    "RESP Values and Encoder",
    "Incremental RESP Parser",
    "Database and String Commands",
    "List and Hash Commands",
    "Session and Pipelining",
    "Nonblocking TCP and epoll",
    "TTL and Expiration",
    "AOF Persistence and Recovery",
]


def main() -> int:
    parser = argparse.ArgumentParser(description="Show MiniRedis17 staged-test commands")
    parser.add_argument("--task", type=int, default=0, help="task level to enable, 0-8")
    args = parser.parse_args()
    if args.task < 0 or args.task >= len(TASKS):
        parser.error("--task must be in 0..8")

    print(f"Task {args.task}: {TASKS[args.task]}")
    print("This tree is already the student starter; no generation step is required.")
    print("Configure/build/test with:")
    print(f"  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMINIREDIS_TASK_LEVEL={args.task}")
    print("  cmake --build build -j")
    print("  ctest --test-dir build --output-on-failure")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
