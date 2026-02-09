#!/usr/bin/env bash
set -euxo pipefail

# Usage:
#   ./run.sh
#
# Make sure you've built uaf_demo.bc first.

klee --search=bfs --max-time=30s --exit-on-error-type=Assert type_confusion.bc
echo "[OK] KLEE finished; see klee-out-* for errors/tests"
