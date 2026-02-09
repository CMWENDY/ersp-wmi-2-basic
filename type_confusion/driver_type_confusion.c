#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#ifdef __KLEE__
  #include "klee/klee.h"
  // Helper to check if a pointer is valid
  extern void klee_check_memory_access(const void *p, size_t n);
#else
  #define klee_make_symbolic(a,b,c) ((void)0)
  #define klee_assume(x) ((void)0)
  #define klee_check_memory_access(p,n) ((void)0)
  #define klee_assert(x) assert(x)
#endif

#include "../metalogin.h"

// External functions from metalogin.c
void init_system(void);
void set_avatar(char *username, char *access_code);
void clear_avatar(void);
void set_start_location(void); 
void render_hex(void);

extern session g_session;

int main(void) {
    init_system();

    char username[MAX_LENGTH];
    char access_code[MAX_LENGTH];
    
    // We also need a symbolic input for the location name to trigger set_start_location
    char location_input[MAX_LENGTH];

#ifdef __KLEE__
    klee_make_symbolic(username, sizeof(username), "username");
    klee_make_symbolic(access_code, sizeof(access_code), "access_code");
    klee_make_symbolic(location_input, sizeof(location_input), "location_input");

    // Concrete termination
    username[sizeof(username) - 1] = 0;
    access_code[sizeof(access_code) - 1] = 0;
    location_input[sizeof(location_input) - 1] = 0;

    // Constraints to ensure functions don't exit early due to empty strings
    klee_assume((unsigned char)username[0] != 0);
    klee_assume((unsigned char)location_input[0] != 0);
#endif

    // --- STEP 1: Create the Avatar ---
    set_avatar(username, access_code);
    
    // Save the pointer address to verify the overlap later
    void *original_avatar_ptr = (void *)g_session.current_avatar;

    // --- STEP 2: Free the Avatar (Creating the Stale Reference) ---
    clear_avatar();

    // --- STEP 3: Allocate the Location (The Overlap) ---
    // Note: We normally call set_start_location(), but since it uses fgets, 
    // we simulate its core logic here using our symbolic location_input.
    // This bypasses the interactive UI while keeping the vulnerability.
    
    size_t len = strnlen(location_input, MAX_LENGTH-1);
    g_session.start_loc = malloc(sizeof(start_loc));
    if (g_session.start_loc) {
        g_session.start_loc->location_name = malloc(len + 1);
        if (g_session.start_loc->location_name) {
            memcpy(g_session.start_loc->location_name, location_input, len + 1);
        }
    }

    // --- STEP 4: PROVE TYPE CONFUSION ---
#ifdef __KLEE__
    // Assertion 1: Prove the "Stale Reference" still exists
    // If the code were safe, current_avatar would be NULL.
    klee_assert(g_session.current_avatar != NULL);

    // Assertion 2: Prove the "Overlap" (Type Confusion)
    // We prove that the NEW location struct is sitting in the OLD avatar's house.
    if (g_session.current_avatar == (avatar *)g_session.start_loc) {
        // If KLEE reaches this line, it has mathematically proven 
        // that 'current_avatar' and 'start_loc' occupy the same memory.
        
        // This check will fail if KLEE detects the memory is freed but used.
        klee_check_memory_access(g_session.current_avatar, sizeof(avatar));
    }
#endif

    // --- STEP 5: Trigger the Leak ---
    render_hex();

    return 0;
}
