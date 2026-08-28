#ifndef UPRINT_H
#define UPRINT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__has_attribute) && __has_attribute(__uprint_loaded__)
// PLUGIN LOADED
// Both uprint() & __uprint_emit() functions shall be declared

/**
 * @brief High-level application logging entry point.
 *
 * This prototype provides code intelligence (IntelliSense / LSP auto-complete)
 * and prevents implicit function declaration compiler warnings.
 *
 * @note When the µprint GCC plugin is enabled during compilation, all calls to 
 *       uprint() are intercepted and rewritten at the GIMPLE IR level. The 
 *       format string is extracted to host ELF metadata, and argument payloads 
 *       are packed and redirected to __uprint_emit().
 *
 * @param format_string Standard C printf-style format string.
 * @param ...           Variadic format arguments matching specifiers.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 1, 2)))
#endif
void uprint(const char *format_string, ...);

/**
 * @brief Low-level binary transport callback for log emission.
 *
 * This function is invoked by compiler-generated code whenever a uprint()
 * statement executes. It receives a fully serialized, contiguous memory buffer.
 *
 * @attention REQUIRED IMPLEMENTATION: The user MUST define this function in 
 *            exactly ONE source file (.c) within the target project (e.g., 
 *            in bsp.c or main.c). Do not define it inside this header file, 
 *            as doing so will cause multiple-definition linker errors in 
 *            multi-file projects.
 *
 * @details The memory block pointed to by `data` is a packed binary packet.
 *
 *          Your implementation should forward this raw byte array directly to 
 *          your target transport peripheral (e.g., UART DMA, SEGGER RTT, or USB CDC).
 *
 * @param data   Pointer to the contiguous packed binary log packet in target memory.
 * @param length Total payload size of the binary log packet in bytes.
 */
void __uprint_emit(const void *data, uint16_t length);

#else // __has_attribute(__uprint_tokenized__)
// PLUGIN NOT LOADED

#ifdef NO_UPRINT
// If user defined the preprocessor macro (as -DNO_UPRINT), we will totally ignore all uprint() calls
#define uprint(...) ((void)0)

#else
// If user did not define the preprocessor macro, we will replace all uprint() calls with printf()
#include <stdio.h>
#define uprint(...) printf(__VA_ARGS__)

#endif  // NO_UPRINT

#endif // __has_attribute(__uprint_tokenized__)

#ifdef __cplusplus
}
#endif

#endif // UPRINT_H
