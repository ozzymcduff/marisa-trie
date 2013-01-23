#ifndef MARISA_GRIMOIRE_INTRIN_H_
#define MARISA_GRIMOIRE_INTRIN_H_

#if defined(__x86_64__) || defined(_M_X64)
 #define MARISA_X64
#elif defined(__i386__) || defined(_M_IX86)
 #define MARISA_X86
#else  // defined(__i386__) || defined(_M_IX86)
 #ifdef MARISA_USE_SSE4_2
  #undef MARISA_USE_SSE4_2
 #endif  // MARISA_USE_SSE4_2
 #ifdef MARISA_USE_SSSE3
  #undef MARISA_USE_SSSE3
 #endif  // MARISA_USE_SSSE3
 #ifdef MARISA_USE_SSE2
  #undef MARISA_USE_SSE2
 #endif  // MARISA_USE_SSE2
#endif  // defined(__i386__) || defined(_M_IX86)

#ifdef MARISA_USE_SSE4_2
 #ifndef MARISA_USE_SSSE3
  #define MARISA_USE_SSSE3
 #endif  // MARISA_USE_SSSE3
 #ifdef MARISA_X64
  #define MARISA_X64_SSE4_2
 #else  // MARISA_X64
  #define MARISA_X86_SSE4_2
 #endif  // MAIRSA_X64
 #include <nmmintrin.h>
#endif  // MARISA_USE_SSE4_2

#ifdef MARISA_USE_SSSE3
 #ifndef MARISA_USE_SSE2
  #define MARISA_USE_SSE2
 #endif  // MARISA_USE_SSE2
 #ifdef MARISA_X64
  #define MARISA_X64_SSSE3
 #else  // MARISA_X64
  #define MARISA_X86_SSSE3
 #endif  // MAIRSA_X64
 #include <tmmintrin.h>
#endif  // MARISA_USE_SSSE3

#ifdef MARISA_USE_SSE2
 #ifdef MARISA_X64
  #define MARISA_X64_SSE2
 #else  // MARISA_X64
  #define MARISA_X86_SSE2
 #endif  // MAIRSA_X64
 #include <emmintrin.h>
#endif  // MARISA_USE_SSE2

#endif  // MARISA_GRIMOIRE_INTRIN_H_
