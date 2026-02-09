# WMI-2: Type Confusion (1-Basic)

This repository documents how a type confusion vulnerability was modeled and confirmed in the **1-basic** challenge using KLEE. The main focus is the driver, how it was derived from a larger codebase, and how it proves that freed memory is later reused incorrectly.

---

## Modeling Approach

### From a Large Codebase to One Buggy Path

The original program includes menus, interactive input, and many execution paths that are unrelated to the bug. Running symbolic execution on the whole program would be noisy and ineffective.

Instead, I manually read the code and isolated the smallest sequence of calls needed to trigger the issue:

```bash
init_system()
→ set_avatar()
→ clear_avatar()
→ allocate start location
→ render_hex()
```


This path is enough to:

- allocate an avatar,
- free it without fully clearing references,
- allocate a new object that may reuse the same heap memory,
- and later access that memory through a stale pointer.

All other logic was ignored so KLEE could focus only on the behavior that matters.

---

## Driver Structure

The driver replaces the original `main()` and directly executes the buggy sequence. It removes menu logic and stdin usage so execution is fully controlled.

In the driver:

- inputs like `username`, `access_code`, and `location_input` are made symbolic,
- minimal constraints are applied so the program does not exit early,
- and only the allocations relevant to the bug are performed.

This keeps the search space manageable while still allowing heap reuse to occur.

---

## How the Heap Reuse Happens

After the avatar is created, the driver frees it using `clear_avatar()`. At this point, the avatar’s memory is released, but the global pointer to it may still exist.

Next, the driver allocates a `start_loc` object. This allocation uses the same heap allocator and can reuse the memory that previously belonged to the avatar. If that happens, two different objects now occupy the same memory location.

This is where the type confusion begins.

---

## What Is Actually Being Confused

The reason this bug leaks data is due to how the two structs are laid out in memory.

Both the `avatar` and the `start_loc` structs store their most important pointer in the first field of the struct. When the memory is reused:

- what used to be `avatar->username`
- now holds `start_loc->location_name`

So when the program later treats the memory as an avatar and reads `avatar->username`, it is actually reading the location name from the `start_loc` object. The memory is valid for the new object, but the program is interpreting it as the wrong type.

---

## Proving the Bug with KLEE

The driver proves the type confusion bug using two explicit checks. These checks make the bug observable to KLEE instead of relying on a crash or undefined behavior.

### 1. Stale Pointer Still Exists

```c
klee_assert(g_session.current_avatar != NULL);
```

This assertion shows that after the avatar is freed, the global pointer to it can still exist. If the program were handling memory correctly, this pointer would always be cleared. Keeping it alive is what allows the bug to happen.

### 2. Heap Overlap (Type Confusion)

```c
if (g_session.current_avatar == (avatar *)g_session.start_loc) {
    klee_check_memory_access(g_session.current_avatar, sizeof(avatar));
}
```

This condition checks whether the memory used for the new ``` start_loc ``` object overlaps with the memory that previously belonged to the avatar. This can only happen if the freed avatar memory is reused by the allocator and both pointers end up pointing to the same heap address.
Once KLEE reaches this block, it reports a use-after-free memory error when the stale pointer is accessed. Because this error only occurs after the pointer overlap is confirmed, it shows that the issue is not accidental — the program is genuinely reusing freed memory and accessing it through the wrong type.
This confirms a real type confusion caused by heap reuse, not just a generic use-after-free.
