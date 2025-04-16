/**
 * @file branch.c
 * @brief Demo branch removal program
 * @date 2025-04-08
 */

// Unoptimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
int branch(int x) {
    if (x == 0)
        return 1;
    return 0;
}
#pragma GCC pop_options

// Manually optimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
int branch_manual(int x) {
    return x == 0;
}
#pragma GCC pop_options

// Compiler-optimized version
#pragma GCC push_options
#pragma GCC optimize("O2")
int branch_compiler(int x) {
    if (x == 0)
        return 1;
    return 0;
}
#pragma GCC pop_options
