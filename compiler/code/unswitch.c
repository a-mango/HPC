/**
 * @file unswitch.c
 * @brief Demo unswitching program
 * @date 2025-04-08
 */

// Unoptimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
void unswitch(int *arr, int flag) {
    for (int i = 0; i < 3; i++) {
        if (flag) {
            arr[i] *= 2;
        }
    }
}
#pragma GCC pop_options

// Manually optimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
void unswitch_manual(int *arr, int flag) {
    if (flag) {
        for (int i = 0; i < 3; i++) {
            arr[i] *= 2;
        }
    }
}
#pragma GCC pop_options

// Compiler-optimized version
#pragma GCC push_options
#pragma GCC optimize("-O1", "-funswitch-loops")
void unswitch_compiler(int *arr, int flag) {
    for (int i = 0; i < 3; i++) {
        if (flag) {
            arr[i] *= 2;
        }
    }
}
#pragma GCC pop_options
