#!/usr/bin/env bash
set -euxo pipefail

# Path to KLEE include directory
: "${KLEE_INC:=/home/wcontrerasmartinez/klee/include}"

echo "[INFO] pwd=$(pwd)"
echo "[INFO] KLEE_INC=$KLEE_INC"

# Verification: Ensure all necessary files exist
test -f driver_type_confusion.c
test -f stubs_demo.c
test -f ../metalogin.c
test -f ../metalogin.h

KLEE_ARGS=()
if [ -f "$KLEE_INC/klee/klee.h" ]; then
  KLEE_ARGS=(-I"$KLEE_INC" -D__KLEE__)
else
  echo "[WARN] klee.h not found at $KLEE_INC/klee/klee.h; building without __KLEE__"
fi

# 1. Compile the Type Confusion Driver
clang "${KLEE_ARGS[@]}" -I.. -O0 -g -emit-llvm -c driver_type_confusion.c -o driver_type_confusion.bc

# 2. Compile the Environment Stubs
clang "${KLEE_ARGS[@]}" -I.. -O0 -g -emit-llvm -c stubs_demo.c -o stubs_demo.bc

# 3. Compile metalogin.c (Using -DKLEE_DRIVER_BUILD to ignore its original main)
clang "${KLEE_ARGS[@]}" -DKLEE_DRIVER_BUILD -I.. -O0 -g -emit-llvm -c ../metalogin.c -o metalogin.bc

# 4. Handle extra dependencies (globals.c)
EXTRA_BC=()
if [ -f ../globals.c ]; then
  clang "${KLEE_ARGS[@]}" -DKLEE_DRIVER_BUILD -I.. -O0 -g -emit-llvm -c ../globals.c -o globals.bc
  EXTRA_BC+=(globals.bc)
fi

# 5. Link everything together into a single bitcode file
# We rename the output to 'type_confusion.bc' to keep it distinct from the UAF demo
llvm-link driver_type_confusion.bc stubs_demo.bc metalogin.bc "${EXTRA_BC[@]}" -o type_confusion.bc

ls -lah *.bc
echo "[OK] Built type_confusion.bc"