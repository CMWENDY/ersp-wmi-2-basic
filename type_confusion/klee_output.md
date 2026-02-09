```c
wcontrerasmartinez@Ezequiel:~/stase_klee_demo/1-basic/demo_klee$ ./run.sh
+ klee --search=bfs --max-time=30s --exit-on-error-type=Assert type_confusion.bc
KLEE: output directory is "/home/wcontrerasmartinez/stase_klee_demo/1-basic/demo_klee/klee-out-3"
KLEE: Using Z3 solver backend
KLEE: Deterministic allocator: Using quarantine queue size 8
KLEE: Deterministic allocator: globals (start-address=0x7a990d200000 size=10 GiB)
KLEE: Deterministic allocator: constants (start-address=0x7a968d200000 size=10 GiB)
KLEE: Deterministic allocator: heap (start-address=0x79968d200000 size=1024 GiB)
KLEE: Deterministic allocator: stack (start-address=0x79768d200000 size=128 GiB)
KLEE: WARNING ONCE: Alignment of memory from call "malloc" is not modelled. Using alignment of 8.
KLEE: WARNING ONCE: Alignment of memory from call "calloc" is not modelled. Using alignment of 8.
KLEE: ERROR: ../metalogin.c:408: memory error: use after free
KLEE: NOTE: now ignoring this error at this location
KLEE: ERROR: driver_type_confusion.c:78: ASSERTION FAIL: g_session.current_avatar != NULL
KLEE: NOTE: now ignoring this error at this location
KLEE: halting execution, dumping remaining states

KLEE: done: total instructions = 857333
KLEE: done: completed paths = 0
KLEE: done: partially completed paths = 5047
KLEE: done: generated tests = 2396
+ echo '[OK] KLEE finished; see klee-out-* for errors/tests'
```
