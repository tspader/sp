Always use named constants for magic numbers, even when they seem obvious.

# Good
```c
#define SPN_TUI_NUM_OPTIONS 3

// Loop through options
for (u32 j = 0; j < SPN_TUI_NUM_OPTIONS; j++) {
  if (input_char == dep->confirm_keys[j]) {
    // Handle input
  }
}

// Declaring array with constant
c8 confirm_keys[SPN_TUI_NUM_OPTIONS];
```

# Bad
```c
// Checking three options directly
if (input_char == dep->confirm_keys[0] ||
    input_char == dep->confirm_keys[1] ||
    input_char == dep->confirm_keys[2]) {
  // Handle input
}

// Declaring array with magic number
c8 confirm_keys[3];
```
