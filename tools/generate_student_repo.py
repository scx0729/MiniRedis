#!/usr/bin/env python3
"""
Generate student repository from reference implementation.

Uses code region markers in the reference implementation:
    // BEGIN STUDENT TODO(task=N)
    // ... reference implementation ...
    // END STUDENT TODO(task=N)

For a given task number T, this tool:
1. Keeps all code for tasks 0..T-1 (already completed by student)
2. Replaces code in task T markers with compilable stubs
3. Removes code for tasks T+1..8 (not yet reached)
4. Removes reference-only files (docs/INSTRUCTOR_GUIDE.md, etc.)

Usage:
    python3 tools/generate_student_repo.py --task 3 --output student/
"""

import argparse
import os
import re
import shutil
import sys

BEGIN_MARKER = re.compile(r'//\s*BEGIN STUDENT TODO\(task=(\d+)\)')
END_MARKER = re.compile(r'//\s*END STUDENT TODO\(task=(\d+)\)')

def process_file(content: str, task: int) -> str:
    """Process a single file, replacing/removing code based on task number."""
    lines = content.split('\n')
    result = []
    skip = False
    current_task = -1

    for line in lines:
        begin_match = BEGIN_MARKER.search(line)
        end_match = END_MARKER.search(line)

        if begin_match:
            current_task = int(begin_match.group(1))
            if current_task < task:
                # Already completed - keep the code (markers will be removed)
                skip = False
                continue  # skip the BEGIN marker itself
            elif current_task == task:
                # Current task - replace with stub
                skip = True
                result.append(line)  # keep BEGIN marker
                result.append('    // TODO: Implement this function')
                result.append('    // See tasks/task{}/README.md for instructions'.format(task))
                continue
            else:
                # Future task - remove entirely
                skip = True
                continue

        if end_match:
            current_task = int(end_match.group(1))
            if current_task <= task:
                if current_task == task:
                    result.append(line)  # keep END marker for current task
                skip = False
                continue
            else:
                skip = False
                continue

        if not skip:
            result.append(line)

    return '\n'.join(result)


def should_include(path: str, task: int) -> bool:
    """Check if a file/directory should be included for the given task."""
    # Exclude instructor-only files
    instructor_only = [
        'docs/INSTRUCTOR_GUIDE.md',
        'docs/REFERENCE_IMPLEMENTATION_REPORT.md',
        'docs/GRADING_GUIDE.md',
        'tests/hidden_spec/',
        'tools/generate_student_repo.py',
    ]
    for excl in instructor_only:
        if path.startswith(excl):
            return False

    # Exclude future task directories
    for i in range(task + 1, 9):
        if path.startswith(f'tasks/task{i}/'):
            return False

    return True


def main():
    parser = argparse.ArgumentParser(description='Generate student repository')
    parser.add_argument('--task', type=int, required=True, help='Task number (0-8)')
    parser.add_argument('--output', type=str, required=True, help='Output directory')
    args = parser.parse_args()

    if args.task < 0 or args.task > 8:
        print(f"Error: task must be 0-8, got {args.task}")
        sys.exit(1)

    src_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dst_dir = os.path.abspath(args.output)

    if os.path.exists(dst_dir):
        print(f"Error: output directory already exists: {dst_dir}")
        sys.exit(1)

    print(f"Generating student repo for task {args.task} -> {dst_dir}")

    # Copy and process files
    for root, dirs, files in os.walk(src_dir):
        # Skip build directories and hidden dirs
        dirs[:] = [d for d in dirs if not d.startswith('.') and not d.startswith('build')]

        rel_root = os.path.relpath(root, src_dir)
        if not should_include(rel_root + '/', args.task):
            continue

        for fname in files:
            rel_path = os.path.join(rel_root, fname) if rel_root != '.' else fname
            if not should_include(rel_path, args.task):
                continue

            src_path = os.path.join(root, fname)
            dst_path = os.path.join(dst_dir, rel_path)

            os.makedirs(os.path.dirname(dst_path), exist_ok=True)

            # Process source files with markers
            if fname.endswith(('.cpp', '.h')):
                with open(src_path, 'r') as f:
                    content = f.read()
                processed = process_file(content, args.task)
                with open(dst_path, 'w') as f:
                    f.write(processed)
            else:
                shutil.copy2(src_path, dst_path)

    print(f"Student repository generated at: {dst_dir}")
    print(f"Build: cmake -S {dst_dir} -B {dst_dir}/build && cmake --build {dst_dir}/build -j")
    print(f"Test: ctest --test-dir {dst_dir}/build --output-on-failure")


if __name__ == '__main__':
    main()
