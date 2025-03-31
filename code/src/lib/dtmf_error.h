/**
 * @file dtmf_error.h
 * @brief Error handling utilities for the DTMF library.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-26
 */

#ifndef DTMF_ERROR_H
#define DTMF_ERROR_H


/*
 * @brief DTMF error codes.
 * @details Used to indicate the status of a DTMF library operation.
 * @see The DTMF_SUCCEED() and DTMF_FAIL() are macros that return the corresponding error code.
 */
typedef enum {
    DTMF_SUCCESS = false,
    DTMF_FAILURE = true,
} dtmf_error_t;


#define DTMF_SUCCEED() return DTMF_SUCCESS
#define DTMF_FAIL() return DTMF_FAILURE

#define DTMF_EXIT_SUCCESS()  \
    do {                     \
        LIKWID_MARKER_CLOSE; \
        exit(EXIT_SUCCESS);  \
    } while (0)

#define DTMF_EXIT_FAILURE()  \
    do {                     \
        LIKWID_MARKER_CLOSE; \
        exit(EXIT_FAILURE);  \
    } while (0)


#ifdef DEBUG
#define DTMF_TRACE(fmt, ...) fprintf(stderr, "Trace: " fmt "\n"__VA_OPT__(, )__VA_ARGS__)
#define DTMF_DEBUG(fmt, ...) fprintf(stderr, "Debug: " fmt "\n"__VA_OPT__(, )__VA_ARGS__)
#else
#define DTMF_TRACE(...) ((void)0)
#define DTMF_DEBUG(...) ((void)0)
#endif

#define DTMF_INFO(fmt, ...) fprintf(stdout, fmt "\n"__VA_OPT__(, )__VA_ARGS__)
#define DTMF_WARN(fmt, ...) fprintf(stderr, "Warning: " fmt "\n"__VA_OPT__(, )__VA_ARGS__)

#define DTMF_ERROR(fmt, ...)                                          \
    do {                                                              \
        fprintf(stderr, "Error: " fmt "\n"__VA_OPT__(, )__VA_ARGS__); \
        return DTMF_FAILURE;                                          \
    } while (0)

#define DTMF_FATAL(fmt, ...)                                          \
    do {                                                              \
        fprintf(stderr, "Fatal: " fmt "\n"__VA_OPT__(, )__VA_ARGS__); \
        exit(EXIT_FAILURE);                                           \
    } while (0)

#define DTMF_ASSERT(CONDITION, fmt, ...)  \
    do {                                  \
        if (!(CONDITION)) {               \
            DTMF_ERROR(fmt, __VA_ARGS__); \
        }                                 \
    } while (0)

#define DTMF_HANDLE(RESULT)           \
    do {                              \
        if (RESULT != DTMF_SUCCESS) { \
            return DTMF_FAILURE;      \
        }                             \
    } while (0)

#endif  // DTMF_ERROR_H
