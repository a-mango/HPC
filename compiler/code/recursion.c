/**
 * @file recursion.c
 * @brief Demo recursion program
 * @date 2025-04-08
 */

// Unoptimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
#pragma GCC pop_options

// Manually optimized version
#pragma GCC push_options
#pragma GCC optimize("-O0")
int factorial_manual(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}
#pragma GCC pop_options

// Compiler-optimized version
#pragma GCC push_options
#pragma GCC optimize("O2")
int factorial_compiler(int n) {
    if (n <= 1) return 1;
    return n * factorial_compiler(n - 1);
}
#pragma GCC pop_options
