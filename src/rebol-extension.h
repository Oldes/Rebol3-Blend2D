////////////////////////////////////////////////////////////////////////
// File: rebol-extension.h
// Home: https://github.com/Oldes/Rebol3/
// Date: 21-Sep-2026/13:23:49
// Note: This file is amalgamated from these sources:
//
//       reb-c.h
//       reb-args.h
//       reb-ext.h
//       reb-ext-handler.h
//       reb-codec.h
//       reb-device.h
//       reb-file.h
//       reb-filereq.h
//       reb-event.h
//       reb-evtypes.h
//       reb-lib.h
//       reb-ext-common.h
//
////////////////////////////////////////////////////////////////////////

// File: reb-c.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2024 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: General C definitions and constants
**  Module:  reb-c.h
**  Author:  Carl Sassenrath, Ladislav Mecir
**  Notes:
**      Various configuration defines (from reb-config.h):
**
**      HAS_LL_CONSTS - compiler allows 1234LL constants
**      ODD_INT_64 - old MSVC typedef for 64 bit int
**      OS_WIDE_CHAR - the OS uses wide chars (not UTF-8)
**
***********************************************************************/
#ifndef REB_C_H
#define REB_C_H

#ifndef FALSE
#define FALSE 0
#define TRUE (!0)
#endif

/***********************************************************************
**
**  C-Code Types
**
**      One of the biggest flaws in the C language was not
**      to indicate bitranges of integers. So, we do that here.
**      You cannot "abstractly remove" the range of a number.
**      It is a critical part of its definition.
**
***********************************************************************/

#ifdef __OBJC__
#define HAS_BOOL // don't redefine BOOL in objective-c code
#endif

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* C-code types: use C99 */

#include <stdint.h>

typedef int8_t			i8;
typedef uint8_t			u8;
typedef int16_t			i16;
typedef uint16_t		u16;
typedef int32_t			i32;
typedef uint32_t		u32;
typedef int64_t			i64;
typedef uint64_t		u64;
typedef intptr_t		REBIPT;		// integral counterpart of void*
typedef uintptr_t		REBUPT;		// unsigned counterpart of void*

#define MAX_I32 INT32_MAX
#define MIN_I32 INT32_MIN
#define MAX_I64 INT64_MAX
#define MIN_I64 INT64_MIN

#define I8_C(c)			INT8_C(c)
#define U8_C(c)			UINT8_C(c)

#define I16_C(c)		INT16_C(c)
#define U16_C(c)		UINT16_C(c)

#define I32_C(c)		INT32_C(c)
#define U32_C(c)		UINT32_C(c)

#define I64_C(c)		INT64_C(c)
#define U64_C(c)		UINT64_C(c)

#else
/* C-code types: C99 definitions unavailable, do it ourselves */

typedef char			i8;
typedef unsigned char	u8;
#define I8(c) 			c
#define U8(c) 			c

typedef short			i16;
typedef unsigned short	u16;
#define I16(c) 			c
#define U16(c) 			c

#ifdef __LP64__
typedef int				i32;
typedef unsigned int	u32;
#else
typedef long            i32;
typedef unsigned long	u32;
#endif
#define I32_C(c) c
#define U32_C(c) c ## U

#ifdef ODD_INT_64       // Windows VC6 nonstandard typing for 64 bits
typedef _int64          i64;
typedef unsigned _int64 u64;
#define I64_C(c) c ## I64
#define U64_C(c) c ## U64
#else
typedef long long       i64;
typedef unsigned long long u64;
#define I64_C(c) c ## LL
#define U64_C(c) c ## ULL
#endif
#ifdef __LLP64__
typedef long long		REBIPT;		// integral counterpart of void*
typedef unsigned long long	REBUPT;		// unsigned counterpart of void*
#else
typedef long			REBIPT;		// integral counterpart of void*
typedef unsigned long	REBUPT;		// unsigned counterpart of void*
#endif

#define MAX_I32 I32_C(0x7fffffff)
#define MIN_I32 ((i32)I32_C(0x80000000)) //compiler treats the hex literal as unsigned without casting
#define MAX_I64 I64_C(0x7fffffffffffffff)
#define MIN_I64 ((i64)I64_C(0x8000000000000000)) //compiler treats the hex literal as unsigned without casting

#endif
/* C-code types */

#define MAX_U32 U32_C(0xffffffff)
#define MAX_U64 U64_C(0xffffffffffffffff)

#ifndef DEF_UINT		// some systems define it, don't define it again
typedef unsigned int    uint;
#endif

// Some systems define a cpu-optimal BOOL already. It is assumed that the
// R3 lib will use that same definition (so sizeof() is identical.)
// (Of course, all of this should have been built into C in 1970.)
#ifndef HAS_BOOL
typedef int BOOL;       // (int is used for speed in modern CPUs)
#endif

// Used for cases where we need 64 bits, even in 32 bit mode.
// (Note: compatible with FILETIME used in Windows)
#pragma pack(4)
typedef struct sInt64 {
	i32 l;
	i32 h;
} I64;
#pragma pack()


/* ---------------------------------------------------------------------
	The following 4 definitions are compiler-specific.
	The C standard does not guarantee that wchar_t has at least
	16 bits, so wchar_t is no less portable than unsigned short!
	All should be unsigned values to avoid sign extension during
	bit mask & shift operations.
------------------------------------------------------------------------ */

typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */
typedef unsigned char	Boolean; /* 0 or 1 */

/* Some fundamental constants */
#define UNI_ERROR (UTF32)0xFFFFFFFF
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF


/***********************************************************************
**
**  REBOL Code Types
**
***********************************************************************/

typedef i32				REBINT;     // 32 bit (64 bit defined below)
typedef u32				REBCNT;     // 32 bit (counting number)
typedef u32             REBLEN;     // 32 bit series length/index - used instead of size_t
typedef u32             REBU32;		// 32 bit Unicode
typedef i64				REBI64;     // 64 bit integer
typedef u64				REBU64;     // 64 bit unsigned integer
typedef i8				REBOOL;     // 8  bit flag (for struct usage)
typedef u32				REBFLG;     // 32 bit flag (for cpu efficiency)
typedef float			REBD32;     // 32 bit decimal
typedef double			REBDEC;     // 64 bit decimal

typedef unsigned char	REBYTE;     // unsigned byte data
typedef u16				REBUNI;     // unicode char
typedef u16				REBU16;     // 16bit UCS2 codepoint
typedef u32 REBUTF;

// REBCHR - only to refer to OS char strings (not internal strings)
#ifdef OS_WIDE_CHAR
typedef REBUNI          REBCHR;
#else
typedef REBYTE          REBCHR;
#endif

#define AS_REBLEN(v)  (REBLEN)(v)  // used to silence conversion from size_t warnings
#define AS_INT(v)        (int)(v)  // used to silence conversion from size_t warnings

#define MIN_D64 ((double)-9.2233720368547758e18)
#define MAX_D64 ((double) 9.2233720368547758e18)
#define MAX_REBLEN 0xFFFFFFFF
#define MAX_UNI 0x10FFFF

// Useful char constants:
enum {
	BEL =   7,
	BS  =   8,
	LF  =  10,
	CR  =  13,
	ESC =  27,
	DEL = 127
};

// Used for MOLDing:
#define MAX_DIGITS 17   // number of digits
#define MAX_NUMCHR 32   // space for digits and -.e+000%

/***********************************************************************
**
**  64 Bit Integers - Now supported in REBOL 3.0
**
***********************************************************************/

#define MAX_INT_LEN     25
#define MAX_HEX_LEN     16

#ifdef ITOA64           // Integer to ascii conversion
#define INT_TO_STR(n,s) _i64toa(n, s_cast(s), 10)
#else
#define INT_TO_STR(n,s) Form_Int_Len(s, n, MAX_INT_LEN)
#endif

#ifdef ATOI64           // Ascii to integer conversion
#define CHR_TO_INT(s)   _atoi64(cs_cast(s))
#else
#define CHR_TO_INT(s)   strtoll(cs_cast(s), 0, 10)
#endif

#define LDIV            lldiv
#define LDIV_T          lldiv_t

/***********************************************************************
**
**  Address and Function Pointers
**
***********************************************************************/

#ifdef TO_WINDOWS
typedef long (__stdcall *FUNCPTR)();
typedef void(__cdecl *CFUNC)(void *);
#else
typedef long (*FUNCPTR)(void);
typedef void(*CFUNC)(void *);
#endif

/***********************************************************************
**
**  Useful Macros
**
***********************************************************************/

#ifdef __has_builtin
#  define HAS_BUILTIN(x) __has_builtin(x)
#else
#  define HAS_BUILTIN(x) 0
#endif

#if defined(_MSC_VER)
# define FORCE_INLINE    __forceinline
# include <stdlib.h>
# define ROTL32(x,y) _rotl(x,y)
# define ROTL64(x,y) _rotl64(x,y)
# define BIG_CONSTANT(x) (x)
// Other compilers..
#else   // defined(_MSC_VER)
# define FORCE_INLINE inline __attribute__((always_inline))
# if HAS_BUILTIN(__builtin_rotateleft32) && HAS_BUILTIN(__builtin_rotateleft64)
#  define ROTL32(x,y) __builtin_rotateleft32(x,y)
#  define ROTL64(x,y) __builtin_rotateleft64(x,y)
# else
    FORCE_INLINE uint32_t rotl32(uint32_t x, int8_t r)
    {
        return (x << r) | (x >> (32 - r));
    }
    FORCE_INLINE uint64_t rotl64(uint64_t x, int8_t r)
    {
        return (x << r) | (x >> (64 - r));
    }
#  define ROTL32(x,y) rotl32(x,y)
#  define ROTL64(x,y) rotl64(x,y)
# endif
# define BIG_CONSTANT(x) (x##LLU)
#endif // !defined(_MSC_VER)


#if defined(_MSC_VER)
#define RESTRICT __restrict
#elif defined(__clang__) || defined(__GNUC__)
#define RESTRICT __restrict__
#elif defined(__IAR_SYSTEMS_ICC__)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif

#define UNUSED(x) (void)x;

// Check a condition e at compile time. If the condition is false, it will
// result in a compilation error because it attempts to create an array 
// with a negative size, which is not allowed in C.
#define STATIC_ASSERT(e) do {(void)sizeof(char[1 - 2*!(e)]);} while(0)

#define FLAGIT(f)           (1u<<(f))
#define GET_FLAG(v,f)       (((v) & FLAGIT(f)) != 0)
#define GET_FLAGS(v,f,g)    (((v) & (FLAGIT(f) | FLAGIT(g))) != 0)
#define SET_FLAG(v,f)       ((v) |= FLAGIT(f))
#define CLR_FLAG(v,f)       ((v) &= ~FLAGIT(f))
#define CLR_FLAGS(v,f,g)    ((v) &= ~(FLAGIT(f) | FLAGIT(g)))
#define ASSIGN_FLAG(f, b, set) ((set) ? SET_FLAG(f, b) : CLR_FLAG(f, b))

#ifndef MIN
#ifdef min
#define MIN(a,b) min(a,b)
#define MAX(a,b) max(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif

// Memory related functions:
#define MAKE_MEM(n)     malloc(n)
#define MAKE_CLEAR_MEM(n)     calloc(n, 1)
#define MAKE_NEW(s)     MAKE_MEM(sizeof(s))
#define FREE_MEM(m)     free(m)
#define CLEAR(m, s)     memset((void*)(m), 0, s);
#define CLEAR_WTH(m, s, v)     memset((void*)(m), v, s);
#define CLEARS(m)       memset((void*)(m), 0, sizeof(*m));
#define COPY_MEM(t,f,l) memcpy((void*)(t), (void*)(f), l)
#define MOVE_MEM(t,f,l) memmove((void*)(t), (void*)(f), l)

// OS has wide char string interfaces:
#ifdef OS_WIDE_CHAR
#define OS_WIDE TRUE
#define TXT(s) (L##s)
#define COPY_STR(t,f,l) wcsncpy((wchar_t*)t,(const wchar_t*)f, l)
#define JOIN_STR(d,s,l) wcsncat((wchar_t*)d,(const wchar_t*)s,l)
#define FIND_STR(d,s)   (REBCHR*)wcsstr((const wchar_t*)d,s)
#define FIND_CHR(d,s)   (REBCHR*)wcschr((const wchar_t*)d,s)
#define LEN_STR(s)      wcslen((const wchar_t*)s)
#define TO_OS_STR(s1,s2,l)   mbstowcs(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) wcstombs(s1,(const wchar_t*)s2,l)
#else
// OS has UTF-8 byte string interfaces:
#define OS_WIDE FALSE
#define TXT(s) (s)
#define COPY_STR(t,f,l) strncpy((char *)t, (const char *)f, l)
#define JOIN_STR(d,s,l) strncat((char *)d,(const char *)s,l)
#define FIND_STR(d,s)   strstr((const char*)d,s)
#define FIND_CHR(d,s)   (REBCHR*)strchr((const char*)d,s)
#define LEN_STR(s)      strlen((const char*)s)
#define TO_OS_STR(s1,s2,l)   strncpy(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) strncpy(s1,(const char*)s2,l)
#endif

#define MAKE_STR(n) (REBCHR*)(MAKE_MEM((n) * sizeof(REBCHR)))  // OS chars!

#define ROUND_TO_INT(d) (REBINT)(floor((d) + 0.5))

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
// Note: 32bit, v must be > 0
#define U32_ROUND_UP_POWER_OF_2(v) \
                    v--;           \
                    v |= v >> 1;   \
                    v |= v >> 2;   \
                    v |= v >> 4;   \
                    v |= v >> 8;   \
                    v |= v >> 16;  \
                    v++;           \

/***********************************************************************
**
**  Color Macros
**
***********************************************************************/
//global pixelformat setup for REBOL image!, image loaders, color handling, tuple! conversions etc.
//the graphics compositor code should rely on this setting(and do specific conversions if needed)
//notes:
//TO_RGBA_COLOR always returns 32bit RGBA value, converts R,G,B,A components to native RGBA order
//TO_PIXEL_COLOR must match internal image! datatype byte order, converts R,G,B,A components to native image format
// C_R, C_G, C_B, C_A Maps color components to correct byte positions for image! datatype byte order

#ifdef ENDIAN_BIG

#define TO_RGBA_COLOR(r,g,b,a) (REBCNT)((r)<<24 | (g)<<16 | (b)<<8 |  (a))

//ARGB pixelformat used on big endian systems
#define C_A 0
#define C_R 1
#define C_G 2
#define C_B 3

#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (r)<<16 | (g)<<8 |  (b))

#else

#define TO_RGBA_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (b)<<16 | (g)<<8 |  (r))

//we use RGBA pixelformat on Android
#ifdef TO_ANDROID_ARM
#define C_R 0
#define C_G 1
#define C_B 2
#define C_A 3
#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (b)<<16 | (g)<<8 |  (r))
#else
//BGRA pixelformat is used on Windows
#define C_B 0
#define C_G 1
#define C_R 2
#define C_A 3
#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (r)<<16 | (g)<<8 |  (b))
#endif

#endif


/***********************************************************************
**
**  Conversion Macros
**
***********************************************************************/

/* interpret four 8 bit unsigned integers as a 32 bit unsigned integer in little endian
   originaly named LE in ChaCha20 code and U8TO32 in Poly1305 code */
#define U8TO32_LE(p)                  \
    (((u32)((p)[0]))       |          \
     ((u32)((p)[1]) << 8)  |          \
     ((u32)((p)[2]) << 16) |          \
     ((u32)((p)[3]) << 24))

/* store a 32 bit unsigned integer as four 8 bit unsigned integers in little endian
   originaly named FROMLE in ChaCha20 code */
#define U32TO8_LE(p, v)               \
  do {                                \
    (p)[0] = (u8)((v));               \
    (p)[1] = (u8)((v) >> 8);          \
    (p)[2] = (u8)((v) >> 16);         \
    (p)[3] = (u8)((v) >> 24);         \
  } while (0)

/* interpret eight 8 bit unsigned integers as a 64 bit unsigned integer in little endian */
#define U8TO64_LE(p)                  \
    (((u64)((p)[0] & 0xff)      ) |   \
     ((u64)((p)[1] & 0xff) <<  8) |   \
     ((u64)((p)[2] & 0xff) << 16) |   \
     ((u64)((p)[3] & 0xff) << 24) |   \
     ((u64)((p)[4] & 0xff) << 32) |   \
     ((u64)((p)[5] & 0xff) << 40) |   \
     ((u64)((p)[6] & 0xff) << 48) |   \
     ((u64)((p)[7] & 0xff) << 56))


/* store a 64 bit unsigned integer as eight 8 bit unsigned integers in little endian */
#define U64TO8_LE(p, v)               \
  do {                                \
    (p)[0] = (u8)((v)      ) & 0xff;  \
    (p)[1] = (u8)((v) >>  8) & 0xff;  \
    (p)[2] = (u8)((v) >> 16) & 0xff;  \
    (p)[3] = (u8)((v) >> 24) & 0xff;  \
    (p)[4] = (u8)((v) >> 32) & 0xff;  \
    (p)[5] = (u8)((v) >> 40) & 0xff;  \
    (p)[6] = (u8)((v) >> 48) & 0xff;  \
    (p)[7] = (u8)((v) >> 56) & 0xff;  \
  } while (0)

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#if !defined(GCC_VERSION_AT_LEAST)
# ifdef __GNUC__
#  define GCC_VERSION_AT_LEAST(m, n) \
                (__GNUC__ > (m) || (__GNUC__ == (m) && __GNUC_MINOR__ >= (n)))
# else
#  define GCC_VERSION_AT_LEAST(m, n) 0
# endif
#endif

//! Function attribute used by functions that never return (that terminate the process).
#if !defined(REB_NORETURN)
# if defined(__clang__) || GCC_VERSION_AT_LEAST(2, 5)
#  define REB_NORETURN __attribute__ ((noreturn))
# elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define REB_NORETURN _Noreturn
# elif defined(__TINYC__)
#  define REB_NORETURN  /* _Noreturn unreliable [1] */
# elif defined(_MSC_VER)
#  define REB_NORETURN __declspec(noreturn)
# else
#  define REB_NORETURN
# endif
#endif
#if !defined(DEAD_END)
# if __has_builtin(__builtin_unreachable) || GCC_VERSION_AT_LEAST(4, 5)
#  define DEAD_END __builtin_unreachable()
# elif defined(_MSC_VER)
#  define DEAD_END __assume(0)
# else
#  define DEAD_END abort()
# endif
#endif

/* These macros are easier-to-spot variants of the parentheses cast.
 * The 'm_cast' is when getting [M]utablity on a const is okay (RARELY!)
 * Plain 'cast' can do everything else (except remove volatile)
 * The 'c_cast' helper ensures you're ONLY adding [C]onst to a value
 */
#define m_cast(t,v)     ((t)(v))
#define cast(t,v)       ((t)(v))
#define c_cast(t,v)     ((t)(v))

#include <string.h> // for strlen() etc, but also defines `size_t`
#define strsize strlen
#if defined(NDEBUG)
    /* These [S]tring and [B]inary casts are for "flips" between a 'char *'
     * and 'unsigned char *' (or 'const char *' and 'const unsigned char *').
     * Being single-arity with no type passed in, they are succinct to use:
     */
    #define s_cast(b)       ((char *)(b))
    #define cs_cast(b)      ((const char *)(b))
    #define b_cast(s)       ((REBYTE *)(s))
    #define cb_cast(s)      ((const REBYTE *)(s))

    #define LEN_BYTES(s) \
        (REBLEN)strlen((const char*)(s))

    #define COPY_BYTES(d,s,n) \
        strncpy((char*)(d), (const char*)(s), (n))

    #define CMP_BYTES(l,r) \
        strcmp((const char*)(l), (const char*)(r))

    inline static REBYTE *APPEND_BYTES(
        REBYTE *dest, const REBYTE *src, size_t max
    ){
        size_t len = LEN_BYTES(dest);
        return b_cast(strncat(
            s_cast(dest), cs_cast(src), MAX(max - len - 1, 0)
        ));
    }
#else
    /* We want to ensure the input type is what we thought we were flipping,
     * particularly not the already-flipped type.  Instead of type_traits, 4
     * functions check in both C and C++ (here only during Debug builds):
     */
    inline static REBYTE *b_cast(char *s)
        { return (REBYTE*)s; }

    inline static const REBYTE *cb_cast(const char *s)
        { return (const REBYTE*)s; }

    inline static char *s_cast(REBYTE *s)
        { return (char*)s; }

    inline static const char *cs_cast(const REBYTE *s)
        { return (const char*)s; }

    // Debug build uses inline functions to ensure you pass in unsigned char *
    //
    inline static unsigned char *COPY_BYTES(
        REBYTE *dest, const REBYTE *src, size_t count
    ){
        return b_cast(strncpy(s_cast(dest), cs_cast(src), count));
    }

    inline static REBLEN LEN_BYTES(const REBYTE *str)
        { return (REBLEN)strlen(cs_cast(str)); }

    inline static int CMP_BYTES(
        const REBYTE *lhs, const REBYTE *rhs
    ){
        return strcmp(cs_cast(lhs), cs_cast(rhs));
    }

    inline static REBYTE *APPEND_BYTES(
        REBYTE *dest, const REBYTE *src, size_t max
    ){
        size_t len = LEN_BYTES(dest);
        return b_cast(strncat(
            s_cast(dest), cs_cast(src), MAX(max - len - 1, 0)
        ));
    }
#endif

#endif // REB_C_H


// File: reb-args.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Program startup arguments
**  Module:  reb-args.h
**  Author:  Carl Sassenrath
**  Notes:
**     Arg struct is used by R3 lib, so must not be modified.
**
***********************************************************************/

// REBOL startup option structure:
typedef struct rebol_args {
	REBCNT options;
	REBYTE *script;
	REBYTE *do_arg;
	REBYTE *version;
	REBYTE *debug;
	REBYTE *import;
	REBYTE *secure;
	REBYTE *boot;
	REBYTE *exe_path;
	REBYTE *current_dir;
	REBYTE *args;         // value for the --args option
	REBCNT  argc;         // unprocessed argument count
	REBCHR **argv;        // unprocessed arguments
} REBARGS;

// REBOL arg option flags:
// Must stay matched to system/catalog/boot-flags.
enum arg_opts {
	ROF_EXT,

	ROF_SCRIPT,
	ROF_ARGS,
	ROF_DO,
	ROF_IMPORT,
	ROF_VERSION,
	ROF_DEBUG,
	ROF_SECURE,

	ROF_HELP,
	ROF_VERS,
	ROF_QUIET,
	ROF_VERBOSE,
	ROF_SECURE_MIN,
	ROF_SECURE_MAX,
	ROF_TRACE,
	ROF_HALT,
	ROF_CGI,
	ROF_BOOT,
	ROF_NO_WINDOW,
	ROF_NO_COLOR,
	ROF_LEGACY_REPL,

	ROF_IGNORE, // not an option
};

#define RO_EXT         (1<<ROF_EXT)
#define RO_HELP        (1<<ROF_HELP)
#define RO_IMPORT      (1<<ROF_IMPORT)
#define RO_CGI         (1<<ROF_CGI)
#define RO_ARGS        (1<<ROF_ARGS)
#define RO_DO          (1<<ROF_DO)
#define RO_DEBUG       (1<<ROF_DEBUG)
#define RO_SECURE_MIN  (1<<ROF_SECURE_MIN)
#define RO_SECURE_MAX  (1<<ROF_SECURE_MAX)
#define RO_QUIET       (1<<ROF_QUIET)
#define RO_SCRIPT      (1<<ROF_SCRIPT)
#define RO_SECURE      (1<<ROF_SECURE)
#define RO_TRACE       (1<<ROF_TRACE)
#define RO_VERSION     (1<<ROF_VERSION)
#define RO_VERS        (1<<ROF_VERS)
#define RO_VERBOSE     (1<<ROF_VERBOSE)
#define RO_HALT        (1<<ROF_HALT)
#define RO_BOOT        (1<<ROF_BOOT)
#define RO_NO_WINDOW   (1<<ROF_NO_WINDOW)
#define RO_NO_COLOR    (1<<ROF_NO_COLOR)
#define RO_LEGACY_REPL (1<<ROF_LEGACY_REPL)

#define RO_IGNORE      (1<<ROF_IGNORE)



// File: reb-ext.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Extensions Include File
**  Module:  reb-ext.h
**  Author:  Carl Sassenrath, Oldes
**  Notes:
**
***********************************************************************/

//#include "reb-defs.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Miscellaneous structures and definitions
**  Module:  reb-defs.h
**  Author:  Carl Sassenrath
**  Notes:
**      This file is used by internal and external C code. It
**      should not depend on many other header files prior to it.
**
***********************************************************************/

#ifndef REB_DEFS_H  // due to sequences within the lib build itself
#define REB_DEFS_H

/* These used for access-os native function */
#define OS_ENA    -1
#define OS_EINVAL -2
#define OS_EPERM  -3
#define OS_ESRCH  -4


#ifdef ENDIAN_LITTLE
#define OS_LITTLE_ENDIAN TRUE 
#define WRITE_BE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_BE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_BE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_BE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
							cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#define WRITE_LE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_LE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_LE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_LE_8(cp, bp)  memcpy(cp, bp, 8);
#else
#define OS_LITTLE_ENDIAN FALSE 
#define WRITE_BE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_BE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_BE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_BE_8(cp, bp)  memcpy(cp, bp, 8);
#define WRITE_LE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_LE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_LE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_LE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
							cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#endif


#pragma pack(4)

// Standard date and time:
typedef struct rebol_dat {
	int year;
	int month;
	int day;
	int time;
	int nano;
	int zone;
} REBOL_DAT;  // not same as REBDAT

typedef int	cmp_t(const void *, const void *);
void unstable_sort(void* a, size_t n, size_t es, cmp_t* cmp);
void   stable_sort(void* a, size_t n, size_t es, cmp_t* cmp);
#define SORT_FLAG_REVERSE 1
#define SORT_FLAG_WIDE    2
#define SORT_FLAG_CASE    3
#define SORT_FLAG_ALL     4
#define SORT_FLAG_BINARY  5 // used with the custom sort function


// Encoding_opts was originally in sys-core.h, but I moved it here so it can
// be used also while makking external extensions. (oldes)

// Encoding options:
enum encoding_opts {
	ENC_OPT_BIG,		// big endian (not little)
	ENC_OPT_UTF8,		// UTF-8
	ENC_OPT_UTF16,		// UTF-16
	ENC_OPT_UTF32,		// UTF-32
	ENC_OPT_BOM,		// byte order marker
	ENC_OPT_CRLF,		// CR line termination
	ENC_OPT_NO_COPY,	// do not copy if ASCII
};

#define ENCF_NO_COPY (1<<ENC_OPT_NO_COPY)
#if OS_CRLF
#define ENCF_OS_CRLF (1<<ENC_OPT_CRLF)
#else
#define ENCF_OS_CRLF 0
#endif

#pragma pack()

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12
#define IS_SURROGATE(c) (c >= 0xD800 && c <= 0xDFFF)
#define IS_INVALID_CHAR(c) (c == UNKNOWN || c > MAX_UNI || IS_SURROGATE(c))

#endif

//#include "ext-types.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: Extension Types (Isolators)
**  Build: 3.22.8
**  Date:  21-Sep-2026
**  File:  ext-types.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


enum REBOL_Ext_Types
{
	RXT_END = 0,                  // 0
	RXT_UNSET,                    // 1
	RXT_NONE,                     // 2
	RXT_HANDLE,                   // 3
	RXT_LOGIC = 4,                // 4
	RXT_INTEGER,                  // 5
	RXT_DECIMAL,                  // 6
	RXT_PERCENT,                  // 7
	RXT_CHAR = 10,                // 8
	RXT_PAIR,                     // 9
	RXT_TUPLE,                    // 10
	RXT_TIME,                     // 11
	RXT_DATE,                     // 12
	RXT_WORD = 16,                // 13
	RXT_SET_WORD,                 // 14
	RXT_GET_WORD,                 // 15
	RXT_LIT_WORD,                 // 16
	RXT_REFINEMENT,               // 17
	RXT_ISSUE,                    // 18
	RXT_STRING = 24,              // 19
	RXT_FILE,                     // 20
	RXT_EMAIL,                    // 21
	RXT_REF,                      // 22
	RXT_URL,                      // 23
	RXT_TAG,                      // 24
	RXT_BLOCK = 32,               // 25
	RXT_PAREN,                    // 26
	RXT_PATH,                     // 27
	RXT_SET_PATH,                 // 28
	RXT_GET_PATH,                 // 29
	RXT_LIT_PATH,                 // 30
	RXT_BINARY = 40,              // 31
	RXT_BITSET,                   // 32
	RXT_VECTOR,                   // 33
	RXT_IMAGE,                    // 34
	RXT_GOB = 47,                 // 35
	RXT_OBJECT = 48,              // 36
	RXT_MODULE,                   // 37
	RXT_PORT,                     // 38
	RXT_STRUCT = 54,              // 39
	RXT_EVENT,                    // 40
    RXT_MAX
};

//#include "sys-value.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Value and Related Definitions
**  Module:  sys-value.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#ifndef VALUE_H
#define VALUE_H

/***********************************************************************
**
**	REBOL Value Type
**
**		This is used for all REBOL values. This is a forward
**		declaration. See end of this file for actual structure.
**
***********************************************************************/

#pragma pack(4)

typedef struct Reb_Header {
#ifdef ENDIAN_LITTLE
	unsigned type:8;	// datatype
	unsigned opts:8;	// special options
	unsigned exts:8;	// extensions to datatype
	unsigned resv:8;	// reserved for future
#else
	unsigned resv:8;	// reserved for future
	unsigned exts:8;	// extensions to datatype
	unsigned opts:8;	// special options
	unsigned type:8;	// datatype
#endif
} REBHED;

struct Reb_Value;
typedef struct Reb_Value REBVAL;
typedef struct Reb_Series REBSER;
typedef struct Reb_Handle_Context REBHOB;
typedef union rxi_arg_val RXIARG;

// Value type identifier (generally, should be handled as integer):

#define VAL_TYPE(v)		((v)->flags.flags.type)			// get only type, not flags
#define SET_TYPE(v,t)	((v)->flags.flags.type = (t))	// set only type, not flags
#define VAL_SET(v,t)	((v)->flags.header = (t))		// set type, clear all flags
// Note: b-init.c verifies that lower 8 bits of header = flags.type

// Clear type identifier:
#define SET_END(v)			VAL_SET(v, 0)

// Value option flags:
enum {
	OPTS_LINE = 0,	// Line break occurs before this value
	OPTS_LOCK,		// Lock word from modification
	OPTS_REVAL,		// Reevaluate result value
	OPTS_UNWORD,	// Not a normal word
	OPTS_TEMP,		// Temporary flag - variety of uses
	OPTS_HIDE,		// Hide the word
};

#define VAL_OPTS(v)			((v)->flags.flags.opts)
#define VAL_SET_OPT(v,n)	SET_FLAG(VAL_OPTS(v), n)
#define VAL_GET_OPT(v,n)	GET_FLAG(VAL_OPTS(v), n)
#define VAL_CLR_OPT(v,n)	CLR_FLAG(VAL_OPTS(v), n)

#define VAL_GET_LINE(v)		VAL_GET_OPT((v), OPTS_LINE)
#define VAL_SET_LINE(v)		VAL_SET_OPT((v), OPTS_LINE)
#define VAL_CLR_LINE(v)		VAL_CLR_OPT((v), OPTS_LINE)

#define VAL_PROTECTED(v)	VAL_GET_OPT((v), OPTS_LOCK)
#define VAL_HIDDEN(v)	    VAL_GET_OPT((v), OPTS_HIDE)

// Used for datatype-dependent data (e.g. op! stores action!)
#define VAL_GET_EXT(v)		((v)->flags.flags.exts)
#define VAL_SET_EXT(v,n)	((v)->flags.flags.exts = (n))

#define	IS_SET(v)			(VAL_TYPE(v) > REB_UNSET)
#define IS_SCALAR(v)		(VAL_TYPE(v) <= REB_DATE)

// When key is removed from map, it has OPTS_HIDE flag
#define VAL_MAP_REMOVED(val) (VAL_GET_OPT(val, OPTS_HIDE)) 


/***********************************************************************
**
**	DATATYPE - Datatype or pseudo-datatype
**
***********************************************************************/

typedef struct Reb_Type {
	REBINT	type;	// base type
	REBSER  *spec;
//	REBINT	min_type;
//	REBINT	max_type;
} REBTYP;

#define	VAL_DATATYPE(v)		((v)->data.datatype.type)
#define	VAL_TYPE_SPEC(v)	((v)->data.datatype.spec)

//#define	VAL_MIN_TYPE(v)	((v)->data.datatype.min_type)
//#define	VAL_MAX_TYPE(v)	((v)->data.datatype.max_type)
#define	IS_OF_DATATYPE(v,t) (IS_DATATYPE(v) && (VAL_DATATYPE(v) == (t)))
#define	NO_TYPE (0)


/***********************************************************************
**
**	NUMBERS - Integer and other simple scalars
**
***********************************************************************/

#define	SET_UNSET(v)	VAL_SET(v, REB_UNSET)

#define	SET_NONE(v)		VAL_SET(v, REB_NONE)
#define NONE_VALUE		ROOT_NONEVAL

#define VAL_INT32(v)	(REBINT)((v)->data.integer)
#define VAL_INT64(v)    ((v)->data.integer)
#define VAL_UNT32(v)    (REBCNT)((v)->data.integer)
#define VAL_UNT64(v)	((v)->data.unteger)
#define	SET_INTEGER(v,n) VAL_SET(v, REB_INTEGER), ((v)->data.integer) = (n)
#define	SET_INT32(v,n)  ((v)->data.integer) = (REBINT)(n)

#define MAX_CHAR		0x0010FFFF
#define VAL_CHAR(v)		((v)->data.uchar)
#define	SET_CHAR(v,n)	VAL_SET(v, REB_CHAR), VAL_CHAR(v) = (REBINT)(n)

#define IS_NUMBER(v)	(VAL_TYPE(v) == REB_INTEGER || VAL_TYPE(v) == REB_DECIMAL)
#define	AS_INT32(v)     (IS_INTEGER(v) ? VAL_INT32(v) : (REBINT)VAL_DECIMAL(v))
#define	AS_INT64(v)     (IS_INTEGER(v) ? VAL_INT64(v) : (REBI64)VAL_DECIMAL(v))


/***********************************************************************
**
**	DECIMAL, MONEY -- Includes denomination and amount
**
***********************************************************************/

#pragma pack()
//#include "sys-deci.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Deci Datatype
**  Module:  sys-deci.h
**  Notes:
**
***********************************************************************/

typedef struct deci {
    unsigned m0:32;	/* significand, lowest part */
	unsigned m1:32;	/* significand, continuation */
    unsigned m2:23;	/* significand, highest part */
	unsigned s:1;	/* sign, 0 means nonnegative, 1 means nonpositive */
	int e:8;		/* exponent */
} deci;


#pragma pack(4)

#define VAL_DECIMAL(v)	((v)->data.decimal)
#define	SET_DECIMAL(v,n) VAL_SET(v, REB_DECIMAL), VAL_DECIMAL(v) = (n)
#define	SET_PERCENT(v,n) VAL_SET(v, REB_PERCENT), VAL_DECIMAL(v) = (n)
#define	AS_DECIMAL(v) (IS_INTEGER(v) ? (REBDEC)VAL_INT64(v) : VAL_DECIMAL(v))

typedef deci REBDCI;
#define VAL_DECI(v)		((v)->data.deci)
#define	SET_MONEY(v,n) VAL_SET(v, REB_MONEY), VAL_DECI(v) = (n)

#ifdef not_used
typedef struct Reb_Decimal {
	REBDEC number;
	REBYTE denom[4];
} REBMNY;
#define VAL_MONEY(v)	((v)->data.money)
#define	VAL_MONEY_DENOM(v)	//((v)->data.money.denom)
#define VAL_MONEY_AMOUNT(v)	((v)->data.money.amount)
#endif


/***********************************************************************
**
**	DATE and TIME
**
***********************************************************************/

typedef struct reb_ymdz {
#ifdef ENDIAN_LITTLE
	REBINT zone:7;  // +/-15:00 res: 0:15
	REBCNT day:5;
	REBCNT month:4;
	REBCNT year:16;
#else
	REBCNT year:16;
	REBCNT month:4;
	REBCNT day:5;
	REBINT zone:7;  // +/-15:00 res: 0:15
#endif
} REBYMD;

typedef union reb_date {
	REBYMD date;
	REBCNT bits;
} REBDAT;

typedef struct Reb_Time {
	REBI64 time;	// time in nanoseconds
	REBDAT date;
} REBTIM;

#define VAL_TIME(v)	((v)->data.time.time)
#define TIME_SEC(n)	((REBI64)(n) * 1000000000L)

#define MAX_SECONDS	((REBI64)9223372036L) //((i64)((2 ** 63) / (10 ** 9)))
#define MAX_HOUR	(MAX_SECONDS / 3600)
#define MAX_TIME	((REBI64)MAX_HOUR * HR_SEC)

#define NANO		1.0e-9
#define	SEC_SEC		((REBI64)1000000000L)
#define MIN_SEC		(60 * SEC_SEC)
#define HR_SEC		(60 * 60 * SEC_SEC)

#define SEC_TIME(n)  ((n) * SEC_SEC)
#define MIN_TIME(n)  ((n) * MIN_SEC)
#define HOUR_TIME(n) ((n) * HR_SEC)

#define SECS_IN(n) ((n) / SEC_SEC)
#define VAL_SECS(n) (VAL_TIME(n) / SEC_SEC)

#define DEC_TO_SECS(n) (i64)(((n) + 5.0e-10) * SEC_SEC)

#define SECS_IN_DAY 86400
#define MICROSECONDS_IN_DAY 86400000000
#define TIME_IN_DAY (SEC_TIME((i64)SECS_IN_DAY))

#define NO_TIME		MIN_I64

#define	MAX_YEAR		0x3fff

#define VAL_DATE(v)		((v)->data.time.date)
#define VAL_YEAR(v)		((v)->data.time.date.date.year)
#define VAL_MONTH(v)	((v)->data.time.date.date.month)
#define VAL_DAY(v)		((v)->data.time.date.date.day)
#define VAL_ZONE(v)		((v)->data.time.date.date.zone)

#define ZONE_MINS 15
#define ZONE_SECS (ZONE_MINS*60)
#define MAX_ZONE (15 * (60/ZONE_MINS))


/***********************************************************************
**
**	TUPLE
**
***********************************************************************/

typedef struct Reb_Tuple {
	REBYTE tuple[12];
} REBTUP;

#define	VAL_TUPLE(v)	((v)->data.tuple.tuple)
#define	VAL_TUPLE_LEN(v) (VAL_GET_EXT(v))
#define MAX_TUPLE 12


/***********************************************************************
**
**	PAIR
**
***********************************************************************/

// X/Y coordinate pair as floats:
typedef struct rebol_xy_float {
	float x;
	float y;
} REBXYF;

// X/Y coordinate pair as integers:
typedef struct rebol_xy_int {
	int x;
	int y;
} REBXYI;

#define REBPAR REBXYI  // temporary until all sources are converted

#define	VAL_PAIR(v)		((v)->data.pair)
#define	VAL_PAIR_X(v)	((v)->data.pair.x)
#define	VAL_PAIR_Y(v) 	((v)->data.pair.y)
#define SET_PAIR(v,x,y)	(VAL_SET(v, REB_PAIR),VAL_PAIR_X(v)=(x),VAL_PAIR_Y(v)=(y))
#define	VAL_PAIR_X_INT(v) ROUND_TO_INT((v)->data.pair.x)
#define	VAL_PAIR_Y_INT(v) ROUND_TO_INT((v)->data.pair.y)


/***********************************************************************
**
**	EVENT
**
***********************************************************************/

#pragma pack()
//#include "reb-event.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: REBOL event definitions
**  Module:  reb-event.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#ifndef REB_EVENT_H
#define REB_EVENT_H

// Note: size must be 12 bytes on 32-bit or 16 on 64-bit!

// Forward references:
#ifndef VALUE_H
typedef struct Reb_Series   REBSER;
typedef struct Reb_Handle_Context REBHOB;
#endif
#ifndef DEVICE_H
typedef struct rebol_device REBDEV;
typedef struct rebol_devreq REBREQ;
#endif

#pragma pack(4)
typedef struct rebol_event {
	u8  type;		// event id (mouse-move, mouse-button, etc)
	u8  flags;		// special flags
	u8  win;		// reserved (was the View window table index; the
					// handle now carries that identity)
	u8  model;		// port, object, gui, callback
	u32 data;		// an x/y position or keycode (raw/decoded)
	union {
		REBREQ *req;  // request (for device events)
		REBSER *port; // port
		REBHOB *hob;  // handle context (widget, window, ...)
		void   *ser;  // object
	};
} REBEVT;
#pragma pack()

// Special event flags:

enum {
	EVF_COPIED,		// event data has been copied
	EVF_HAS_XY,		// map-event will work on it
	EVF_DOUBLE,		// double click detected
	EVF_CONTROL,
	EVF_SHIFT,
	EVF_HAS_SYM,    // `data` is a canon symbol id; `code` reads as a word
	EVF_HAS_CODE,   // XY value is interpreted as integer instead of pair
	EVF_ALT,
};


// Event port data model

enum {
	EVM_DEVICE,		// I/O request holds the port pointer
	EVM_PORT,		// event holds port pointer
	EVM_OBJECT,		// event holds object frame pointer
	EVM_HANDLE,		// event holds a context handle (REBHOB)
	EVM_CALLBACK,	// Callback event uses system/ports/callback port
	EVM_MIDI,		// event holds midi port pointer
	EVM_CONSOLE,    // native console events
};

// Special messages
#define WM_DNS (WM_USER+100)

#endif

#pragma pack(4)

#define	VAL_EVENT_TYPE(v)	((v)->data.event.type)  //(VAL_EVENT_INFO(v) & 0xff)
#define	VAL_EVENT_FLAGS(v)	((v)->data.event.flags) //((VAL_EVENT_INFO(v) >> 16) & 0xff)
#define	VAL_EVENT_WIN(v)	((v)->data.event.win)   //((VAL_EVENT_INFO(v) >> 24) & 0xff)
#define	VAL_EVENT_MODEL(v)	((v)->data.event.model)
#define	VAL_EVENT_DATA(v)	((v)->data.event.data)
#define	VAL_EVENT_TIME(v)	((v)->data.event.time)
#define	VAL_EVENT_REQ(v)	((v)->data.event.req)
#define	VAL_EVENT_SER(v)	((v)->data.event.ser)
#define	VAL_EVENT_HOB(v)	((v)->data.event.hob)

#define IS_EVENT_MODEL(v,f)	(VAL_EVENT_MODEL(v) == (f))

#define SET_EVENT_INFO(val, type, flags, win) \
	VAL_EVENT_TYPE(val)=type, VAL_EVENT_FLAGS(val)=flags, VAL_EVENT_WIN(val)=win
	//VAL_EVENT_INFO(val) = (type | (flags << 16) | (win << 24))

#define	VAL_EVENT_X(v)		((REBINT) (short) (VAL_EVENT_DATA(v) & 0xffff))
#define	VAL_EVENT_Y(v) 		((REBINT) (short) ((VAL_EVENT_DATA(v) >> 16) & 0xffff))
#define VAL_EVENT_XY(v)		(VAL_EVENT_DATA(v))
#define SET_EVENT_XY(v,x,y) VAL_EVENT_DATA(v) = ((y << 16) | (x & 0xffff))

#define	VAL_EVENT_KEY(v)	(VAL_EVENT_DATA(v) & 0xffff)
#define	VAL_EVENT_KCODE(v)	((VAL_EVENT_DATA(v) >> 16) & 0xffff)
#define SET_EVENT_KEY(v,k,c) VAL_EVENT_DATA(v) = ((c << 16) + k)

#define	IS_KEY_EVENT(type)	0

#ifdef old_code
#define	TO_EVENT_XY(x,y)	(((y)<<16)|((x)&0xffff))
#define	SET_EVENT_INFO(v,t,k,c,w,f)	((VAL_FLAGS(v)=(VAL_FLAGS(v)&0x0f)|((f)&0xf0)),\
									(VAL_EVENT_INFO(v)=(((t)&0xff)|(((k)&0xff)<<8)|\
									(((c)&0xff)<<16)|(((w)&0xff)<<24))))
#endif

/***********************************************************************
**
*/	struct Reb_Series
/*
**		Series header points to data and keeps track of tail and size.
**		Additional fields can be used for attributes and GC. Every
**		string and block in REBOL uses one of these to permit GC
**		and compaction.
**
***********************************************************************/
{
	REBYTE	*data;		// series data head
	REBLEN	tail;		// one past end of useful data
	REBLEN	rest;		// total number of units from bias to end
	REBINT  sizes;      // 16 bits bias, 8 bits reserved, 8 bits wide!
	REBCNT  flags;
	union {
		REBCNT size;	// used for vectors and bitsets
		REBSER *series;	// used by MAP (hashes) and by STRUCT (see below)
		struct {
			REBCNT wide:16;
			REBCNT high:16;
		} area;
		REBUPT all; /* for copying, must have the same size as the union */
	};
#ifdef SERIES_LABELS
	REBYTE  *label;		// identify the series
#endif
};

#define SERIES_TAIL(s)	 ((s)->tail)
#define SERIES_REST(s)	 ((s)->rest)
#define	SERIES_LEN(s)    ((s)->tail + 1) // Includes terminator
#define	SERIES_SIZES(s)  ((s)->sizes)
#define	SERIES_FLAGS(s)	 ((s)->flags)
#define	SERIES_WIDE(s)	 (((s)->sizes) & 0xff)
#define SERIES_DATA(s)   ((s)->data)
#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))
#define	SERIES_SKIP(s,i) (SERIES_DATA(s) + (SERIES_WIDE(s) * (i)))

#define END_FLAG 0x80000000  // Indicates end of block as an index (from DO_NEXT)

#ifdef SERIES_LABELS
#define SERIES_LABEL(s)  ((s)->label)
#define SET_SERIES_LABEL(s,l) (((s)->label) = (l))
#else
#define SERIES_LABEL(s)  "-"
#define SET_SERIES_LABEL(s,l)
#endif

// Flag: If wide field is not set, series is free (not used):
#define	SERIES_FREED(s)  (!SERIES_WIDE(s))

// Bias is empty space in front of head:
#define	SERIES_BIAS(s)	   (REBCNT)((SERIES_SIZES(s) >> 16) & 0xffff)
#define MAX_SERIES_BIAS    0x1000
#define SERIES_SET_BIAS(s,b) (SERIES_SIZES(s) = (SERIES_SIZES(s) & 0xffff) | (b << 16))
#define SERIES_ADD_BIAS(s,b) (SERIES_SIZES(s) += (b << 16))
#define SERIES_SUB_BIAS(s,b) (SERIES_SIZES(s) -= (b << 16))

// Size in bytes of memory allocated (including bias area):
#define SERIES_TOTAL(s) ((SERIES_REST(s) + SERIES_BIAS(s)) * (REBCNT)SERIES_WIDE(s))
// Size in bytes of series (not including bias area):
#define	SERIES_SPACE(s) (SERIES_REST(s) * (REBCNT)SERIES_WIDE(s))
// Size in bytes being used, including terminator:
#define SERIES_USED(s) (SERIES_LEN(s) * SERIES_WIDE(s))

// Optimized expand when at tail (but, does not reterminate)
#define EXPAND_SERIES_TAIL(s,l) if (SERIES_FITS(s, l)) s->tail += l; else Expand_Series(s, AT_TAIL, l)
#define RESIZE_SERIES(s,l) s->tail = 0; if (!SERIES_FITS(s, l)) Expand_Series(s, AT_TAIL, l); s->tail = 0
#define RESET_TAIL(s) s->tail = 0; SERIES_CLR_FLAG(s, SER_UTF8);
#define RESET_SERIES(s) RESET_TAIL(s); TERM_SERIES(s);

// Clear all and clear to tail:
#define CLEAR_SERIES(s) CLEAR(SERIES_DATA(s), SERIES_SPACE(s))
#define ZERO_SERIES(s) memset(SERIES_DATA(s), 0, SERIES_USED(s))
#define TERM_SERIES(s) memset(SERIES_SKIP(s, SERIES_TAIL(s)), 0, SERIES_WIDE(s))

// Returns space that a series has available (less terminator):
#define SERIES_FULL(s) (SERIES_LEN(s) >= SERIES_REST(s))
#define SERIES_AVAIL(s) (SERIES_REST(s) - SERIES_LEN(s))
#define SERIES_FITS(s,n) ((SERIES_TAIL(s) + (REBCNT)(n) + 1) < SERIES_REST(s))

// Flag used for extending series at tail:
#define	AT_TAIL	((REBCNT)(~0))	// Extend series at tail

// Is it a byte-sized series? (this works because no other odd size allowed)
#ifdef BYTE_SIZE // macOS SDK used the same name
#undef BYTE_SIZE
#endif
#define BYTE_SIZE(s) (SERIES_SIZES(s) & 1)
#define VAL_BYTE_SIZE(v) (BYTE_SIZE(VAL_SERIES(v)))
#define VAL_STR_IS_ASCII(v) (VAL_BYTE_SIZE(v) && Is_ASCII(VAL_BIN_DATA(v), VAL_LEN(v)))

// Series Flags (max32):
enum {
	SER_MARK = 1,		// Series was found during GC mark scan.
	SER_KEEP = 1<<1,	// Series is permanent, do not GC it.
	SER_LOCK = 1<<2,	// Series is locked, do not expand it
	SER_EXT  = 1<<3,	// Series data is external (library), do not GC it.
	SER_FREE = 1<<4,	// mark series as removed
	SER_BARE = 1<<5,	// Series has no links to GC-able values
	SER_PROT = 1<<6,	// Series is protected from modification
	SER_MON  = 1<<7,	// Monitoring
	SER_INT  = 1<<8,	// Series data is internal (loop frames) and should not be accessed by users
	SER_UTF8 = 1<<9,	// Series contains not only ASCII characters
	SER_SLICE= 1<<10,	// Series is sliced (side link contains the original)
	SER_SIZEP= 1<<11,   // Series is protected from length modification only (e.g. sliced series)
};

#define SERIES_SET_FLAG(s, f) (SERIES_FLAGS(s) |=  (f))
#define SERIES_CLR_FLAG(s, f) (SERIES_FLAGS(s) &= ~(f))
#define SERIES_GET_FLAG(s, f) (SERIES_FLAGS(s) &   (f))

#define	IS_FREEABLE(s)    !SERIES_GET_FLAG(s, SER_MARK|SER_KEEP|SER_FREE)
#define MARK_SERIES(s)    SERIES_SET_FLAG(s, SER_MARK)
#define UNMARK_SERIES(s)  SERIES_CLR_FLAG(s, SER_MARK)
#define IS_MARK_SERIES(s) SERIES_GET_FLAG(s, SER_MARK)
#define KEEP_SERIES(s,l)  do {SERIES_SET_FLAG(s, SER_KEEP); LABEL_SERIES(s,l);} while(0)
#define EXT_SERIES(s)     SERIES_SET_FLAG(s, SER_EXT)
#define IS_EXT_SERIES(s)  SERIES_GET_FLAG(s, SER_EXT)
#define INT_SERIES(s)     SERIES_SET_FLAG(s, SER_INT)
#define IS_INT_SERIES(s)  SERIES_GET_FLAG(s, SER_INT)
#define LOCK_SERIES(s)    SERIES_SET_FLAG(s, SER_LOCK)
#define IS_LOCK_SERIES(s) SERIES_GET_FLAG(s, SER_LOCK)
#define BARE_SERIES(s)    SERIES_SET_FLAG(s, SER_BARE)
#define IS_BARE_SERIES(s) SERIES_GET_FLAG(s, SER_BARE)
#define PROTECT_SERIES(s) SERIES_SET_FLAG(s, SER_PROT)
#define UNPROTECT_SERIES(s)  SERIES_CLR_FLAG(s, SER_PROT)
#define IS_PROTECT_SERIES(s) SERIES_GET_FLAG(s, SER_PROT)
#define UTF8_SERIES(s)       SERIES_SET_FLAG(s, SER_UTF8)
#define IS_UTF8_SERIES(s)    SERIES_GET_FLAG(s, SER_UTF8)
#define IS_UTF8_STRING(v)    SERIES_GET_FLAG(VAL_SERIES(v), SER_UTF8)
#define SLICE_SERIES(s)      SERIES_SET_FLAG(s, SER_EXT|SER_SLICE|SER_SIZEP)
#define IS_SLICE_SERIES(s)   SERIES_GET_FLAG(s, SER_SLICE)
#define IS_FIXED_SIZE(s)     SERIES_GET_FLAG(s, SER_LOCK|SER_SIZEP)
#define IS_FIXED_SIZE_VALUE(v) SERIES_GET_FLAG(VAL_SERIES(v), SER_LOCK|SER_SIZEP)

#define TRAP_PROTECT(s) if (IS_PROTECT_SERIES(s)) Trap0(RE_PROTECTED)
#define TRAP_FIXED_SIZE(s) if (IS_FIXED_SIZE(s)) Trap0(RE_FIXED_SIZED_SERIES)

#ifdef SERIES_LABELS
#define LABEL_SERIES(s,l) s->label = (l)
#else
#define LABEL_SERIES(s,l)
#endif

#ifdef MEM_STRESS
#define FREE_SERIES(s)    SERIES_SET_FLAG(s, SER_FREE) // mark as removed
#define	CHECK_MARK(s,d) \
		if (SERIES_GET_FLAG(s, SER_FREE)) Choke(); \
		if (!IS_MARK_SERIES(s)) Mark_Series(s, d);
#else
#define FREE_SERIES(s)
#define	CHECK_MARK(s,d) if (!IS_MARK_SERIES(s)) Mark_Series(s, d);
#endif

// Using the mark queue only if we are deep enough
#define	QUEUE_CHECK_MARK(s,d) \
		if (!IS_MARK_SERIES(s)) {\
			if (depth < 64) Mark_Series(s, d); \
			else Queue_Mark_Series(s);\
		}

//#define LABEL_SERIES(s,l) s->label = (l)
#define IS_BLOCK_SERIES(s) (SERIES_WIDE(s) == sizeof(REBVAL))

// !!! Remove if not used after port:
//#define	SERIES_SIDE(s)	 ((s)->link.side)
//#define	SERIES_FRAME(s)	 ((s)->link.frame)
//#define SERIES_NOT_REBOLS(s) SERIES_SET_FLAG(s, SER_XLIB)


/***********************************************************************
**
**	SERIES -- Generic series macros
**
***********************************************************************/

#pragma pack()
//#include "reb-gob.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Graphical compositing objects
**  Module:  reb-gob.h
**  Author:  Carl Sassenrath
**  Description:
**		GOBs are lower-level graphics object used by the compositing
**		and rendering system. Because a GUI can contain thousands of
**		GOBs, they are designed and structured to be simple and small.
**		Note that GOBs are also used for windowing.
**	Warning:
**		GOBs are allocated from a special pool and
**		are accounted for by the standard garbage collector.
**
***********************************************************************/
#ifndef REB_GOB_H
#define REB_GOB_H

//#include "reb-view.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Additional code modifications and improvements Copyright 2012 Saphirion AG
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Title: Misc View related shared definitions
**  Author: Richard Smolak 
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

extern REBXYF log_size;
extern REBXYF phys_size;

#define LOG_COORD_X(p)	((p) * log_size.x)
#define LOG_COORD_Y(p)	((p) * log_size.y)
#define PHYS_COORD_X(p)	((p) * phys_size.x)
#define PHYS_COORD_Y(p)	((p) * phys_size.y)

typedef enum {
	SM_SCREEN_NUM,
	SM_SCREEN_WIDTH,
	SM_SCREEN_HEIGHT,
	SM_SCREEN_X,
	SM_SCREEN_Y,
	SM_VIRTUAL_SCREEN_WIDTH,
	SM_VIRTUAL_SCREEN_HEIGHT,
	SM_WORK_WIDTH,
	SM_WORK_HEIGHT,
	SM_TITLE_HEIGHT,
	SM_SCREEN_DPI_X,
	SM_SCREEN_DPI_Y,
	SM_BORDER_WIDTH,
	SM_BORDER_HEIGHT,
	SM_BORDER_FIXED_WIDTH,
	SM_BORDER_FIXED_HEIGHT,
	SM_WINDOW_MIN_WIDTH,
	SM_WINDOW_MIN_HEIGHT,
	SM_WORK_X,
	SM_WORK_Y
} METRIC_TYPE;


enum GOB_FLAGS {		// GOB attribute and option flags
	GOBF_TOP = 0,		// Top level (window or output image)
	GOBF_WINDOW,		// Window (parent is OS window reference)
	GOBF_OPAQUE,		// Has no alpha
	GOBF_STATIC,		// Does not change
	GOBF_HIDDEN,		// Is hidden (e.g. hidden window)
	GOBF_RESIZE,		// Can be resized
	GOBF_NO_TITLE,		// Has window title
	GOBF_NO_BORDER,		// Has window border
	GOBF_DROPABLE,		// Let window receive drag and drop
	GOBF_TRANSPARENT,	// Window is in transparent mode
	GOBF_POPUP,			// Window is a popup (with owner window)
	GOBF_MODAL,			// Modal event filtering
	GOBF_ON_TOP,		// The window is always on top
	GOBF_ACTIVE,		// Window is active
	GOBF_MINIMIZE,		// Window is minimized
	GOBF_MAXIMIZE,		// Window is maximized
	GOBF_RESTORE,		// Window is restored
	GOBF_FULLSCREEN,	// Window is fullscreen
};

enum GOB_STATE {		// GOB state flags
	GOBS_OPEN = 0,		// Window is open
	GOBS_ACTIVE,		// Window is active
	GOBS_NEW,			// Gob is new to pane (old-offset, old-size wrong)
};

enum GOB_TYPES {		// Types of content
	GOBT_NONE = 0,
	GOBT_COLOR,
	GOBT_IMAGE,
	GOBT_WIDGET,       // must be between IMAGE and STRING so GC can mark its content!
	GOBT_STRING,
	GOBT_DRAW,
	GOBT_TEXT,
	GOBT_EFFECT,
	GOBT_MAX
};

enum GOB_DTYPES {		// Userdata types
	GOBD_NONE = 0,
	GOBD_OBJECT,
	GOBD_BLOCK,
	GOBD_STRING,
	GOBD_BINARY,
	GOBD_RESV,			// unicode
	GOBD_INTEGER
};

#pragma pack(4)

typedef struct rebol_gob REBGOB;

struct rebol_gob {		// size: 64 bytes!
	REBCNT flags;		// option flags
	REBCNT state;		// state flags

	REBSER *pane;		// List of child GOBs
	REBGOB *parent;		// Parent GOB (or window ptr)

	REBYTE alpha;		// transparency
	REBYTE ctype;		// content data type
	REBYTE dtype;		// pointer data type
	REBYTE resv;		// reserved

	union {
		REBGOB *owner;	// temp field - reused for different things
	};

	REBSER *content;	// content value (block, string, color)
	REBSER *data;		// user defined data

	REBXYF offset;		// location
	REBXYF size;
	REBXYF old_offset;	// prior location
	REBXYF old_size;	// prior size
};
#pragma pack()

typedef struct gob_window {				// Maps gob to window
	REBGOB *gob;
	void* win;
	void* compositor;
} REBGOBWINDOWS;

#define GOB_X(g)		((g)->offset.x)
#define GOB_Y(g)		((g)->offset.y)
#define GOB_W(g)		((g)->size.x)
#define GOB_H(g)		((g)->size.y)

#define GOB_LOG_X(g)		(LOG_COORD_X((g)->offset.x))
#define GOB_LOG_Y(g)		(LOG_COORD_Y((g)->offset.y))
#define GOB_LOG_W(g)		(LOG_COORD_X((g)->size.x))
#define GOB_LOG_H(g)		(LOG_COORD_Y((g)->size.y))

#define GOB_X_INT(g)	ROUND_TO_INT((g)->offset.x)
#define GOB_Y_INT(g)	ROUND_TO_INT((g)->offset.y)
#define GOB_W_INT(g)	ROUND_TO_INT((g)->size.x)
#define GOB_H_INT(g)	ROUND_TO_INT((g)->size.y)

#define GOB_LOG_X_INT(g)	ROUND_TO_INT(LOG_COORD_X((g)->offset.x))
#define GOB_LOG_Y_INT(g)	ROUND_TO_INT(LOG_COORD_Y((g)->offset.y))
#define GOB_LOG_W_INT(g)	ROUND_TO_INT(LOG_COORD_X((g)->size.x))
#define GOB_LOG_H_INT(g)	ROUND_TO_INT(LOG_COORD_Y((g)->size.y))

#define GOB_XO(g)		((g)->old_offset.x)
#define GOB_YO(g)		((g)->old_offset.y)
#define GOB_WO(g)		((g)->old_size.x)
#define GOB_HO(g)		((g)->old_size.y)
#define GOB_XO_INT(g)	ROUND_TO_INT((g)->old_offset.x)
#define GOB_YO_INT(g)	ROUND_TO_INT((g)->old_offset.y)
#define GOB_WO_INT(g)	ROUND_TO_INT((g)->old_size.x)
#define GOB_HO_INT(g)	ROUND_TO_INT((g)->old_size.y)

#define CLEAR_GOB_STATE(g) ((g)->state = 0)

#define SET_GOB_FLAG(g,f)       SET_FLAG((g)->flags, f)
#define GET_GOB_FLAG(g,f)       GET_FLAG((g)->flags, f)
#define CLR_GOB_FLAG(g,f)       CLR_FLAG((g)->flags, f)
#define CLR_GOB_FLAGS(g,f,h)    CLR_FLAGS((g)->flags, f, h)
#define SET_GOB_STATE(g,f)      SET_FLAG((g)->state, f)
#define GET_GOB_STATE(g,f)      GET_FLAG((g)->state, f)
#define CLR_GOB_STATE(g,f)      CLR_FLAG((g)->state, f)
#define CLR_GOB_STATES(g,f,h)   CLR_FLAGS((g)->state, f, h)

#define GOB_ALPHA(g)		((g)->alpha)
#define GOB_TYPE(g)			((g)->ctype)
#define SET_GOB_TYPE(g,t)	((g)->ctype = (t))
#define GOB_DTYPE(g)		((g)->dtype)
#define SET_GOB_DTYPE(g,t)	((g)->dtype = (t))
#define GOB_DATA(g)			((g)->data)
#define SET_GOB_DATA(g,v)	((g)->data = (v))
#define GOB_TMP_OWNER(g)	((g)->owner)

#define IS_GOB_OPAQUE(g)  GET_GOB_FLAG(g, GOBF_OPAQUE)
#define SET_GOB_OPAQUE(g) SET_GOB_FLAG(g, GOBF_OPAQUE)
#define CLR_GOB_OPAQUE(g) CLR_GOB_FLAG(g, GOBF_OPAQUE)

#define GOB_PANE(g)		((g)->pane)
#define GOB_PARENT(g)	((g)->parent)
#define GOB_CONTENT(g)	((g)->content)

#define GOB_WIDGET_HANDLE(g) (BLK_HEAD(GOB_CONTENT(g)))
#define GOB_WIDGET_TYPE(g) (BLK_SKIP(GOB_CONTENT(g), 1))
#define GOB_WIDGET_SPEC(g) (BLK_SKIP(GOB_CONTENT(g), 2))
#define GOB_WIDGET_DATA(g) (BLK_SKIP(GOB_CONTENT(g), 3))

// Control dependencies on series structures:
#ifdef REB_DEF
#define GOB_STRING(g)	SERIES_DATA(GOB_CONTENT(g))
#define GOB_TAIL(g)		SERIES_TAIL((g)->pane)
#define GOB_HEAD(g)		((REBGOB **)(SERIES_DATA(GOB_PANE(g))))
#else
#define GOB_STRING(g)	((REBYTE *)RL_Series(GOB_CONTENT(g), (REBCNT)RXI_SER_DATA))
#define GOB_TAIL(g)		((REBCNT)RL_Series(GOB_PANE(g), (REBCNT)RXI_SER_TAIL))
#define GOB_HEAD(g)		((REBGOB **)RL_Series(GOB_PANE(g), (REBCNT)RXI_SER_DATA))
#endif
#define GOB_BITMAP(g)	GOB_STRING(g)
#define GOB_SKIP(g,n)	(GOB_HEAD(g)+n)

#define IS_WINDOW(g)	(GOB_PARENT(g) == Gob_Root && GET_GOB_FLAG(g, GOBF_WINDOW))

#define IS_GOB_COLOR(g)	 (GOB_TYPE(g) == GOBT_COLOR)
#define IS_GOB_DRAW(g)	 (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_DRAW)
#define IS_GOB_IMAGE(g)	 (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_IMAGE)
#define IS_GOB_EFFECT(g) (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_EFFECT)
#define IS_GOB_STRING(g) (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_STRING)
#define IS_GOB_TEXT(g)   (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_TEXT)

// GC Flags:
enum {
	GOB_MARK = 1,		// Gob was found during GC mark scan.
	GOB_USED = 1<<1		// Gob is used (not free).
};

#define IS_GOB_MARK(g)	((g)->resv & GOB_MARK)
#define MARK_GOB(g)		((g)->resv |= GOB_MARK)
#define UNMARK_GOB(g)	((g)->resv &= ~GOB_MARK)
#define IS_GOB_USED(g)	((g)->resv & GOB_USED)
#define USE_GOB(g)		((g)->resv |= GOB_USED)
#define FREE_GOB(g)		((g)->resv &= ~GOB_USED)

extern REBGOB *Gob_Root; // Top level GOB (the screen)

#endif
#pragma pack(4)

typedef struct Reb_Series_Ref
{
	REBSER	*series;
	REBCNT	index;
	union {
		REBSER	*side;		// lookaside block for lists/hashes/images
		REBINT  back;		// (Used in DO for stack back linking)
//		REBFRM	*frame;		// (may also be used as frame for binding blocks)
		REBFLG  vtype;      // vector! type encoding
	} link;
} REBSRI;

#define VAL_SERIES(v)	    ((v)->data.series.series)
#define VAL_INDEX(v)	    ((v)->data.series.index)
#define	VAL_TAIL(v)		    (VAL_SERIES(v)->tail)
#define	VAL_REST(v)		    (VAL_SERIES(v)->rest)
#define VAL_LEN(v)			(Val_Series_Len(v))

#define VAL_DATA(s)			(VAL_BIN_HEAD(s) + (VAL_INDEX(s) * VAL_SERIES_WIDTH(s)))

#define VAL_BACK(v)			((v)->data.series.link.back)
#define	VAL_SERIES_SIDE(v)  ((v)->data.series.link.side)
#define	VAL_SERIES_FRAME(v) ((v)->data.series.link.frame)
#define VAL_SERIES_WIDTH(v) (SERIES_WIDE(VAL_SERIES(v)))
#define VAL_LIMIT_SERIES(v) if (VAL_INDEX(v) > VAL_TAIL(v)) VAL_INDEX(v) = VAL_TAIL(v)
#define VAL_IS_UTF8(v)      IS_UTF8_SERIES(VAL_SERIES(v))

#define DIFF_PTRS(a,b) (REBCNT)((REBYTE*)a - (REBYTE*)b)


/***********************************************************************
**
**	STRINGS -- All string related values
**
***********************************************************************/

#define	SET_STRING(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_STRING)
#define	SET_BINARY(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_BINARY)
#define	SET_FILE(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_FILE)
#define	SET_STR_TYPE(t,v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, t)
#define SET_STR_END(s,n) (*STR_SKIP(s,n) = 0)

// Arg is a binary (byte) series:
#define	BIN_HEAD(s)		((REBYTE *)((s)->data))
#define BIN_DATA(s)		((REBYTE *)((s)->data))
#define	BIN_TAIL(s)		(REBYTE*)STR_TAIL(s)
#define BIN_SKIP(s, n)	(((REBYTE *)((s)->data))+(n))
#define	BIN_LEN(s)		(SERIES_TAIL(s))
#define VAL_BIN_AT(v)   ((REBYTE*)(BIN_DATA(VAL_SERIES(v))+VAL_INDEX(v)))
#define VAL_BIN_LEN(v)  ((SERIES_TAIL(VAL_SERIES(v)) - VAL_INDEX(v)))

// Arg is a unicode series:
#define UNI_HEAD(s)		((REBUNI *)((s)->data))
#define UNI_SKIP(s, n)	(((REBUNI *)((s)->data))+(n))
#define UNI_TAIL(s)		(((REBUNI *)((s)->data))+(s)->tail)
#define	UNI_LAST(s)		(((REBUNI *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	UNI_LEN(s)		(SERIES_TAIL(s))
#define UNI_TERM(s)		(*UNI_TAIL(s) = 0)
#define UNI_RESET(s)	(UNI_HEAD(s)[(s)->tail = 0] = 0)

// Obsolete (remove after Unicode conversion):
#define STR_HEAD(s)		((REBYTE *)((s)->data))
#define STR_DATA(s)		((REBYTE *)((s)->data))
#define STR_SKIP(s, n)	(((REBYTE *)((s)->data))+(n))
#define STR_TAIL(s)		(((REBYTE *)((s)->data))+(s)->tail)
#define	STR_LAST(s)		(((REBYTE *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	STR_LEN(s)		(SERIES_TAIL(s))
#define STR_TERM(s)		(*STR_TAIL(s) = 0)
#define STR_RESET(s)	(STR_HEAD(s)[(s)->tail = 0] = 0)

// Arg is a binary value:
#define VAL_BIN(v)		BIN_HEAD(VAL_SERIES(v))
#define VAL_BIN_HEAD(v)	BIN_HEAD(VAL_SERIES(v))
#define VAL_BIN_DATA(v)	BIN_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_BIN_SKIP(v,n) BIN_SKIP(VAL_SERIES(v), (n))
#define VAL_BIN_TAIL(v)	BIN_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)

// Arg is a unicode value:
#define VAL_UNI(v)		UNI_HEAD(VAL_SERIES(v))
#define VAL_UNI_HEAD(v) UNI_HEAD(VAL_SERIES(v))
#define VAL_UNI_TAIL(v)	UNI_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)
#define VAL_UNI_DATA(v) UNI_SKIP(VAL_SERIES(v), VAL_INDEX(v))

// Get a char, from either byte or unicode string:
#define GET_ANY_CHAR(s,n)   (REBUNI)(BYTE_SIZE(s) ? BIN_HEAD(s)[n] : UNI_HEAD(s)[n])
#define GET_UTF8_CHAR(s,n) (REBU32)(IS_UTF8_SERIES(s) ? UTF8_Get_Codepoint(BIN_SKIP(s, n)) : BIN_HEAD(s)[n])
#define SET_ANY_CHAR(s,n,c) \
		if (c > 0x7F || IS_UTF8_SERIES(s)) { \
			UTF8_SERIES(s); \
			UTF8_Replace_Codepoint(s, n, c); \
		} else BIN_HEAD(s)[n] = (REBYTE)c;
#define GET_CHAR_UNI(f,p,i) (uni ? ((REBUNI*)p)[i] : ((REBYTE*)bp)[i])

#define VAL_ANY_CHAR(v) GET_ANY_CHAR(VAL_SERIES(v), VAL_INDEX(v))

//#define VAL_STR_LAST(v)	STR_LAST(VAL_SERIES(v))
//#define	VAL_MEM_LEN(v)	(VAL_TAIL(v) * VAL_SERIES_WIDTH(v))

// `mold` limit related defines
#define NO_LIMIT (REBCNT)-1
#define MOLD_HAS_LIMIT(mold)  (mold->limit != NO_LIMIT)
#define MOLD_OVER_LIMIT(mold) (mold->series->tail >= mold->limit)
#define MOLD_REST(mold) (mold->limit - mold->series->tail)
#define CHECK_MOLD_LIMIT(mold, len)                           \
		if (MOLD_HAS_LIMIT(mold)) {                           \
			if (MOLD_OVER_LIMIT(mold)) return;                \
			if (MOLD_REST(mold) < len) len = MOLD_REST(mold); \
		}

/***********************************************************************
**
**	VECTOR
**
***********************************************************************/

// Encoding identifiers:
enum {
	VTSI08, VTSI16, VTSI32, VTSI64,
	VTUI08, VTUI16, VTUI32, VTUI64,
	VTSF08, VTSF16, VTSF32, VTSF64, // VTSF08 and VTSF16 not used!
	VT_MAX,
};

// Elements are structs - their size is not encoded in the type identifier!
#define VTSTRUCT  VT_MAX

#define VECT_BB_MASK    0x00000003  // bits 1..0   -- bit-width code (0-3)
#define VECT_SIGN_MASK  0x00000004  // bit  2      -- signed flag
#define VECT_TYPE_MASK  0x00000008  // bit  3      -- float flag

#define VECT_BB_SHIFT    0
#define VECT_SIGN_SHIFT  2
#define VECT_TYPE_SHIFT  3

// Encode vector element type from components
#define VECT_MAKE_TYPE(bb, sign, isfloat) \
    ( (isfloat) ? (VTSF08 + (bb)) \
    : (sign)    ? (VTSI08 + (bb)) \
    :             (VTUI08 + (bb)) )

#define VECT_INFO_TYPE_MASK   0x000000FF
#define VECT_INFO_ROWS_SHIFT  8
#define VECT_INFO_ROWS_MAX   (0xFFFFFFFFu >> VECT_INFO_ROWS_SHIFT) // 0x00FFFFFF = 16777215

#define VAL_VEC_INFO(v)   ((v)->data.series.link.vtype)           // raw packed field
#define VAL_VEC_TYPE(v)   (VAL_VEC_INFO(v) & VECT_INFO_TYPE_MASK) // one of VTSI08..VTSF64
#define VAL_VEC_ROWS(v)   ((REBCNT)VAL_VEC_INFO(v) >> VECT_INFO_ROWS_SHIFT)
#define VAL_VEC_SET_ROWS(v,r) \
	(VAL_VEC_INFO(v) = (VAL_VEC_INFO(v) & VECT_INFO_TYPE_MASK) | ((REBCNT)(r) << VECT_INFO_ROWS_SHIFT))
#define VAL_VEC_COLS(v) \
	(VAL_VEC_ROWS(v) ? VAL_TAIL(v) / VAL_VEC_ROWS(v) : 0)

// Derive width from the low 2 bits of the enum value
#define VECT_BITS(vtype) (8 << (vtype & 3))
#define VECT_WIDE(vtype) (1 << (vtype & 3))
// Sign: unsigned identifiers start at VTUI08=4, so bit 2 is the sign flag
#define VECT_SIGN(vtype) ((vtype & 4) == 0) // 0=signed, 1=unsigned
// Float: float identifiers start at VTSF08=8, so bit 3 is the float flag
#define VECT_DECI(vtype) ((vtype & 8) != 0)
#define VECT_DIMS(vtype) 1 // TODO!

// Struct elements have an arbitrary size which cannot be derived from the type
// identifier, so it is kept as the series width - and that one is 8 bits only!
#define VECT_IS_STRUCT(vtype)  ((vtype) == VTSTRUCT)
#define VECT_STRUCT_MAX_SIZE   255

#define VAL_VEC_IS_STRUCT(v) VECT_IS_STRUCT(VAL_VEC_TYPE(v))
// A vector of structs keeps the element's field list in the series link, just
// like a struct's data series does, so that an element may be used as a struct
// view. Both the fields and the specification are protected from the GC when
// the struct is prepared, so there is nothing to mark here.
#define VAL_VEC_STRUCT(v) (VAL_SERIES(v)->series)

#define VAL_VEC_BITS(v)  (VECT_BITS(VAL_VEC_INFO(v)))
#define VAL_VEC_WIDE(v)  (VAL_VEC_IS_STRUCT(v) ? SERIES_WIDE(VAL_SERIES(v)) : VECT_WIDE(VAL_VEC_INFO(v)))
#define VAL_VEC_SIGN(v)  (VECT_SIGN(VAL_VEC_INFO(v)))  
#define VAL_VEC_DECI(v)  (VECT_DECI(VAL_VEC_INFO(v)))  

#define VAL_VEC_TAIL(v)  VAL_TAIL(v)
#define VAL_VEC_HEAD(v)  (VAL_BIN_HEAD(v))
#define VAL_VEC_DATA(v)  (VAL_BIN_HEAD(v) + VAL_INDEX(v) * VAL_VEC_WIDE(v))

#define SET_VECTOR(v,s,t) \
	VAL_SERIES(v)=(s), \
	VAL_VEC_INFO(v) = ((t) & VECT_INFO_TYPE_MASK) | (1 << VECT_INFO_ROWS_SHIFT), \
	VAL_INDEX(v)=0, \
	VAL_SET(v, REB_VECTOR)


/***********************************************************************
**
**	IMAGES, QUADS - RGBA
**
***********************************************************************/

//typedef struct Reb_ImageInfo
//{
//	REBCNT width;
//	REBCNT height;
//	REBINT transp;
//} REBIMI;

#define QUAD_HEAD(s)	((REBYTE *)((s)->data))
#define QUAD_SKIP(s,n)	(((REBYTE *)((s)->data))+((n) * 4))
#define QUAD_TAIL(s)	(((REBYTE *)((s)->data))+((s)->tail * 4))
#define	QUAD_LEN(s)		(SERIES_TAIL(s))

#define	IMG_SIZE(s)		((s)->size)
#define	IMG_WIDE(s)		((s)->area.wide)
#define	IMG_HIGH(s)		((s)->area.high)
#define IMG_DATA(s)		((REBYTE *)((s)->data))

#define VAL_IMAGE_HEAD(v)	QUAD_HEAD(VAL_SERIES(v))
#define VAL_IMAGE_TAIL(v)	QUAD_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)
#define VAL_IMAGE_DATA(v)	QUAD_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_IMAGE_BITS(v)	((REBCNT *)VAL_IMAGE_HEAD((v)))
#define	VAL_IMAGE_WIDE(v)	(IMG_WIDE(VAL_SERIES(v)))
#define	VAL_IMAGE_HIGH(v)	(IMG_HIGH(VAL_SERIES(v)))
#define	VAL_IMAGE_LEN(v)	VAL_LEN(v)

#define SET_IMAGE(v,s) VAL_SET(v, REB_IMAGE);VAL_SERIES(v)=s;VAL_INDEX(v) = 0;


//#define VAL_IMAGE_TRANSP(v) (VAL_IMAGE_INFO(v)->transp)
//#define VAL_IMAGE_TRANSP_TYPE(v) (VAL_IMAGE_TRANSP(v)&0xff000000)
//#define VITT_UNKNOWN	0x00000000
//#define VITT_NONE		0x01000000
//#define VITT_ALPHA		0x02000000
//#define	VAL_IMAGE_DEPTH(v)	((VAL_IMAGE_INFO(v)>>24)&0x3f)
//#define VAL_IMAGE_TYPE(v)		((VAL_IMAGE_INFO(v)>>30)&3)

// New Image Datatype defines:

//tuple to image! pixel order bytes
#define TO_PIXEL_TUPLE(t) TO_PIXEL_COLOR(VAL_TUPLE(t)[0], VAL_TUPLE(t)[1], VAL_TUPLE(t)[2], \
							VAL_TUPLE_LEN(t) > 3 ? VAL_TUPLE(t)[3] : 0xff)
//tuple to RGBA bytes
#define TO_COLOR_TUPLE(t) TO_RGBA_COLOR(VAL_TUPLE(t)[0], VAL_TUPLE(t)[1], VAL_TUPLE(t)[2], \
							VAL_TUPLE_LEN(t) > 3 ? VAL_TUPLE(t)[3] : 0xff)

/***********************************************************************
**
**	Logic and Logic Bits
**
***********************************************************************/

#define VAL_LOGIC(v)	((v)->data.logic)
#define	SET_LOGIC(v,n)	VAL_SET(v, REB_LOGIC), VAL_LOGIC(v) = ((n)!=0) //, VAL_LOGIC_WORDS(v)=0
#define SET_TRUE(v)		SET_LOGIC(v, TRUE)  // compound statement
#define SET_FALSE(v)	SET_LOGIC(v, FALSE) // compound statement
#define IS_FALSE(v)		(IS_NONE(v) || (IS_LOGIC(v) && !VAL_LOGIC(v)))
#define	IS_TRUE(v)		(!IS_FALSE(v))
#define VAL_I32(v)		((v)->data.logic)	// used for handles, etc.


/***********************************************************************
**
**	BIT_SET -- Bit sets
**
***********************************************************************/

#define BITS_NOT(s)        ((s)->size)
#define	VAL_BITSET(v)      VAL_SERIES(v)
#define	VAL_BITSET_NOT(v)  BITS_NOT(VAL_SERIES(v))

#define	VAL_BIT_DATA(v)	VAL_BIN(v)

#define	SET_BIT(d,n)	((d)[(n) >> 3] |= (1 << ((n) & 7)))
#define	CLR_BIT(d,n)	((d)[(n) >> 3] &= ~(1 << ((n) & 7)))
#define	IS_BIT(d,n)		((d)[(n) >> 3] & (1 << ((n) & 7)))


/***********************************************************************
**
**	BLOCKS -- Block is a terminated string of values
**
***********************************************************************/

#define NOT_END(v)		(!IS_END(v))

// Arg is a series:
#define BLK_HEAD(s)		((REBVAL *)((s)->data))
#define BLK_SKIP(s, n)	(((REBVAL *)((s)->data))+(n))
#define	BLK_TAIL(s)		(((REBVAL *)((s)->data))+(s)->tail)
#define	BLK_LAST(s)		(((REBVAL *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	BLK_LEN(s)		(SERIES_TAIL(s))
#define BLK_TERM(s)		SET_END(BLK_TAIL(s))
#define BLK_RESET(b)	(b)->tail = 0, SET_END(BLK_HEAD(b))

// Arg is a value:
#define VAL_BLK(v)		BLK_HEAD(VAL_SERIES(v))
#define VAL_BLK_DATA(v)	BLK_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_BLK_DATA_SAFE(v)	BLK_SKIP(VAL_SERIES(v), MIN(VAL_INDEX(v),VAL_TAIL(v))) // don't go behind tail
#define VAL_BLK_SKIP(v,n)	BLK_SKIP(VAL_SERIES(v), (n))
#define VAL_BLK_TAIL(v)	BLK_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)
#define	VAL_BLK_LEN(v)	VAL_LEN(v)
#define VAL_BLK_TERM(v)	BLK_TERM(VAL_SERIES(v))

#define	COPY_VALUES(f,t,l) memcpy(t, f, (l) * sizeof(REBVAL))

#define COPY_BLK_PART(d, s, n) memcpy((d)->data, s, (n) * sizeof(REBVAL)); SERIES_TAIL(d) = n; BLK_TERM(d)

#define IS_EMPTY(v)		(VAL_INDEX(v) >= VAL_TAIL(v))


/***********************************************************************
**
**	LIST & HASH Block Lookaside buffer
**
***********************************************************************/

typedef struct Reb_Side {		// lookaside series
	REBCNT	next;		// next element
	REBCNT	past;		// prior element
} REBLAB;

//#define LIST_HEAD(s)	((REBLAB *)((s)->data))
//#define LIST_SKIP(s,n)	(((REBLAB *)((s)->data))+(n))
//
//#define	VAL_LIST(v)		LIST_HEAD(VAL_SERIES_SIDE(v))
//
//#define	SET_LIST(v,s,l)	VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SER_LIST(v)=(l), VAL_SET(v, REB_LIST)


/***********************************************************************
**
**	SYMBOLS -- Used only for symbol tables
**
***********************************************************************/

typedef struct Reb_Symbol {
	REBCNT	canon;	// Index of the canonical (first) word
	REBCNT	alias;	// Index to next alias form
	REBCNT	name;	// Index into PG_Word_Names string
} REBSYM;

// Arg is value:
#define VAL_SYM_NINDEX(v)	((v)->data.symbol.name)
#define VAL_SYM_NAME(v)		(STR_HEAD(PG_Word_Names) + VAL_SYM_NINDEX(v))
#define VAL_SYM_CANON(v)	((v)->data.symbol.canon)
#define VAL_SYM_ALIAS(v)	((v)->data.symbol.alias)

// Return the CANON value for a symbol number:
#define SYMBOL_TO_CANON(sym) (VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, sym)))
// Return the CANON value for a word value:
#define WORD_TO_CANON(w) (VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(w))))

#define SYMBOL_TO_NAME(sym) VAL_SYM_NAME(BLK_SKIP(PG_Word_Table.series, sym))

#define IS_STAR(v) (IS_WORD(v) && VAL_WORD_CANON(v) == SYM__P)


/***********************************************************************
**
**	WORDS -- All word related types
**
***********************************************************************/

typedef struct Reb_Word {
	REBCNT	sym;		// Index of the word's symbol
	REBINT	index;		// Index of the word in the frame
	REBSER	*frame;		// Frame in which the word is defined
} REBWRD;

typedef struct Reb_Word_Spec {
	REBCNT	sym;		// Index of the word's symbol (and pad for U64 alignment)
	REBU64	typeset;
} REBWRS;

#define IS_SAME_WORD(v, n)		(IS_WORD(v) && VAL_WORD_CANON(v) == n)

#define VAL_WORD_SYM(v)			((v)->data.word.sym)
#define VAL_WORD_INDEX(v)		((v)->data.word.index)
#define VAL_WORD_FRAME(v)		((v)->data.word.frame)
#define HAS_FRAME(v)			VAL_WORD_FRAME(v)

#define	UNBIND(v)				VAL_WORD_FRAME(v)=0, VAL_WORD_INDEX(v)=0

#define VAL_WORD_CANON(v)		VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(v)))
#define	VAL_WORD_NAME(v)		VAL_SYM_NAME(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(v)))
#define	VAL_WORD_NAME_STR(v)	STR_HEAD(VAL_WORD_NAME(v))

// When words are used in frame word lists, fields get a different meaning:
#define	VAL_BIND_SYM(v)			((v)->data.wordspec.sym)
#define VAL_BIND_CANON(v)		VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_BIND_SYM(v))) //((v)->data.wordspec.index)
#define VAL_BIND_TYPESET(v)		((v)->data.wordspec.typeset)
#define VAL_WORD_FRAME_WORDS(v) VAL_WORD_FRAME(v)->words
#define VAL_WORD_FRAME_VALUES(v) VAL_WORD_FRAME(v)->values

// Is it the same symbol? Quick check, then canon check:
#define SAME_SYM(a,b) (VAL_WORD_SYM(a)==VAL_BIND_SYM(b)||VAL_WORD_CANON(a)==VAL_BIND_CANON(b))

/***********************************************************************
**
**	Frame -- Used to bind words to values.
**
**		This type of value is used at the head of a frame block.
**		It should appear in no other place.
**
***********************************************************************/

typedef struct Reb_Frame {
	REBSER	*words;
	REBSER	*spec;
//	REBSER	*parent;
} REBFRM;

// Value to frame fields:
#define	VAL_FRM_WORDS(v)	((v)->data.frame.words)
#define	VAL_FRM_SPEC(v)		((v)->data.frame.spec)
//#define	VAL_FRM_PARENT(v)	((v)->data.frame.parent)

// Word number array (used by Bind_Table):
#define WORDS_HEAD(w)		((REBINT *)(w)->data)
#define WORDS_LAST(w)		(((REBINT *)(w)->data)+(w)->tail-1) // (tail never zero)

// Frame series to frame components:
#define FRM_WORD_SERIES(c)	VAL_FRM_WORDS(BLK_HEAD(c))
#define FRM_WORDS(c)		BLK_HEAD(FRM_WORD_SERIES(c))
#define FRM_VALUES(c)		BLK_HEAD(c)
#define FRM_VALUE(c,n)		BLK_SKIP(c,(n))
#define FRM_WORD(c,n)		BLK_SKIP(FRM_WORD_SERIES(c),(n))
#define FRM_WORD_SYM(c,n)	VAL_BIND_SYM(FRM_WORD(c,n))

#define VAL_FRM_WORD(v,n)	BLK_SKIP(FRM_WORD_SERIES(VAL_SERIES(v)),(n))

// Object field (series, index):
#define OFV(s,n)			BLK_SKIP(s,n)

#define SET_FRAME(v, s, w) \
	VAL_FRM_SPEC(v) = (s); \
	VAL_FRM_WORDS(v) = (w); \
	VAL_SET(v, REB_FRAME)

#define SET_SELFLESS(f) VAL_BIND_SYM(FRM_WORDS(f)) = 0
#define IS_SELFLESS(f) (!VAL_BIND_SYM(FRM_WORDS(f)))

/***********************************************************************
**
**	OBJECTS - Object Support
**
***********************************************************************/

typedef struct Reb_Object {
	REBSER	*frame;
	REBSER	*body;		// module body
//	REBSER	*spec;
//	REBCNT	num;		// shortcut for checking error number
} REBOBJ;

//#define	SET_OBJECT(v,s,f) VAL_OBJ_SPEC(v) = (s), VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_OBJECT)
#define	SET_OBJECT(v,f)		VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_OBJECT)
#define	SET_MODULE(v,f)		VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_MODULE)

#define VAL_OBJ_FRAME(v)	((v)->data.object.frame)
#define VAL_OBJ_VALUES(v)	FRM_VALUES((v)->data.object.frame)
#define VAL_OBJ_VALUE(v,n)	FRM_VALUE((v)->data.object.frame, n)
#define VAL_OBJ_WORDS(v)	FRM_WORD_SERIES((v)->data.object.frame)
#define VAL_OBJ_WORD(v,n)	BLK_SKIP(VAL_OBJ_WORDS(v), (n))
//#define VAL_OBJ_SPEC(v)		((v)->data.object.spec)
#define	CLONE_OBJECT(c)		Copy_Series(c)

#define	VAL_MOD_FRAME(v)	((v)->data.object.frame)
#define VAL_MOD_BODY(v)		((v)->data.object.body)
#define VAL_MOD_SPEC(v)		VAL_FRM_SPEC(VAL_OBJ_VALUES(v))

/***********************************************************************
**
**	PORTS - External series interface
**
***********************************************************************/

#define	VAL_PORT(v)			VAL_OBJ_FRAME(v)
#define SET_PORT(v,s)		VAL_SET(v, REB_PORT), VAL_PORT(v) = s


/***********************************************************************
**
**	ERRORS - Error values
**
***********************************************************************/

typedef struct Reb_Error {
	union Reo {
		REBSER	*object;
		REBVAL	*value;		// RETURN value (also BREAK, THROW)
	} reo;
	REBCNT	num;			// (Determines value used below.)
	REBCNT  sym;			// THROW symbol
} REBERR;

// Value Accessors:
#define	VAL_ERR_NUM(v)		((v)->data.error.num)
#define VAL_ERR_OBJECT(v)	((v)->data.error.reo.object)
#define VAL_ERR_VALUE(v)	((v)->data.error.reo.value)
#define VAL_ERR_SYM(v)		((v)->data.error.sym)

#define	IS_THROW(v)			(VAL_ERR_NUM(v) < RE_THROW_MAX)
#define	IS_BREAK(v)			(VAL_ERR_NUM(v) == RE_BREAK)
#define	IS_RETURN(v)		(VAL_ERR_NUM(v) == RE_RETURN)
#define	IS_CONTINUE(v)		(VAL_ERR_NUM(v) == RE_CONTINUE)
#define THROWN(v)			(IS_ERROR(v) && IS_THROW(v))

#define THROWN_DISARM_OFFSET 100000 // used to disarm thrown errors (converted to complete error object)

#define	SET_ERROR(v,n,a)	VAL_SET(v, REB_ERROR), VAL_ERR_NUM(v)=n, VAL_ERR_OBJECT(v)=a, VAL_ERR_SYM(v)=0
#define	SET_THROW(v,n,a)	VAL_SET(v, REB_ERROR), VAL_ERR_NUM(v)=n, VAL_ERR_VALUE(v)=a, VAL_ERR_SYM(v)=0

#define VAL_ERR_VALUES(v)	((ERROR_OBJ *)(FRM_VALUES(VAL_ERR_OBJECT(v))))
#define	VAL_ERR_ARG1(v)		(&VAL_ERR_VALUES(v)->arg1)
#define	VAL_ERR_ARG2(v)		(&VAL_ERR_VALUES(v)->arg2)

// Error Object (frame) Accessors:
#define ERR_VALUES(frame)	((ERROR_OBJ *)FRM_VALUES(frame))
#define	ERR_NUM(frame)		VAL_INT32(&ERR_VALUES(frame)->code)


/***********************************************************************
**
**	GOBS - Graphic Objects
**
***********************************************************************/

typedef struct Reb_Gob {
	REBGOB *gob;
	REBCNT index;
} REBGBO;

#define	VAL_GOB(v)			((v)->data.gob.gob)
#define	VAL_GOB_INDEX(v)	((v)->data.gob.index)
#define SET_GOB(v,g)		VAL_SET(v, REB_GOB), VAL_GOB(v)=g, VAL_GOB_INDEX(v)=0

typedef struct rebol_compositor_ctx REBCMP; // Rebol compositor context

/***********************************************************************
**
**	FUNCTIONS - Natives, actions, operators, and user functions
**
***********************************************************************/

typedef int  (*REBFUN)(REBVAL *ds);				// Native function
typedef int  (*REBACT)(REBVAL *ds, REBCNT a);	// Action function
typedef void (*REBDOF)(REBVAL *ds);				// DO evaltype dispatch function
typedef int  (*REBPAF)(REBVAL *ds, REBVAL *p, REBCNT a); // Port action func

typedef int     (*REB_HANDLE_FREE_FUNC)(void *hnd);
typedef int     (*REB_HANDLE_MOLD_FUNC)(REBHOB *hob, REBSER *ser);
typedef int     (*REB_HANDLE_EVAL_PATH)(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

typedef void (*ANYFUNC)(void *);
typedef void (*TRYFUNC)(void *);
typedef int  (*CMD_FUNC)(REBCNT n, REBSER *args);

#define REBNATIVE(n) int N_##n(REBVAL *ds)
#define REBTYPE(n)   int T_##n(REBVAL *ds, REBCNT action)
#define REBPACT(n)   int P_##n(REBVAL *ds)

typedef struct Reb_Function {
	REBSER	*spec;	// Spec block for function
	REBSER	*args;	// Block of Wordspecs (with typesets)
	union Reb_Func_Code {
		REBFUN	code;
		REBSER	*body;
		REBCNT	act;
	} func;
} REBFCN;

#define VAL_FUNC_SPEC(v)	  ((v)->data.func.spec)	// a series
#define VAL_FUNC_SPEC_BLK(v)  BLK_HEAD((v)->data.func.spec)
#define VAL_FUNC_ARGS(v)	  ((v)->data.func.args)
#define VAL_FUNC_WORDS(v)     VAL_FUNC_ARGS(v)
#define VAL_FUNC_CODE(v)	  ((v)->data.func.func.code)
#define VAL_FUNC_BODY(v)	  ((v)->data.func.func.body)
#define VAL_FUNC_ACT(v)       ((v)->data.func.func.act)
#define VAL_FUNC_ARGC(v)	  SERIES_TAIL((v)->data.func.args)

typedef struct Reb_Path_Value {
	REBVAL *value;	// modified
	REBVAL *select;	// modified
	REBVAL *path;	// modified
	REBVAL *store;  // modified (holds constructed values)
	REBSER *setfrm; // modified
	REBVAL *setval;	// static
	REBVAL *orig;	// static
} REBPVS;

enum Path_Eval_Result {
	PE_OK,
	PE_SET,
	PE_USE,
	PE_NONE,
	PE_BAD_SELECT,
	PE_BAD_SET,
	PE_BAD_RANGE,
	PE_BAD_SET_TYPE,
	PE_BAD_ARGUMENT
};

typedef REBINT (*REBPEF)(REBPVS *pvs); // Path evaluator function

typedef REBINT (*REBCTF)(REBVAL *a, REBVAL *b, REBINT s);

/***********************************************************************
**
**	HANDLE
**
***********************************************************************/
// Initially there was just HANDLE_FUNCTION type, where there was stored
// pointer or index directly in the REB_HANDLE value.
//
// That was later extended to HANDLE_SERIES, where data is holding pointer
// to Rebol managed `series` allocated using `Make_Series`.
// These series had to be locked so the series was not GCed...
// To free such a handle was possible by setting HANDLE_RELEASABLE flag...
//
// Now it's possible to use `HANDLE_CONTEXT` handles registered in system using
// `Register_Handle` function in c-handle.c file.
// Registered handle tracks info, how much bytes is needed when creating a new
// handle of given type. And it also allows to define custom `free` callback function.
//
// Ideally in the future there should be also `mold` and `query` callbacks,
// so it `handle!` value should not be transparent anymore and it would be possible
// to check it's internall data. This new handle can be used as a pseudo-object type.
//
// It's now possible to list all registered handle types using `system/catalog/handles`.
//
// It's quite possible that HANDLE_SERIES/HANDLE_RELEASABLE combo will be removed in
// the future and there will be only HANDLE_FUNCTION (better name?) with HANDLE_CONTEXT
// (but these are still in use so far).

enum Handle_Flags {
	HANDLE_FUNCTION       = 0     ,  // handle has pointer to function (or holds an index) so GC don't mark it
	HANDLE_SERIES         = 1 << 0,  // handle has pointer to REB series, GC will mark it, if not set as releasable 
	HANDLE_RELEASABLE     = 1 << 1,  // GC will not try to mark it, if it is SERIES handle type
	HANDLE_CONTEXT        = 1 << 2,
	HANDLE_CONTEXT_MARKED = 1 << 3,  // used in handle's context (HOB)
	HANDLE_CONTEXT_USED   = 1 << 4,  // --//--
	HANDLE_CONTEXT_LOCKED = 1 << 5,  // so Rebol will not GC the handle if C side still depends on it
};

/***********************************************************************
**  Which flags are part of what a handle IS.
**
**  The low three say what kind of payload the value carries; the rest
**  are context bookkeeping which changes while the value does not.
***********************************************************************/
#define HANDLE_VALUE_FLAGS  (HANDLE_SERIES | HANDLE_RELEASABLE | HANDLE_CONTEXT)

enum Handle_Spec_Flags {
	HANDLE_REQUIRES_HOB_ON_FREE = 1 << 0
};

typedef struct Reb_Handle_Spec {
	REBCNT size;
	REBFLG flags;
	REB_HANDLE_FREE_FUNC free;
	REB_HANDLE_EVAL_PATH get_path;
	REB_HANDLE_EVAL_PATH set_path;
	REB_HANDLE_MOLD_FUNC mold;
} REBHSP;

typedef struct Reb_Handle_Context {
	union {
		REBYTE *data; // Pointer to raw data
		void *handle; // Unspecified pointer (external handle)
	};
	REBCNT  sym;      // Index of the word's symbol. Used as a handle's type!
	REBFLG  flags:16; // Handle_Flags (HANDLE_CONTEXT_MARKED and HANDLE_CONTEXT_USED)
	REBCNT  index:16; // Index into Reb_Handle_Spec value
	REBSER *series;   // Optional pointer to Rebol series, which may be marked by GC
} REBHOB;

typedef struct Reb_Handle {
	union {
		ANYFUNC	code;
		REBHOB *ctx;
		REBSER *data;
		REBINT  index;
	};
	REBCNT	sym;    // Index of the word's symbol. Used as a handle's type! TODO: remove and use REBHOB
	REBFLG  flags;  // Handle_Flags
} REBHAN;

#define VAL_HANDLE(v)		((v)->data.handle.code)
#define VAL_HANDLE_DATA(v)  ((v)->data.handle.data)
#define VAL_HANDLE_CTX(v)   ((v)->data.handle.ctx)
#define VAL_HANDLE_I32(v)   ((v)->data.handle.index) // for handles which are storing just index in the data field
#define VAL_HANDLE_TYPE(v)  ((v)->data.handle.sym)
#define VAL_HANDLE_SYM(v)   ((v)->data.handle.sym)
#define VAL_HANDLE_FLAGS(v) ((v)->data.handle.flags)
#define VAL_HANDLE_CONTEXT_FLAGS(v) (VAL_HANDLE_CTX(v)->flags)
#define VAL_HANDLE_CONTEXT_DATA(v)  (VAL_HANDLE_CTX(v)->data)
#define VAL_HANDLE_NAME(v)  VAL_SYM_NAME(BLK_SKIP(PG_Word_Table.series, VAL_HANDLE_SYM(v)))  // used in MOLD as an info about handle's type

#define HANDLE_SET_FLAG(v, f) (VAL_HANDLE_FLAGS(v) |=  (f))
#define HANDLE_CLR_FLAG(v, f) (VAL_HANDLE_FLAGS(v) &= ~(f))
#define HANDLE_GET_FLAG(v, f) (VAL_HANDLE_FLAGS(v) &   (f))

#define IS_SERIES_HANDLE(v)     HANDLE_GET_FLAG(v, HANDLE_SERIES)
#define IS_CONTEXT_HANDLE(v)    HANDLE_GET_FLAG(v, HANDLE_CONTEXT)
#define IS_FUNCTION_HANDLE(v)  !HANDLE_GET_FLAG(v, HANDLE_SERIES)

#define  IS_VALID_CONTEXT_HANDLE(v, t) (VAL_HANDLE_TYPE(v)==(t) &&  IS_USED_HOB(VAL_HANDLE_CTX(v)))
#define NOT_VALID_CONTEXT_HANDLE(v, t) (VAL_HANDLE_TYPE(v)!=(t) || !IS_USED_HOB(VAL_HANDLE_CTX(v)))

#define SET_HANDLE(v,h,t,f)	VAL_SET(v, REB_HANDLE), VAL_HANDLE(v) = (void*)(h), VAL_HANDLE_TYPE(v) = t, HANDLE_SET_FLAG(v,f)

#define MAKE_HANDLE(v, t) SET_HANDLE(v, Make_Handle_Context(t), t, HANDLE_CONTEXT)

#define IS_USED_HOB(h)         ((h)->flags &   HANDLE_CONTEXT_USED)   // used to detect if handle's context is still valid
#define     USE_HOB(h)         ((h)->flags |=  HANDLE_CONTEXT_USED | HANDLE_CONTEXT_MARKED)
#define   UNUSE_HOB(h)         ((h)->flags &= ~HANDLE_CONTEXT_USED)
#define IS_MARK_HOB(h)         ((h)->flags &  (HANDLE_CONTEXT_MARKED | HANDLE_CONTEXT_LOCKED)) // GC marks still used handles, so these are not released
#define    MARK_HOB(h)         ((h)->flags |=  HANDLE_CONTEXT_MARKED)
#define  UNMARK_HOB(h)         ((h)->flags &= ~HANDLE_CONTEXT_MARKED)
#define MARK_HANDLE_CONTEXT(v) (MARK_HOB(VAL_HANDLE_CTX(v)))

/***********************************************************************
**
**	LIBRARY -- External library management structures
**
***********************************************************************/

typedef struct Reb_Library {
	long handle;        // ALPHA wants a long
	REBSER *name;
	REBCNT id;
} REBLIB;

#define VAL_LIBRARY(v)        (v->data.library)
#define VAL_LIBRARY_HANDLE(v) (v->data.library.handle)
#define VAL_LIBRARY_NAME(v)   (v->data.library.name)
#define VAL_LIBRARY_ID(v)     (v->data.library.id)


/***********************************************************************
**
**	ROUTINE -- External library routine structures
**
***********************************************************************/

typedef struct Reb_Routine {
	FUNCPTR funcptr;
	REBSER  *spec; // struct-ptr
	REBCNT  id;
} REBROT;

typedef struct Reb_Rot_Info {
	REBCNT call_idx;
	REBCNT pad1;
	REBCNT pad2;
} REBFRO;

#define VAL_ROUTINE(v)          (v->data.routine)
#define VAL_ROUTINE_FUNCPTR(v)  (v->data.routine.funcptr)
#define VAL_ROUTINE_SPEC_SER(v) (v->data.routine.spec)
#define VAL_ROUTINE_SPEC(v)     ((REBVAL *) (((REBFRO *)BLK_HEAD(VAL_ROUTINE_SPEC_SER(v))) + 1))
#define VAL_ROUTINE_INFO(v)	((REBFRO *) (((REBFRO *)BLK_HEAD(VAL_ROUTINE_SPEC_SER(v)))))
#define VAL_ROUTINE_ID(v)       (v->data.routine.id)

#define RFRO_CALLIDX(i) ((i)->call_idx)

typedef struct Reb_Typeset {
	REBCNT  pad;	// Allows us to overlay this type on WORD spec type
	REBU64  bits;
} REBTYS;

#define VAL_TYPESET(v)  ((v)->data.typeset.bits)
#define TYPE_CHECK(v,n) ((VAL_TYPESET(v) & ((REBU64)1 << (n))) != (REBU64)0)
#define TYPE_SET(v,n)   (VAL_TYPESET(v) |= ((REBU64)1 << (n)))
#define EQUAL_TYPESET(v,w) (VAL_TYPESET(v) == VAL_TYPESET(w))
#define TYPESET(n) ((REBU64)1 << (n))

//#define TYPE_CHECK(v,n) ((VAL_TYPESET(v)[(n)/32] & (1 << ((n)%32))) != 0)
//#define TYPE_SET(v,n)   (VAL_TYPESET(v)[(n)/32] |= (1 << ((n)%32)))
//#define EQUAL_TYPESET(v,n) (VAL_TYPESET(v)[0] == VAL_TYPESET(n)[0] && VAL_TYPESET(v)[1] == VAL_TYPESET(n)[1])

/***********************************************************************
**
**	STRUCT -- C Structures
**
***********************************************************************/

typedef struct Reb_Struct {
	REBSER *spec;
	REBSER *data;
	REBCNT offset;
	//REBCNT flags;
} REBSTU;

typedef struct Reb_Struct_Field {
	REBCNT sym;
	REBINT type;      /* rebol type */
	REBCNT offset;
	REBCNT dimension; /* for arrays */
	REBCNT size;      /* size of element, in bytes */

	REBSER *spec;     /* for nested struct */

	unsigned int array : 1;
	unsigned int done : 1; /* field is initialized?, used by GC to decide if the value needs to be marked */
} REBSTF;

typedef struct Reb_Struct_Info {
	REBCNT id;    // hash of the specification block
	REBCNT size;  // length of the complete struct in bytes
	REBCNT count; // number of struct fields
	REBCNT name;  // optionaly registered name
	REBCNT flags;
	REBCNT hash;  // hash of field types
} REBSTI;

#define	SET_STRUCT(v) VAL_SET(v, REB_STRUCT), VAL_STRUCT_OFFSET(v) = 0

#define STRUCT_OFFSET(s)     ((s)->offset)
#define STRUCT_SPEC(s)       ((s)->spec)
#define STRUCT_FIELDS_SER(s) (STRUCT_SPEC(s)->series)
#define STRUCT_FIELDS(s)     ((REBSTF *)BLK_HEAD(STRUCT_FIELDS_SER(s)) + 1)
#define STRUCT_FIELDS_NUM(s) (SERIES_TAIL(STRUCT_FIELDS_SER(s)) - 1)
#define STRUCT_INFO(s)       ((REBSTI *)BLK_HEAD(STRUCT_FIELDS_SER(s)))
#define STRUCT_DATA(s)       ((s)->data)
#define STRUCT_DATA_BIN(s)   (BIN_SKIP(STRUCT_DATA(s), STRUCT_OFFSET(s)))
#define STRUCT_ID(s)         (STRUCT_INFO(s)->id)
#define STRUCT_SIZE(s)       (STRUCT_INFO(s)->size)   // complete size in bytes
#define STRUCT_COUNT(s)      (STRUCT_INFO(s)->count)  // number of fields
#define STRUCT_NAME(s)       (STRUCT_INFO(s)->name)
#define STRUCT_FLAGS(s)      (STRUCT_INFO(s)->flags)
#define STRUCT_HASH(s)       (STRUCT_INFO(s)->hash)
#define STRUCT_NEEDS_MARK(s) ((STRUCT_FLAGS(s) & 1) != 0)
#define STRUCT_PROTECTED(s)  ((STRUCT_FLAGS(s) & 2) != 0)

#define VAL_STRUCT(v)        (v->data.structure)
#define VAL_STRUCT_SPEC(v)   (v->data.structure.spec)
#define VAL_STRUCT_OFFSET(v) (v->data.structure.offset)
#define VAL_STRUCT_FIELDS(v) (VAL_STRUCT_SPEC(v)->series)
#define VAL_STRUCT_DATA(v)   (v->data.structure.data)
#define VAL_STRUCT_DATA_BIN(v) (BIN_SKIP(VAL_STRUCT_DATA(v), v->data.structure.offset))
#define VAL_STRUCT_INFO(v)   ((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))
#define VAL_STRUCT_SIZE(v)   (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->size)
#define VAL_STRUCT_COUNT(v)  (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->count)
#define VAL_STRUCT_ID(v)     (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->id)
#define VAL_STRUCT_NAME(v)   (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->name)
#define VAL_STRUCT_FLAGS(v)  (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->flags)
#define VAL_STRUCT_HASH(v)  (((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->hash)
#define VAL_STRUCT_NEEDS_MARK(v) ((((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->flags & 1) != 0)
#define VAL_STRUCT_PROTECTED(v) ((((REBSTI *)BLK_HEAD(VAL_STRUCT_FIELDS(v)))->flags & 2) != 0)

// A struct value is only a view into a data series: the root struct and the
// values of all its (nested) struct fields share one data series and differ
// just in the spec and the offset. The series link (otherwise used only by
// MAP) holds the related field lists:
//
//     spec->series   field list (REBSTI + REBSTF array) of that specification
//     data->series   field list of the ROOT struct of these data
//
// The root is needed by the GC: the data series is marked only once, so the
// Rebol values in it must always be marked from the root of the data, even
// when the data are reached from a value covering just a nested part of it.

#define STRUCT_DATA_ROOT(s)     (STRUCT_DATA(s)->series)
#define VAL_STRUCT_DATA_ROOT(v) (VAL_STRUCT_DATA(v)->series)

// Accessors for a bare field list series (the GC has no REBSTU to use)
#define FIELDS_INFO(ser)        ((REBSTI *)BLK_HEAD(ser))
#define FIELDS_SPEC(ser)        ((ser)->series) // back-pointer to the spec block
#define FIELDS_NEED_MARK(ser)   ((FIELDS_INFO(ser)->flags & STRUCT_FLAG_MARK) != 0)

// Rebol values stored in a struct are accessed in place, so the fields
// holding them must be aligned - see the `#pragma pack` above!
#define STRUCT_VALUE_ALIGN 4

// True when a struct field is a nested struct which holds Rebol values
#define FIELD_SPEC_HAS_VALUES(f) \
	((f)->spec && (f)->spec->series && FIELDS_NEED_MARK((f)->spec->series))

enum {
	STRUCT_FLAG_MARK      = 1, // holds Rebol values which the GC must mark
	STRUCT_FLAG_PROTECTED = 2, // raw data modification is not allowed
};



/***********************************************************************
**
**	UTYPE - User defined types
**
***********************************************************************/

typedef struct Reb_Utype {
	REBSER	*func;	// func object
	REBSER	*data;	// data object
} REBUDT;

#define VAL_UTYPE_FUNC(v)	((v)->data.utype.func)
#define VAL_UTYPE_DATA(v)	((v)->data.utype.data)

// All bits of value fields:
typedef struct Reb_All {
	REBCNT bits[3];
} REBALL;

#define VAL_ALL_BITS(v) ((v)->data.all.bits)

/***********************************************************************
**
*/	struct Reb_Value
/*
**		The structure/union for all REBOL values. Most efficient
**		if it fits into 16 bytes of memory (but not required).
**
***********************************************************************/
{
	union Reb_Val_Head {
		REBHED flags;
		REBCNT header;
	} flags;
#if defined(__LP64__) || defined(__LLP64__)
	REBINT	padding; //make it 32-bit
#endif
	union Reb_Val_Data {
		REBWRD	word;
		REBSRI	series;
		REBCNT  logic;
		REBI64	integer;
		REBU64	unteger;
		REBDEC	decimal;
		REBCNT  uchar;
		REBERR	error;
		REBTYP	datatype;
		REBFRM	frame;
		REBWRS	wordspec;
		REBTYS  typeset;
		REBSYM	symbol;
		REBTIM	time;
		REBTUP	tuple;
		REBFCN	func;
		REBOBJ	object;
		REBXYF	pair;
		REBEVT	event;
		REBLIB  library;
		REBROT  routine;
		REBSTU  structure;
		REBGBO	gob;
		REBUDT  utype;
		REBDCI  deci;
		REBHAN  handle;
		REBALL  all;
	} data;
};

#define ANY_SERIES(v)		(VAL_TYPE(v) >= REB_BINARY && VAL_TYPE(v) <= REB_LIT_PATH)
#define ANY_STR(v)			(VAL_TYPE(v) >= REB_STRING && VAL_TYPE(v) <= REB_TAG)
#define ANY_BINSTR(v)		(VAL_TYPE(v) >= REB_BINARY && VAL_TYPE(v) <= REB_TAG)
#define ANY_BLOCK(v)		(VAL_TYPE(v) >= REB_BLOCK  && VAL_TYPE(v) <= REB_HASH)
#define ANY_BLOCK_OR_MAP(v) (VAL_TYPE(v) >= REB_BLOCK  && VAL_TYPE(v) <= REB_MAP)
#define	ANY_WORD(v)			(VAL_TYPE(v) >= REB_WORD   && VAL_TYPE(v) <= REB_ISSUE)
#define	ANY_PATH(v)			(VAL_TYPE(v) >= REB_PATH   && VAL_TYPE(v) <= REB_LIT_PATH)
#define ANY_FUNC(v)			(VAL_TYPE(v) >= REB_NATIVE && VAL_TYPE(v) <= REB_FUNCTION)
#define ANY_EVAL_BLOCK(v)	(VAL_TYPE(v) >= REB_BLOCK  && VAL_TYPE(v) <= REB_PAREN)
#define ANY_OBJECT(v)		(VAL_TYPE(v) >= REB_OBJECT && VAL_TYPE(v) <= REB_PORT)
#define ANY_NUMBER(v)	    (VAL_TYPE(v) >= REB_INTEGER && VAL_TYPE(v) <= REB_MONEY)
#define ANY_SCALAR(v)	    (VAL_TYPE(v) >= REB_UNSET  && VAL_TYPE(v) <= REB_DATE)

#define ANY_BLOCK_TYPE(t)   (t >= REB_BLOCK   && t <= REB_LIT_PATH)
#define ANY_STR_TYPE(t)     (t >= REB_STRING  && t <= REB_TAG)

#pragma pack()

#endif // value.h


//#include "reb-evtypes.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: Event Types
**  Build: 3.22.8
**  Date:  21-Sep-2026
**  File:  reb-evtypes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


#ifndef REB_EVTYPES_H
#define REB_EVTYPES_H

enum event_types {
	EVT_IGNORE,                   // 0
	EVT_INTERRUPT,                // 1
	EVT_CUSTOM,                   // 2
	EVT_ERROR,                    // 3
	EVT_INIT,                     // 4
	EVT_DEVICE,                   // 5
	EVT_CALLBACK,                 // 6
	EVT_SHUTDOWN,                 // 7
	EVT_SUSPEND,                  // 8
	EVT_RESUME,                   // 9
	EVT_THEME_CHANGE,             // 10
	EVT_OPEN = 32,                // 32
	EVT_CLOSE,                    // 33
	EVT_CONNECT,                  // 34
	EVT_ACCEPT,                   // 35
	EVT_READ,                     // 36
	EVT_WRITE,                    // 37
	EVT_WROTE,                    // 38
	EVT_LOOKUP,                   // 39
	EVT_READY,                    // 40
	EVT_DONE,                     // 41
	EVT_TIME,                     // 42
	EVT_PENDING,                  // 43
	EVT_SHOW = 64,                // 64
	EVT_HIDE,                     // 65
	EVT_OFFSET,                   // 66
	EVT_RESIZE,                   // 67
	EVT_ACTIVE,                   // 68
	EVT_INACTIVE,                 // 69
	EVT_MINIMIZE,                 // 70
	EVT_MAXIMIZE,                 // 71
	EVT_RESTORE,                  // 72
	EVT_FOCUS,                    // 73
	EVT_UNFOCUS,                  // 74
	EVT_CLOSE_REQUEST,            // 75
	EVT_DPI_CHANGE,               // 76
	EVT_MOVE = 96,                // 96
	EVT_DOWN,                     // 97
	EVT_UP,                       // 98
	EVT_ALT_DOWN,                 // 99
	EVT_ALT_UP,                   // 100
	EVT_AUX_DOWN,                 // 101
	EVT_AUX_UP,                   // 102
	EVT_CLICK,                    // 103
	EVT_ENTER,                    // 104
	EVT_LEAVE,                    // 105
	EVT_SCROLL_LINE,              // 106
	EVT_SCROLL_PAGE,              // 107
	EVT_TOUCH_DOWN,               // 108
	EVT_TOUCH_UP,                 // 109
	EVT_TOUCH_MOVE,               // 110
	EVT_TOUCH_CANCEL,             // 111
	EVT_KEY = 128,                // 128
	EVT_KEY_UP,                   // 129
	EVT_NAMED_KEY,                // 130
	EVT_NAMED_KEY_UP,             // 131
	EVT_CHAR,                     // 132
	EVT_CHANGE = 160,             // 160
	EVT_SCROLL,                   // 161
	EVT_MENU_SELECT,              // 162
	EVT_MENU_OPEN,                // 163
	EVT_MENU_CLOSE,               // 164
	EVT_DROP_FILE,                // 165
	EVT_DROP_TEXT,                // 166
	EVT_MAX = 192
};

enum event_keys {
	EVK_NONE,
	EVK_PAGE_UP,
	EVK_PAGE_DOWN,
	EVK_END,
	EVK_HOME,
	EVK_LEFT,
	EVK_UP,
	EVK_RIGHT,
	EVK_DOWN,
	EVK_INSERT,
	EVK_DELETE,
	EVK_F1,
	EVK_F2,
	EVK_F3,
	EVK_F4,
	EVK_F5,
	EVK_F6,
	EVK_F7,
	EVK_F8,
	EVK_F9,
	EVK_F10,
	EVK_F11,
	EVK_F12,
	EVK_PASTE_START,
	EVK_PASTE_END,
	EVK_ESCAPE,
	EVK_SHIFT,
	EVK_CONTROL,
	EVK_ALT,
	EVK_PAUSE,
	EVK_CAPITAL,
	EVK_BACKTAB,
	EVK_BACKSPACE,
	EVK_BEGIN,
	EVK_MAX
};


#endif // REB_EVTYPES_H

//#include "reb-ext-handler.h"
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Defines function pointer types used by runtime extensions.
**  Module:  reb-ext-handler.h
**  Author:  Oldes
**  Notes:
**
***********************************************************************/


/***********************************************************************
**  De/Compression runtime extension function pointer types.
**  Used by the RL_Register_Compress_Method function to register
**  compression methods from native extensions.
***********************************************************************/
typedef int (*COMPRESS_FUNC)(
    const REBYTE *input,
    REBLEN        in_len,
	REBCNT        level,
    REBSER      **output,
	REBINT       *error
);

typedef int (*DECOMPRESS_FUNC)(
    const REBYTE *input,
	REBLEN        in_len,
	REBLEN        out_len,
    REBSER      **output,
	REBINT       *error
);



// RXIARG has 16bytes and so there is room only for 15 args, because
// the first RXIARG in the RXIFRM contains types of all used command args.
#define MAX_RXI_ARGS 15

/* Prefix naming conventions:

  RL:  REBOL library API function (or function access macro)
  RXI: REBOL eXtensions Interface (general constructs)
  RXA: REBOL eXtensions function Argument (value)
  RXR: REBOL eXtensions function Return types
  RXE: REBOL eXtensions Error codes
  RXC: REBOL eXtensions Callback flag

*/

// Value structure (for passing args to and from):
// o: originaly (before 64bit) it was designed to fit to 8bytes
// o: but tuple was still allowed to have 10 bytes, so one could not receive
// o: all possible tuple values in the extension side!!
// Now, when there can be stored all 12 allowed tuple bytes, the value must have 16bytes
// on both (32 and 64bit) targets, so maximum number of arguments could be extended (from 7 to 15)
#pragma pack(4)
typedef union rxi_arg_val {
	void *addr;
	i64    int64;
	u64    uint64;
	double dec64;
	REBXYF pair;
	REBYTE bytes[MAX_RXI_ARGS+1];
	struct {
		i32 int32a;
		i32 int32b;
	};
	struct {
		REBD32 dec32a;
		REBD32 dec32b;
	};
	struct {
		void *series;
		u32 index;
	};
	struct {
		void *image;
		REBCNT width:16;
		REBCNT height:16;
		REBCNT image_index;
	};
	struct {
		union {
			void  *ptr;
			REBHOB *hob;    // Handle's context object
		};
		REBCNT type;      // Handle's name (symbol)
		REBFLG flags:16;  // Handle_Flags
		REBCNT index:16;  // Index into Reb_Handle_Spec value
	} handle;
	struct {
		// keeping the same layout how it was before (first byte is size)
		// There could be a more optimal way how to pass colors!
		REBYTE tuple_len;
		REBYTE tuple_bytes[MAX_TUPLE];
	};
	struct {
		REBSER *series; // Rebol series where struct's data are stored
		REBCNT offset;  // like series' index (used with nested structs)
		REBCNT id;      // unique struct id counted as a hash of its specification
	} structure;
	struct {
		REBSER *series;
		REBCNT index;
		REBCNT info;
	} vector;
	// An event is 16 bytes (12 on 32-bit) - it fits whole into the slot,
	// which is why it needs no series and no spec id like struct/vector.
	REBEVT event;
} RXIARG;

// For direct access to arg array:
#define RXI_COUNT(a)	(a[0].bytes[0])
#define RXI_TYPE(a,n)	(a[0].bytes[n])

// Command function call frame:
typedef struct rxi_cmd_frame {
	RXIARG args[MAX_RXI_ARGS+1];	// arg values (128 bits each)
} RXIFRM;

typedef struct rxi_cmd_context {
	void *envr;		// for holding a reference to your environment
	REBSER *block;	// block being evaluated
	REBCNT index;	// 0-based index of current command in block
} REBCEC;

typedef int (*RXICAL)(int cmd, RXIFRM *args, REBCEC *ctx);

#pragma pack()

// Resolved description of a struct argument, filled by RL_Struct_Info().
//
// A struct value is only a view into a data series: the root struct and every
// (nested) struct field share one data series and differ just in the spec and
// the offset. So the size of THIS struct can never be derived from the data
// series - it lives in the specification and must be looked up by the id.
typedef struct rxi_struct_info {
	REBYTE *data;   // first byte of THIS struct (root data + offset)
	REBCNT  size;   // size of THIS struct, in bytes
	REBCNT  count;  // number of fields
	REBCNT  id;     // spec id (hash of the specification block)
	REBCNT  flags;  // STRUCT_FLAG_MARK | STRUCT_FLAG_PROTECTED
	REBSER *fields; // field list series: REBSTI header + REBSTF[count]
} RXISTRU;

// Field list of a resolved struct (the REBSTI header is skipped).
#define RXI_STRUCT_FIELDS(i)    ((REBSTF *)BLK_HEAD((i)->fields) + 1)

// TRUE when an extension may write raw bytes into the struct's data.
// STRUCT_FLAG_MARK data holds real REBVALs which the GC walks, and
// STRUCT_FLAG_PROTECTED forbids raw modification outright. Reading is
// always allowed - this gates writes only.
#define RXI_STRUCT_WRITABLE(i)  \
	(((i)->flags & (STRUCT_FLAG_MARK | STRUCT_FLAG_PROTECTED)) == 0)


// Access macros (indirect access via RXIFRM pointer):
#define RXA_ARG(f,n)            ((f)->args[n])
#define RXA_COUNT(f)            (RXA_ARG(f,0).bytes[0]) // number of args
#define RXA_TYPE(f,n)           (RXA_ARG(f,0).bytes[n]) // types (of first 7 args)
#define RXA_REF(f,n)            (RXA_ARG(f,n).int32a)

#define RXA_INT64(f,n)          (RXA_ARG(f,n).int64)
#define RXA_INT32(f,n)          (i32)(RXA_ARG(f,n).int64)
#define RXA_UINT64(f,n)         (RXA_ARG(f,n).uint64)
#define RXA_DEC64(f,n)          (RXA_ARG(f,n).dec64)
#define RXA_LOGIC(f,n)          (RXA_ARG(f,n).int32a)
#define RXA_CHAR(f,n)           (RXA_ARG(f,n).int32a)
#define RXA_TIME(f,n)           (RXA_ARG(f,n).int64)
#define RXA_DATE(f,n)           (RXA_ARG(f,n).int32a)
#define RXA_WORD(f,n)           (RXA_ARG(f,n).int32a)
#define RXA_PAIR(f,n)           (RXA_ARG(f,n).pair)
#define RXA_TUPLE(f,n)          (RXA_ARG(f,n).tuple_bytes)
#define RXA_TUPLE_LEN(f,n)      (RXA_ARG(f,n).tuple_len)
#define RXA_SERIES(f,n)         (RXA_ARG(f,n).series)
#define RXA_INDEX(f,n)          (RXA_ARG(f,n).index)
#define RXA_OBJECT(f,n)         (RXA_ARG(f,n).addr)
#define RXA_MODULE(f,n)         (RXA_ARG(f,n).addr)
#define RXA_PORT(f,n)           (RXA_ARG(f,n).addr)
#define RXA_HANDLE(f,n)         (RXA_ARG(f,n).handle.ptr)
#define RXA_HANDLE_CONTEXT(f,n) (RXA_ARG(f,n).handle.hob)
#define RXA_HANDLE_TYPE(f,n)    (RXA_ARG(f,n).handle.type)
#define RXA_HANDLE_FLAGS(f,n)   (RXA_ARG(f,n).handle.flags)
#define RXA_HANDLE_INDEX(f,n)   (RXA_ARG(f,n).handle.index)
#define RXA_IMAGE(f,n)          (RXA_ARG(f,n).image)
#define RXA_IMAGE_BITS(f,n)     ((REBYTE *)RL_SERIES((RXA_ARG(f,n).image), RXI_SER_DATA))
#define RXA_IMAGE_WIDTH(f,n)    (RXA_ARG(f,n).width)
#define RXA_IMAGE_HEIGHT(f,n)   (RXA_ARG(f,n).height)
#define RXA_IMAGE_INDEX(f,n)    (RXA_ARG(f,n).image_index)
// The pixel the value is AT, and how many pixels are left from there -
// the counterparts of VAL_IMAGE_DATA and VAL_IMAGE_LEN.
#define RXA_IMAGE_DATA(f,n)     (RXA_IMAGE_BITS(f,n) + (RXA_IMAGE_INDEX(f,n) * 4))
#define RXA_IMAGE_LEN(f,n)      (((REBCNT)RXA_IMAGE_WIDTH(f,n) * (REBCNT)RXA_IMAGE_HEIGHT(f,n)) \
                                 - RXA_IMAGE_INDEX(f,n))

#define RXA_STRUCT_SER(f,n)     (RXA_ARG(f,n).structure.series)
#define RXA_STRUCT_OFFSET(f,n)  (RXA_ARG(f,n).structure.offset)
#define RXA_STRUCT_ID(f,n)      (RXA_ARG(f,n).structure.id)
#define RXA_STRUCT_SPEC(f,n)    (RL_STRUCT_SPEC(RXA_STRUCT_ID(f,n)))
// Resolve a struct argument in one lookup. Returns FALSE for an unknown spec
// id or a view which does not fit into its data series - ALWAYS check it.
#define RXA_STRUCT_INFO(f,n,i)  (RL_STRUCT_INFO(&RXA_ARG(f,n), (i)))

#define RXA_VECTOR_SERIES(f,n)  (RXA_ARG(f,n).vector.series)
#define RXA_VECTOR_INDEX(f,n)   (RXA_ARG(f,n).vector.index)
#define RXA_VECTOR_INFO(f,n)    (RXA_ARG(f,n).vector.info)

// Events cross by value, so these just read and write the fields in place.
// The union member holding a pointer (req/port/ser) is deliberately NOT
// exposed here - see the EVM_HANDLE patch.
#define RXA_EVENT(f,n)          (RXA_ARG(f,n).event)
#define RXA_EVENT_TYPE(f,n)     (RXA_ARG(f,n).event.type)
#define RXA_EVENT_FLAGS(f,n)    (RXA_ARG(f,n).event.flags)
#define RXA_EVENT_WIN(f,n)      (RXA_ARG(f,n).event.win)
#define RXA_EVENT_MODEL(f,n)    (RXA_ARG(f,n).event.model)
#define RXA_EVENT_DATA(f,n)     (RXA_ARG(f,n).event.data)
#define RXA_EVENT_X(f,n)        ((REBINT)(short)(RXA_EVENT_DATA(f,n) & 0xffff))
#define RXA_EVENT_Y(f,n)        ((REBINT)(short)((RXA_EVENT_DATA(f,n) >> 16) & 0xffff))
#define RXA_SET_EVENT_XY(f,n,x,y) (RXA_EVENT_DATA(f,n) = (((y) << 16) | ((x) & 0xffff)))

// An extension-sourced event refers to one of the extension's own context
// handles - a widget, a window, a drop-file payload. The handle keeps the
// state; the event only says which one this happened to.
#define RXA_EVENT_HANDLE(f,n)   (RXA_ARG(f,n).event.hob)
#define RXA_SET_EVENT_HANDLE(f,n,h) \
	(RXA_EVENT_MODEL(f,n) = EVM_HANDLE, RXA_EVENT_HANDLE(f,n) = (h))

// Command function return values:
enum rxi_return {
	RXR_UNSET,
	RXR_NONE,
	RXR_TRUE,
	RXR_FALSE,

	RXR_VALUE,
	RXR_BLOCK,
	RXR_ERROR,
	RXR_BAD_ARGS,
	RXR_NO_COMMAND,
};

// Used with RXI_SERIES_INFO:
enum {
	RXI_SER_DATA,	// pointer to data
	RXI_SER_TAIL,	// series tail index (length of data)
	RXI_SER_SIZE,	// size of series (in units)
	RXI_SER_WIDE,	// width of series (in bytes)
	RXI_SER_LEFT,	// units free in series (past tail)
};

// Vector info
#define RXI_VEC_TYPE_MASK   0x00000003
#define RXI_VEC_SIGN_MASK   0x00000004
#define RXI_VEC_FLOAT_MASK  0x00000008
#define RXI_VEC_ROWS_SHIFT  8

#define RXI_VECTOR_TYPE(info)   ((info) & 0x000000FF)
#define RXI_VECTOR_BITS(info)   (8 << ((info) & RXI_VEC_TYPE_MASK))
#define RXI_VECTOR_SIGNED(info) (((info) & RXI_VEC_SIGN_MASK) == 0)
#define RXI_VECTOR_FLOAT(info)  (((info) & RXI_VEC_FLOAT_MASK) != 0)
#define RXI_VECTOR_ROWS(info)   ((REBCNT)(info) >> RXI_VEC_ROWS_SHIFT)
#define RXI_VECTOR_COLS(tail, info) \
	(RXI_VECTOR_ROWS(info) ? ((REBCNT)(tail) / RXI_VECTOR_ROWS(info)) : 0)

// Error Codes (returned in result value from some API functions):
enum {
	RXE_NO_ERROR,
	RXE_NO_WORD,	// the word cannot be found (e.g. in an object)
	RXE_NOT_FUNC,	// the value is not a function (for callback)
	RXE_BAD_ARGS,	// function arguments to not match
};

#define SET_EXT_ERROR(v,n) ((v)->int32a = (n))
#define GET_EXT_ERROR(v)   ((v)->int32a)

typedef struct rxi_callback_info {
	u32 flags;
	REBSER *obj;	// object that holds the function
	u32 word;		// word id for function (name)
	RXIARG *args;	// argument list for function
	RXIARG result;	// result from function
} RXICBI;

enum {
	RXC_NONE,
	RXC_ASYNC,		// async callback
	RXC_QUEUED,		// pending in event queue
	RXC_DONE,		// call completed, structs can be freed
	RXC_ALLOC,		// callback was allocated and must be freed on done
};


#define AS_WORD(w) RL_MAP_WORD(b_cast(w)) // may be used to awoid warning casting from char* to REBYTE*


// File: reb-codec.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2021-2023 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: REBOL Codec Definitions
**  Module:  reb-codec.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
#ifndef CODI_DEFINED
#define CODI_DEFINED

//#include "reb-c.h"

// Codec image interface:
//
// If your codec routine returns CODI_IMAGE, it is expected that the
// ->bits field contains a block of memory allocated with Make_Mem
// of size (->w * ->h * 4).  This will be freed by the
// REBNATIVE(do_codec) in n-system.c
//
// If your codec routine returns CODI_BINARY, it is
// expected that the ->data field contains a block of memory
// allocated with Make_Mem of size ->len.  This will be freed by
// the REBNATIVE(do_codec) in n-system.c
//
// If your codec routine returns CODI_TEXT, it is
// expected that the ->data field is 3rd input binary! argument in
// the REBNATIVE(do_codec) in n-system.c
// so the deallocation is left to GC
//
struct reb_codec_image {
	int action;
	int w;
	int h;
	u32 len;
	union {
		int alpha;
		int type;  // used to provide info about prefered image type (codec)
	};
	unsigned char *data;
	union {
		unsigned int *bits;
		void *other;
	};
	int error;
};

typedef struct reb_codec_image REBCDI;

typedef int (*codo)(REBCDI *cdi);

// Media types:
enum {
	CODI_ERROR,
	CODI_CHECK,				// error code is inverted result (IDENTIFY)
	CODI_BINARY,
	CODI_TEXT,
	CODI_IMAGE,
	CODI_SOUND,
	CODI_BLOCK,
	CODI_STRING,			// result is in codi->other as a series (no need to copy).
};

// Codec commands:
enum {
	CODI_IDENTIFY,
	CODI_DECODE,
	CODI_ENCODE,
};

// Codec errors:
enum {
	CODI_ERR_NA = 1,		// Feature not available
	CODI_ERR_NO_ACTION,		// Requested action unknown
	CODI_ERR_ENCODING,		// Encoding method not supported
	CODI_ERR_SIGNATURE,		// Header signature is not correct
	CODI_ERR_BIT_LEN,		// Bit length is not supported
	CODI_ERR_BAD_TABLE,		// Image tables are wrong
	CODI_ERR_BAD_DATA,		// Generic
};

enum {
	CODI_IMG_PNG,           // Portable Network Graphics
	CODI_IMG_JPEG,          // Joint Photographic Experts Group
	CODI_IMG_GIF,           // Graphics Interchange Format
	CODI_IMG_DDS,           // DirectDraw Surface
	CODI_IMG_DNG,           // Digital Negative
	CODI_IMG_BMP,           // Device independent bitmap
	CODI_IMG_ICO,
	CODI_IMG_TIFF,          // Tagged Image File Format
	CODI_IMG_JXR,           // Windows Digital Photo (JpegXR)
	CODI_IMG_HEIF,          // High Efficiency Image Format
	CODI_IMG_WEBP,          //
	CODI_IMG_JP2,           // JPEG 2000 (JP2)
};

//#ifndef WINCODEC_ERR_COMPONENTNOTFOUND
//#define WINCODEC_ERR_COMPONENTNOTFOUND  0x88982F50
//#endif

#endif


// File: reb-device.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2013-2025 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: External REBOL Devices (OS Independent)
**  Module:  reb-device.h
**  Author:  Carl Sassenrath
**  Notes:
**      Critical: all struct alignment must be 4 bytes (see compile options)
**
***********************************************************************/

#ifndef DEVICE_H
#define DEVICE_H

// REBOL Device Identifiers:
// Critical: Must be in same order as Device table in host-device.c
enum {
	RDI_SYSTEM,
	RDI_STDIO,
	RDI_CONSOLE,
	RDI_FILE,
	RDI_EVENT,
	RDI_NET,
	RDI_DNS,
	RDI_CHECKSUM,
	RDI_CLIPBOARD,
	RDI_MIDI,
	RDI_CRYPT,
	RDI_SERIAL,
	RDI_AUDIO,
	RDI_MAX,		// number of built-in devices
	RDI_LIMIT = 32	// size of the device table; RDI_MAX..RDI_LIMIT-1
					// are assigned at run time by RL_Register_Device
};


// REBOL Device Commands:
enum {
	RDC_INIT,		// init device driver resources
	RDC_QUIT,		// cleanup device driver resources

	RDC_OPEN,		// open device unit (port)
	RDC_CLOSE,		// close device unit

	RDC_READ,		// read from unit
	RDC_WRITE,		// write to unit

	RDC_POLL,		// check for activity
	RDC_CONNECT,	// connect (in or out)

	RDC_QUERY,		// query unit info
	RDC_MODIFY,		// set modes (also get modes)

	RDC_CREATE,		// create unit target
	RDC_DELETE,		// delete unit target
	RDC_RENAME,
	RDC_LOOKUP,

	RDC_FLUSH,
	RDC_MAX,

	RDC_CUSTOM=32	// start of custom commands
};

// Device Request (Command) Return Codes:
#define DR_PEND   1	// request is still pending
#define DR_DONE   0	// request is complete w/o errors
#define DR_ERROR -1	// request had an error
#define DR_IGNORE -2 // unrecognized escape sequence

// REBOL Device Flags and Options (bitnums):
enum {
	// Status flags:
	RDF_INIT,		// Device is initialized
	RDF_OPEN,		// Global open (for devs that cannot multi-open)
	// Options:
	RDO_MUST_INIT = 16,	// Do not allow auto init (manual init required)
	RDO_AUTO_POLL,	// Poll device, even if no requests (e.g. interrupts)
};

// REBOL Request Flags (bitnums):
enum {
	RRF_OPEN,		// Port is open
	RRF_DONE,		// Request is done (used when extern proc changes it)
	RRF_FLUSH,		// Flush WRITE
//	RRF_PREWAKE,	// C-callback before awake happens (to update port object)
	RRF_PENDING,	// Request is attached to pending list
	RRF_ALLOC,		// Request is allocated, not a temp on stack
	RRF_WIDE,		// Wide char IO
	RRF_ACTIVE,		// Port is active, even no new events yet
	RRF_ERROR,      // WRITE to std_err
};

// REBOL Device Errors:
enum {
	RDE_NONE,
	RDE_NO_DEVICE,	// command did not provide device
	RDE_NO_COMMAND,	// command past end
	RDE_NO_INIT,	// device has not been inited
};

// Device registration results (OS_Register_Device / RL_Register_Device).
// A non-negative result is the assigned device id.
enum {
	RDR_TABLE_FULL   = -1,	// no free slot in the device table
	RDR_BAD_DEVICE   = -2,	// device structure is not usable
	RDR_BAD_REBDEV   = -3,	// caller built against a different REBDEV layout
};

enum {
	RDM_NULL,		// Null device
	RDM_READ_LINE,	// Read line mode
	RDM_CGI
};

// Serial Parity
enum {
	SERIAL_PARITY_NONE,
	SERIAL_PARITY_ODD,
	SERIAL_PARITY_EVEN
};

// Serial Flow Control
enum {
	SERIAL_FLOW_CONTROL_NONE,
	SERIAL_FLOW_CONTROL_HARDWARE,
	SERIAL_FLOW_CONTROL_SOFTWARE
};

typedef struct {
	REBU32 uchar;
	REBU16 virtu;
	REBU16 flags;
} REBKEY;


#pragma pack(4)

// Forward references:
#ifndef REB_EVENT_H
typedef struct rebol_device REBDEV;
typedef struct rebol_devreq REBREQ;
#endif

// Commands:
typedef i32 (*DEVICE_CMD_FUNC)(REBREQ *req);
#define DEVICE_CMD i32 // Used to define

// Device structure:
struct rebol_device {
	char *title;			// title of device
	u32 version;			// version, revision, release
	u32 date;				// year, month, day, hour
	DEVICE_CMD_FUNC *commands;	// command dispatch table
	u32 max_command;		// keep commands in bounds
	u32 flags;				// state: open, signal
	REBREQ *pending;		// pending requests
	i32 req_size;			// size of request struct
};
#define DEFINE_DEV(w,t,v,c,m,s) REBDEV w = { \
	.title = t, \
	.version = v, \
	.date = 0, \
	.commands = c, \
	.max_command = m, \
	.flags = 0, \
	.pending = 0, \
	.req_size = s \
}

// Request structure:		// Allowed to be extended by some devices
// NOTE: when size of this struct is modified, reflect it in the make-reb-lib.reb file! (CHECK_STRUCT_ALIGN)
struct rebol_devreq {
	u32 clen;				// size of extended structure

	// Linkages:
	u32 device;				// device id (dev table)
	REBREQ *next;			// linked list (pending or done lists)
	void *port;				// link back to REBOL port object
	union {
		void *handle;		// OS object
		int socket;			// OS identifier
		int id;
	};

	// Command info:
	i32  command;			// command code
	u32  error;				// error code
	u32  modes;				// special modes, types or attributes
	u16  flags;				// request flags
	u16  state;				// device process flags
	i32  timeout;			// request timeout
#if defined(__LP64__) || defined(__LLP64__)
	i32 _pad;
#endif
//	int (*prewake)(void *);	// callback before awake

	// Common fields:
	union {
		REBYTE *data;		// data to transfer
		REBREQ *sock;		// temp link to related socket
		const REBYTE *cdata;
	};
	u32  length;			// length to transfer
	u32  actual;			// length actually transferred

	// Special fields for common IO uses:
	union {
		struct {
			REBCHR *path;			// file string (in OS local format)
			i64  size;				// file size
			i64  index;				// file index position
			I64  modified_time;     // file modification time (struct)
			I64  accessed_time;     // file access time (struct)
			I64  created_time;      // file creartion time (struct)
		} file;
		struct {
			u32  local_ip;			// local address used
			u32  local_port;		// local port used
			u32  remote_ip;			// remote address
			u32  remote_port;		// remote port
			void *host_info;		// for DNS usage
		} net;
		struct {
			u32  buffer_rows;
			u32  buffer_cols;
			u32  window_rows;
			u32  window_cols;
			i32  length;            // number of bytes already available to read (from stdio) 
		} console;
		struct {
			u32 device_in;  // requested device ID (1-based; 0 = none)
			u32 device_out;
		} midi;
		struct {
			u8  type;
			u8  channels;
			u16 bits;
			u32 rate;
			u32 loop_count;
		} audio;
		struct {
			u32 mode;
			u32 value;
		} modify;

		struct {
			REBCHR *path;			// device path string (in OS local format)
			void *prior_attr;		// termios: retain previous settings to revert on close
			i32 baud;				// baud rate of serial port
			u8	data_bits;			// 5, 6, 7 or 8
			u8	parity;				// odd, even, mark or space
			u8	stop_bits;			// 1 or 2
			u8	flow_control;		// hardware or software
		} serial;

		REBKEY key;

	};
};
#pragma pack()

// Simple macros for common OPEN? test (for some but not all ports):
#define SET_OPEN(r)		SET_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)
#define SET_CLOSED(r)	CLR_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)
#define IS_OPEN(r)		GET_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)

#endif //DEVICE_H


// File: reb-file.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2024 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: Special file device definitions
**  Module:  reb-file.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

// RFM - REBOL File Modes
enum {
	RFM_READ = 0,
	RFM_WRITE,
	RFM_APPEND,
	RFM_SEEK,
	RFM_NEW,
	RFM_READONLY,
	RFM_TRUNCATE,
	RFM_RESEEK,			// file index has moved, reseek
//	RFM_NAME_MEM,		// converted name allocated in mem
	RFM_DIR = 16,
	RFM_DRIVES,         // used only on Windows to get logical drives letters (read %/)
	RFM_PATTERN,        // used only on Posix for reading wildcard patterns (read %*.txt)
};

// RFE - REBOL File Error
enum {
	RFE_BAD_PATH = 1,
	RFE_NO_MODES,		// No file modes specified
	RFE_OPEN_FAIL,		// File open failed
	RFE_BAD_SEEK,		// Seek not supported for this file
	RFE_NO_HANDLE,		// File struct has no handle
	RFE_NO_SEEK,		// Seek action failed
	RFE_BAD_READ,		// Read failed (general)
	RFE_BAD_WRITE,		// Write failed (general)
	RFE_DISK_FULL,		// No space on target volume
	RFE_RESIZE_SERIES,  // Used on Posix to report, that the target series must be resized
};

#define MAX_FILE_NAME 1022


// File: reb-filereq.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: File requestor definitions
**  Module:  reb-filereq.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#define MAX_FILE_REQ_BUF (16*1024)
#define MAX_DIR_REQ_BUF  ( 1*1024) // windows has path limit 260 chars by default

#pragma pack(4)
typedef struct Reb_File_Requestor {
	REBCNT flags;	// multi, load/save, unicode
	REBCHR *title;	// title of requestor
	REBCHR *button; // button name
	REBCHR *dir;	// dir path
	REBCHR *files;	// buffer to hold results
	REBCHR *filter;	// buffer to hold results
	REBINT len;		// length of buffer
} REBRFR;
#pragma pack()

// File Request Flags:
enum {
	FRF_MULTI,
	FRF_SAVE,
	FRF_KEEP,
};



// File: reb-lib.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: REBOL Host and Extension API
**  Build: 3.22.8
**  Date:  21-Sep-2026
**  File:  reb-lib.reb
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-reb-lib.reb)
**
***********************************************************************/


// These constants are created by the release system and can be used to check
// for compatiblity with the reb-lib DLL (using RL_Version.)
#define RL_VER 3
#define RL_REV 22
#define RL_UPD 8

// Bumped ONLY when an existing RL_API function's signature/semantics
// change in a way that breaks old extension binaries calling it - i.e.
// exactly the cases the "append new functions at the end, never modify
// existing ones" convention (see a-lib.c header) is meant to avoid.
// Extensions compiled against ABI N remain loadable by any host whose
// RL_MIN_SUPPORTED_ABI <= N <= RL_ABI_VERSION.
#define RL_ABI_VERSION        1   // bump only on a break like this one
#define RL_MIN_SUPPORTED_ABI  1   // floor; raise only when dropping support for an old break-era

// Compatiblity with the lib requires that structs are aligned using the same
// method. This is concrete, not abstract. The macro below uses struct
// sizes to inform the developer that something is wrong.
#if defined(__LP64__) || defined(__LLP64__)
#define CHECK_STRUCT_ALIGN (sizeof(REBREQ) == 120 && sizeof(REBEVT) == 16)
#else
#define CHECK_STRUCT_ALIGN (sizeof(REBREQ) == 96 && sizeof(REBEVT) == 12)
#endif

// Function entry points for reb-lib (used for MACROS below):
typedef struct rebol_ext_api {
	void (*version)(REBYTE vers[]);
	REBARGS* (*init)(int argc, char **argv);
	void (*dispose)(void);
	int (*start)(REBYTE *bin, REBINT len, REBCNT flags);
	void (*reset)(void);
	void *(*extend)(REBYTE *source, RXICAL call);
	void (*escape)(REBINT reserved);
	int (*do_string)(REBYTE *text, REBCNT flags, RXIARG *result);
	int (*do_binary)(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result);
	int (*do_block)(REBSER *blk, REBCNT flags, RXIARG *result);
	void (*do_commands)(REBSER *blk, REBCNT flags, REBCEC *context);
	void (*print)(const REBYTE *fmt, ...);
	void (*print_tos)(REBCNT flags, REBYTE *marker);
	int (*event)(REBEVT *evt);
	int (*update_event)(REBEVT *evt);
	void *(*make_block)(u32 size);
	void (*expand_series)(REBSER *series, REBCNT index, REBCNT delta);
	void *(*make_string)(u32 size, int unicode);
	void *(*make_image)(u32 width, u32 height);
	int (*make_vector)(RXIARG *out, REBCNT type, REBINT cols, REBINT rows);
	void (*protect_gc)(REBSER *series, u32 flags);
	int (*get_string)(REBSER *series, u32 index, void **str, REBOOL needs_wide);
	int (*get_utf8_string)(REBSER *series, u32 index, void **str);
	u32 (*map_word)(REBYTE *string);
	u32 *(*map_words)(REBSER *series);
	REBYTE *(*word_string)(u32 word);
	u32 (*find_word)(u32 *words, u32 word);
	REBUPT (*series)(REBSER *series, REBCNT what);
	REBU32 (*get_char)(REBSER *series, u32 index);
	u32 (*set_char)(REBSER *series, u32 index, u32 chr);
	int (*get_value_resolved)(REBSER *series, u32 index, RXIARG *result);
	int (*get_value)(REBSER *series, u32 index, RXIARG *result);
	int (*set_value)(REBSER *series, u32 index, RXIARG val, int type);
	u32 *(*words_of_object)(REBSER *obj);
	int (*get_field)(REBSER *obj, u32 word, RXIARG *result);
	int (*set_field)(REBSER *obj, u32 word, RXIARG val, int type);
	int (*callback)(RXICBI *cbi);
	REBCNT (*encode_utf8)(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts);
	REBSER* (*encode_utf8_string)(void *src, REBCNT len, REBFLG uni, REBFLG opts);
	REBSER* (*decode_utf_string)(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni);
	REBCNT (*register_handle)(const REBYTE *name, REBCNT size, void* free_func);
	REBHOB* (*make_handle_context)(REBCNT sym);
	void (*free_handle_context)(REBHOB *hob);
	REBCNT (*decode_utf8_char)(const REBYTE *str, REBCNT *len);
	REBCNT (*register_handle_spec)(const REBYTE *name, REBHSP *spec);
	REBSER* (*to_local_path)(RXIARG *file, REBFLG full, REBFLG utf8);
	REBSER* (*to_rebol_path)(void *src, REBCNT len, REBINT uni);
	REBSER* (*struct_spec)(REBCNT id);
	REBCNT (*encode_utf8_char)(REBYTE *dst, REBU32 chr);
	void* (*mem_alloc)(void *opaque, size_t size);
	void (*mem_free)(void* opaque, void* address);
	int (*register_compress_method)(const REBYTE* name, COMPRESS_FUNC encoder, DECOMPRESS_FUNC decoder);
	REBFLG (*struct_info)(RXIARG *arg, RXISTRU *out);
	REBFLG (*make_struct)(RXIARG *out, REBCNT id, RXISTRU *info);
	void* (*alloc)(size_t size);
	void (*free)(void *mem, size_t size);
	int (*register_device)(REBDEV *dev, u32 dev_size);
	REBREQ *(*port_state)(REBSER *port, REBCNT device);
	int (*do_device)(REBREQ *req, REBCNT command);
} RL_LIB;

#ifndef API_EXPORT
# define RL_API API_EXPORT
# ifdef TO_WINDOWS
#  define API_EXPORT __declspec(dllexport)
# else
#  define API_EXPORT __attribute__((visibility("default")))
# endif
#endif

// Extension entry point functions:
#ifdef TO_WINDOWS
#ifdef __cplusplus
#define RXIEXT extern "C" API_EXPORT
#else
#define RXIEXT API_EXPORT
#endif
#else
#define RXIEXT extern API_EXPORT
#endif

RXIEXT const char *RX_Init(int opts, RL_LIB *lib);
RXIEXT int RX_Quit(int opts);
RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *data);
RXIEXT int RX_Abi(void);

// The macros below will require this base pointer:
extern RL_LIB *RL;  // is passed to the RX_Init() function

// Macros to access reb-lib functions (from non-linked extensions):

#define RL_VERSION(a)               RL->version(a)
/*
**	void RL_Version(REBYTE vers[])
**
**	Obtain current REBOL interpreter version information.
**
**	Returns:
**		A byte array containing version, revision, update, and more.
**	Arguments:
**		vers - a byte array to hold the version info. First byte is length,
**			followed by version, revision, update, system, variation.
**	Notes:
**		This function can be called before any other initialization
**		to determine version compatiblity with the caller.
**
*/

#define RL_INIT(a,b)                RL->init(a,b)
/*
**	REBARGS* RL_Init(int argc, char **argv)
**
**	Initialize the REBOL interpreter.
**
**	Returns:
**		Zero on success, otherwise an error indicating that the
**		host library is not compatible with this release.
**	Arguments:
**		rargs - REBOL command line args and options structure.
**			See the host-args.c module for details.
**		lib - the host lib (OS_ functions) to be used by REBOL.
**			See host-lib.c for details.
**	Notes:
**		This function will allocate and initialize all memory
**		structures used by the REBOL interpreter. This is an
**		extensive process that takes time.
**
*/

#define RL_DISPOSE(void)            RL->dispose(void)
/*
**	void RL_Dispose(void)
**
**	Disposes the REBOL interpreter.
**
**	Returns:
**		nothing
**	Arguments:
**		none
**	Notes:
**		This function will deallocate and release all memory
**		structures used by the REBOL interpreter.
**
*/

#define RL_START(a,b,c)             RL->start(a,b,c)
/*
**	int RL_Start(REBYTE *bin, REBINT len, REBCNT flags)
**
**	Evaluate the default boot function.
**
**	Returns:
**		Zero on success, otherwise indicates an error occurred.
**	Arguments:
**		bin - optional startup code (compressed), can be null
**		len - length of above bin
**		flags - special flags
**	Notes:
**		This function completes the startup sequence by calling
**		the sys/start function.
**
*/

#define RL_RESET(void)              RL->reset(void)
/*
**	void RL_Reset(void)
**
**	Reset REBOL (not implemented)
**
**	Returns:
**		nothing
**	Arguments:
**		none
**	Notes:
**		Intended to reset the REBOL interpreter.
**
*/

#define RL_EXTEND(a,b)              RL->extend(a,b)
/*
**	void *RL_Extend(REBYTE *source, RXICAL call)
**
**	Appends embedded extension to system/catalog/boot-exts.
**
**	Returns:
**		A pointer to the REBOL library (see reb-lib.h).
**	Arguments:
**		source - A pointer to a UTF-8 (or ASCII) string that provides
**			extension module header, function definitions, and other
**			related functions and data.
**		call - A pointer to the extension's command dispatcher.
**	Notes:
**		This function simply adds the embedded extension to the
**		boot-exts list. All other processing and initialization
**		happens later during startup. Each embedded extension is
**		queried and init using LOAD-EXTENSION system native.
**		See c:extensions-embedded
**
*/

#define RL_ESCAPE(a)                RL->escape(a)
/*
**	void RL_Escape(REBINT reserved)
**
**	Signal that code evaluation needs to be interrupted.
**
**	Returns:
**		nothing
**	Arguments:
**		reserved - must be set to zero.
**	Notes:
**		This function set's a signal that is checked during evaluation
**		and will cause the interpreter to begin processing an escape
**		trap. Note that control must be passed back to REBOL for the
**		signal to be recognized and handled.
**
*/

#define RL_DO_STRING(a,b,c)         RL->do_string(a,b,c)
/*
**	int RL_Do_String(REBYTE *text, REBCNT flags, RXIARG *result)
**
**	Load a string and evaluate the resulting block.
**
**	Returns:
**		The datatype of the result.
**	Arguments:
**		text - A null terminated UTF-8 (or ASCII) string to transcode
**			into a block and evaluate.
**		flags - set to zero for now
**		result - value returned from evaluation.
**
*/

#define RL_DO_BINARY(a,b,c,d,e)     RL->do_binary(a,b,c,d,e)
/*
**	int RL_Do_Binary(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result)
**
**	Evaluate an encoded binary script such as compressed text.
**
**	Returns:
**		The datatype of the result or zero if error in the encoding.
**	Arguments:
**		bin - by default, a REBOL compressed UTF-8 (or ASCII) script.
**		length - the length of the data.
**		flags - special flags (set to zero at this time).
**		key - encoding, encryption, or signature key.
**		result - value returned from evaluation.
**	Notes:
**		As of A104, only compressed scripts are supported, however,
**		rebin, cloaked, signed, and encrypted formats will be supported.
**
*/

#define RL_DO_BLOCK(a,b,c)          RL->do_block(a,b,c)
/*
**	int RL_Do_Block(REBSER *blk, REBCNT flags, RXIARG *result)
**
**	Evaluate a block. (not implemented)
**
**	Returns:
**		The datatype of the result or zero if error in the encoding.
**	Arguments:
**		blk - A pointer to the block series
**		flags - set to zero for now
**		result - value returned from evaluation
**	Notes:
**		Not implemented. Contact Carl on R3 Chat if you think you
**		could use it for something.
**
*/

#define RL_DO_COMMANDS(a,b,c)       RL->do_commands(a,b,c)
/*
**	void RL_Do_Commands(REBSER *blk, REBCNT flags, REBCEC *context)
**
**	Evaluate a block of extension commands at high speed.
**
**	Returns:
**		Nothing
**	Arguments:
**		blk - a pointer to the block series
**		flags - set to zero for now
**		context - command evaluation context struct or zero if not used.
**	Notes:
**		For command blocks only, not for other blocks.
**		The context allows passing to each command a struct that is
**		used for back-referencing your environment data or for tracking
**		the evaluation block and its index.
**
*/

#define RL_PRINT(a,...)             RL->print(a,__VA_ARGS__)
/*
**	void RL_Print(const REBYTE *fmt, ...)
**
**	Low level print of formatted data to the console.
**
**	Returns:
**		nothing
**	Arguments:
**		fmt - A format string similar but not identical to printf.
**			Special options are available.
**		... - Values to be formatted.
**	Notes:
**		This function is low level and handles only a few C datatypes
**		at this time.
**
*/

#define RL_PRINT_TOS(a,b)           RL->print_tos(a,b)
/*
**	void RL_Print_TOS(REBCNT flags, REBYTE *marker)
**
**	Print top REBOL stack value to the console. (pending changes)
**
**	Returns:
**		Nothing
**	Arguments:
**		flags - special flags (set to zero at this time).
**		marker - placed at beginning of line to indicate output.
**	Notes:
**		This function is used for the main console evaluation
**		input loop to print the results of evaluation from stack.
**		The REBOL data stack is an abstract structure that can
**		change between releases. This function allows the host
**		to print the result of processed functions.
**		Note that what is printed is actually TOS+1.
**		Marker is usually "==" to show output.
**		The system/options/result-types determine which values
**		are automatically printed.
**
*/

#define RL_EVENT(a)                 RL->event(a)
/*
**	int RL_Event(REBEVT *evt)
**
**	Appends an application event (e.g. GUI) to the event port.
**
**	Returns:
**		Returns TRUE if queued, or FALSE if event queue is full.
**	Arguments:
**		evt - A properly initialized event structure. The
**			contents of this structure are copied as part of
**			the function, allowing use of locals.
**	Notes:
**		Sets a signal to get REBOL attention for WAIT and awake.
**		To avoid environment problems, this function only appends
**		to the event queue (no auto-expand). So if the queue is full
**
*/

#define RL_UPDATE_EVENT(a)          RL->update_event(a)
/*
**	int RL_Update_Event(REBEVT *evt)
**
**	Updates an application event (e.g. GUI) to the event port.
**
**	Returns:
**		Returns 1 if updated, or 0 if event appended, and -1 if full.
**	Arguments:
**		evt - A properly initialized event structure. The
**			 model and type of the event are used to address 
**           the unhandled event in the queue, when it is found,
**           it will be replaced with this one
**
*/

#define RL_MAKE_BLOCK(a)            RL->make_block(a)
/*
**	void *RL_Make_Block(u32 size)
**
**	Allocate a new block.
**
**	Returns:
**		A pointer to a block series.
**	Arguments:
**		size - the length of the block. The system will add one extra
**			for the end-of-block marker.
**	Notes:
**		Blocks are allocated with REBOL's internal memory manager.
**		Internal structures may change, so NO assumptions should be made!
**		Blocks are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
**		However, you can lock blocks to prevent deallocation. (?? default)
*/

#define RL_EXPAND_SERIES(a,b,c)     RL->expand_series(a,b,c)
/*
**	void RL_Expand_Series(REBSER *series, REBCNT index, REBCNT delta)
**
**	Expand a series at a particular index point by the number
**	number of units specified by delta.
**
**	Returns:
**		
**	Arguments:
**		series - series to expand
**		index - position where to expand
**		delta - number of UNITS to expand from TAIL (keeping terminator)
*/

#define RL_MAKE_STRING(a,b)         RL->make_string(a,b)
/*
**	void *RL_Make_String(u32 size, int unicode)
**
**	Allocate a new string or binary series.
**
**	Returns:
**		A pointer to a string or binary series.
**	Arguments:
**		size - the length of the string. The system will add one extra
**			for a null terminator (not strictly required, but good for C.)
**		unicode - set FALSE for ASCII/Latin1 strings, set TRUE for Unicode.
**	Notes:
**		Strings can be REBYTE or REBCHR sized (depends on R3 config.)
**		Strings are allocated with REBOL's internal memory manager.
**		Internal structures may change, so NO assumptions should be made!
**		Strings are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
**		However, you can lock strings to prevent deallocation. (?? default)
*/

#define RL_MAKE_IMAGE(a,b)          RL->make_image(a,b)
/*
**	void *RL_Make_Image(u32 width, u32 height)
**
**	Allocate a new image of the given size.
**
**	Returns:
**		A pointer to an image series, or zero if size is too large.
**	Arguments:
**		width - the width of the image in pixels
**		height - the height of the image in lines
**	Notes:
**		Images are allocated with REBOL's internal memory manager.
**		Image are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
*/

#define RL_MAKE_VECTOR(a,b,c,d)     RL->make_vector(a,b,c,d)
/*
**	int RL_Make_Vector(RXIARG *out, REBCNT type, REBINT cols, REBINT rows)
**
**	Allocate a new vector series with the given element type and shape.
**
**	Returns:
**		1 on success, 0 on failure. On failure `out` is left untouched.
**		Fails if `type` is not a valid element type, if `cols` is
**		negative, if `rows` is less than 1 or too large to encode, or
**		if the total element count cannot be allocated.
**	Arguments:
**		out:  RXIARG to receive the vector (series, index and info)
**		type: element type, one of the VT* values (VTSI08..VTSF64)
**		cols: number of columns (0 makes an empty vector)
**		rows: number of rows; 1 for an unshaped (plain) vector
**	Notes:
**		Contents are zero-filled. The element type and row count are
**		encoded together in `out->vector.info`; use the VECT_* macros
**		rather than unpacking it by hand. The VT* numbering is part of
**		the extension ABI and must not be reordered.
**
**		A vector with more than one row is length-locked, matching
**		vectors shaped from Rebol: rows travel with the value while the
**		buffer length is shared, so APPEND/INSERT/CLEAR on it will error.
**
**		Allocated with REBOL's internal memory manager and garbage
**		collected once no REBOL value references it. Until the returned
**		RXIARG reaches REBOL, nothing references the series -- do not
**		call other RL_ functions that may allocate in between.
*/

#define RL_PROTECT_GC(a,b)          RL->protect_gc(a,b)
/*
**	void RL_Protect_GC(REBSER *series, u32 flags)
**
**	Protect memory from garbage collection.
**
**	Returns:
**		nothing
**	Arguments:
**		series - a series to protect (block, string, image, ...)
**		flags - set to 1 to protect, 0 to unprotect
**	Notes:
**		You should only use this function when absolutely necessary,
**		because it bypasses garbage collection for the specified series.
**		Meaning: if you protect a series, it will never be freed.
**		Also, you only need this function if you allocate several series
**		such as strings, blocks, images, etc. within the same command
**		and you don't store those references somewhere where the GC can
**		find them, such as in an existing block or object (variable).
*/

#define RL_GET_STRING(a,b,c,d)      RL->get_string(a,b,c,d)
/*
**	int RL_Get_String(REBSER *series, u32 index, void **str, REBOOL needs_wide)
**
**	Obtain a pointer into a string (bytes or unicode).
**
**	Returns:
**		The length and type of string. When len > 0, string is unicode.
**		When len < 0, string is bytes.
**  Arguments:
**		series - string series pointer
**		index - index from beginning (zero-based)
**		str   - pointer to first character
**		needs_wide - unicode string is required, converts if needed
**	Notes:
**		If the len is less than zero, then the string is optimized to
**		codepoints (chars) 255 or less for ASCII and LATIN-1 charsets.
**		Strings are allowed to move in memory. Therefore, you will want
**		to make a copy of the string if needed.
*/

#define RL_GET_UTF8_STRING(a,b,c)   RL->get_utf8_string(a,b,c)
/*
**	int RL_Get_UTF8_String(REBSER *series, u32 index, void **str)
**
**	Obtain a pointer into an UTF8 encoded string.
**
**	Returns:
**		The length of string is bytes.
**  Arguments:
**		series - string series pointer
**		index - index from beginning (zero-based)
**		str   - pointer to first character
**	Notes:
**		Strings are allowed to move in memory. Therefore, you will want
**		to make a copy of the string if needed.
**
**	This function should be DEPRECATED! All strings should be UTF-8 encoded now.
*/

#define RL_MAP_WORD(a)              RL->map_word(a)
/*
**	u32 RL_Map_Word(REBYTE *string)
**
**	Given a word as a string, return its global word identifier.
**
**	Returns:
**		The word identifier that matches the string.
**	Arguments:
**		string - a valid word as a UTF-8 encoded string.
**	Notes:
**		Word identifiers are persistent, and you can use them anytime.
**		If the word is new (not found in master symbol table)
**		it will be added and the new word identifier is returned.
*/

#define RL_MAP_WORDS(a)             RL->map_words(a)
/*
**	u32 *RL_Map_Words(REBSER *series)
**
**	Given a block of word values, return an array of word ids.
**
**	Returns:
**		An array of global word identifiers (integers). The [0] value is the size.
**	Arguments:
**		series - block of words as values (from REBOL blocks, not strings.)
**	Notes:
**		Word identifiers are persistent, and you can use them anytime.
**		The block can include any kind of word, including set-words, lit-words, etc.
**		If the input block contains non-words, they will be skipped.
**		The array is allocated with OS_MAKE and you can OS_FREE it any time.
*/

#define RL_WORD_STRING(a)           RL->word_string(a)
/*
**	REBYTE *RL_Word_String(u32 word)
**
**	Return a string related to a given global word identifier.
**
**	Returns:
**		A copy of the word string, null terminated.
**	Arguments:
**		word - a global word identifier
**	Notes:
**		The result is a null terminated copy of the name for your own use.
**		The string is always UTF-8 encoded (chars > 127 are encoded.)
**		In this API, word identifiers are always canonical. Therefore,
**		the returned string may have different spelling/casing than expected.
**		The string is allocated with OS_MAKE and you can OS_FREE it any time.
*/

#define RL_FIND_WORD(a,b)           RL->find_word(a,b)
/*
**	u32 RL_Find_Word(u32 *words, u32 word)
**
**	Given an array of word ids, return the index of the given word.
**
**	Returns:
**		The index of the given word or zero.
**	Arguments:
**		words - a word array like that returned from MAP_WORDS (first element is size)
**		word - a word id
**	Notes:
**		The first element of the word array is the length of the array.
*/

#define RL_SERIES(a,b)              RL->series(a,b)
/*
**	REBUPT RL_Series(REBSER *series, REBCNT what)
**
**	Get series information.
**
**	Returns:
**		Returns information related to a series.
**	Arguments:
**		series - any series pointer (string or block)
**		what - indicates what information to return (see RXI_SER enum)
**	Notes:
**		Invalid what arg nums will return zero.
*/

#define RL_GET_CHAR(a,b)            RL->get_char(a,b)
/*
**	REBU32 RL_Get_Char(REBSER *series, u32 index)
**
**	Get a character from UTF8 encoded string.
**
**	Returns:
**		A Unicode character point from string. If index is
**		at or past the tail, a -1 is returned.
**	Arguments:
**		series - string series pointer
**		index - zero based index of character
**	Notes:
**		This function works for byte and unicoded strings.
**		The maximum size of a Unicode char is determined by
**		R3 build options. The default is 16 bits.
*/

#define RL_SET_CHAR(a,b,c)          RL->set_char(a,b,c)
/*
**	u32 RL_Set_Char(REBSER *series, u32 index, u32 chr)
**
**	Set a character into a byte or unicode string.
**
**	Returns:
**		The index passed as an argument.
**	Arguments:
**		series - string series pointer
**		index - where to store the character. If past the tail,
**			the string will be auto-expanded by one and the char
**			will be appended.
*/

#define RL_GET_VALUE_RESOLVED(a,b,c) RL->get_value_resolved(a,b,c)
/*
**	int RL_Get_Value_Resolved(REBSER *series, u32 index, RXIARG *result)
**
**	Get a value from a block. If value is WORD or PATH, than its value is resolved.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/

#define RL_GET_VALUE(a,b,c)         RL->get_value(a,b,c)
/*
**	int RL_Get_Value(REBSER *series, u32 index, RXIARG *result)
**
**	Get a value from a block.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/

#define RL_SET_VALUE(a,b,c,d)       RL->set_value(a,b,c,d)
/*
**	int RL_Set_Value(REBSER *series, u32 index, RXIARG val, int type)
**
**	Set a value in a block.
**
**	Returns:
**		TRUE if index past end and value was appended to tail of block.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		val  - new value for field
**		type - datatype of value
*/

#define RL_WORDS_OF_OBJECT(a)       RL->words_of_object(a)
/*
**	u32 *RL_Words_Of_Object(REBSER *obj)
**
**	Returns information about the object.
**
**	Returns:
**		Returns an array of words used as fields of the object.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**	Notes:
**		Returns a word array similar to MAP_WORDS (first element is size).
**		The array is allocated with OS_MAKE. You can OS_FREE it any time.
*/

#define RL_GET_FIELD(a,b,c)         RL->get_field(a,b,c)
/*
**	int RL_Get_Field(REBSER *obj, u32 word, RXIARG *result)
**
**	Get a field value (context variable) of an object.
**
**	Returns:
**		Datatype of value or zero if word is not found in the object.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**		word - global word identifier (integer)
**		result - gets set to the value of the field
*/

#define RL_SET_FIELD(a,b,c,d)       RL->set_field(a,b,c,d)
/*
**	int RL_Set_Field(REBSER *obj, u32 word, RXIARG val, int type)
**
**	Set a field (context variable) of an object.
**
**	Returns:
**		The type arg, or zero if word not found in object or if field is protected.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**		word - global word identifier (integer)
**		val  - new value for field
**		type - datatype of value
*/

#define RL_CALLBACK(a)              RL->callback(a)
/*
**	int RL_Callback(RXICBI *cbi)
**
**	Evaluate a REBOL callback function, either synchronous or asynchronous.
**
**	Returns:
**		Sync callback: type of the result; async callback: true if queued
**	Arguments:
**		cbi - callback information including special option flags,
**			object pointer (where function is located), function name
**			as global word identifier (within above object), argument list
**			passed to callback (see notes below), and result value.
**	Notes:
**		The flag value will determine the type of callback. It can be either
**		synchronous, where the code will re-enter the interpreter environment
**		and call the specified function, or asynchronous where an EVT_CALLBACK
**		event is queued, and the callback will be evaluated later when events
**		are processed within the interpreter's environment.
**		For asynchronous callbacks, the cbi and the args array must be managed
**		because the data isn't processed until the callback event is
**		handled. Therefore, these cannot be allocated locally on
**		the C stack; they should be dynamic (or global if so desired.)
**		See c:extensions-callbacks
*/

#define RL_ENCODE_UTF8(a,b,c,d,e,f) RL->encode_utf8(a,b,c,d,e,f)
/*
**	REBCNT RL_Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts)
**
**	Encode the unicode into UTF8 byte string.
**		
**	Returns:
**		Number of source chars used.
**		Updates len for dst bytes used.
**		Does not add a terminator.
**	Arguments:
**		max  - The maximum size of the result (UTF8).
**		uni  - Source string can be byte or unichar sized (uni = TRUE);
**		opts - Convert LF/CRLF
*/

#define RL_ENCODE_UTF8_STRING(a,b,c,d) RL->encode_utf8_string(a,b,c,d)
/*
**	REBSER* RL_Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts)
**
**	Encode the unicode into UTF8 byte string.
**
**	Returns:
**		Rebol series value with an UTF8 encoded data.
**	Arguments:
**		src  - series as a REBYTE or REBUNI.
**		len  - number of source bytes to convert.
**		uni  - Source string can be byte or unichar sized (uni = TRUE);
**		opts - Convert LF/CRLF
*/

#define RL_DECODE_UTF_STRING(a,b,c,d,e) RL->decode_utf_string(a,b,c,d,e)
/*
**	REBSER* RL_Decode_UTF_String(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni)
**
**	Decode the UTF8 encoded data into Rebol series.
**
**	Returns:
**		Rebol series with char size 1 or 2
**	Arguments:
**		src  - UTF8 encoded data
**		len  - number of source bytes to convert.
**		utf  - is 0, 8, +/-16, +/-32.
**		ccr  - Convert LF/CRLF
**		uni  - keep uni version even for plain ascii
**
**  Note:
**		uni is not used anymore! Instead there is REBCNT err,
**		which is used to report error position. But not used here,
**		because this function may be used by old extensions:/
*/

#define RL_REGISTER_HANDLE(a,b,c)   RL->register_handle(a,b,c)
/*
**	REBCNT RL_Register_Handle(const REBYTE *name, REBCNT size, void* free_func)
**
**	Stores handle's specification (required data size and optional free callback.
**
**	Returns:
**		symbol id of the word (whether found or new)
**		or NOT_FOUND if handle with give ID is already registered.
**	Arguments:
**		name      - handle's name as a c-string (length is being detected)
**		size      - size of needed memory to handle
**		free_func - custom function to be called when handle is released
**
*/

#define RL_MAKE_HANDLE_CONTEXT(a)   RL->make_handle_context(a)
/*
**	REBHOB* RL_Make_Handle_Context(REBCNT sym)
**
**	Allocates memory large enough to hold given handle's id
**
**	Returns:
**		A pointer to a Rebol's handle value.
**	Arguments:
**		sym - handle's word id
**
*/

#define RL_FREE_HANDLE_CONTEXT(a)   RL->free_handle_context(a)
/*
**	void RL_Free_Handle_Context(REBHOB *hob)
**
**	Frees memory of given handle's context
**
**	Returns:
**		nothing
**	Arguments:
**		hob - handle's context
**
*/

#define RL_DECODE_UTF8_CHAR(a,b)    RL->decode_utf8_char(a,b)
/*
**	REBCNT RL_Decode_UTF8_Char(const REBYTE *str, REBCNT *len)
**
**	Converts a single UTF8 code-point (to 32 bit).
**
**	Returns:
**		32 bit character code
**	Arguments:
**		src  - UTF8 encoded data
**		len  - number of source bytes consumed.
*/

#define RL_REGISTER_HANDLE_SPEC(a,b) RL->register_handle_spec(a,b)
/*
**	REBCNT RL_Register_Handle_Spec(const REBYTE *name, REBHSP *spec)
**
**	Stores handle's specification (required data size and optional callbacks).
**  It's an extended version of old RL_Register_Handle function.
**
**	Returns:
**		symbol id of the word (whether found or new)
**		or NOT_FOUND if handle with give ID is already registered.
**	Arguments:
**		name      - handle's name as a c-string (length is being detected)
**		spec      - Handle's specification:
**                  * size of needed memory to handle,
**                  * reserved flags
**                  * release function
**                  * get path accessor
**                  * set path accessor
**
*/

#define RL_TO_LOCAL_PATH(a,b,c)     RL->to_local_path(a,b,c)
/*
**	REBSER* RL_To_Local_Path(RXIARG *file, REBFLG full, REBFLG utf8)
**
**	Convert REBOL filename to a local filename.
**
**	Returns:
**		A new series with the converted path or 0 on error.
**	Arguments:
**		file - Rebol file as an extension argument (series + index)
**		full - prepend current directory
**		utf8 - convert to UTF-8 if needed
**
*/

#define RL_TO_REBOL_PATH(a,b,c)     RL->to_rebol_path(a,b,c)
/*
**	REBSER* RL_To_Rebol_Path(void *src, REBCNT len, REBINT uni)
**
**	Convert local filename to a REBOL filename.
**
**	Returns:
**		A new series with the converted path or 0 on error.
**	Arguments:
**		ser - series as a REBYTE or REBUNI.
**		len - number of source bytes consumed.
**		uni - if series is REBYTE (0) or REBUNI (1)
**
*/

#define RL_STRUCT_SPEC(a)           RL->struct_spec(a)
/*
**	REBSER* RL_Struct_Spec(REBCNT id)
**
**	Get struct specification.
**
**	Returns:
**		Returns a struct specification data.
**	Arguments:
**		id - unique struct id (counted as a hash of the specification)
*/

#define RL_ENCODE_UTF8_CHAR(a,b)    RL->encode_utf8_char(a,b)
/*
**	REBCNT RL_Encode_UTF8_Char(REBYTE *dst, REBU32 chr)
**
**	Converts a single char to UTF8 code-point.
**
**	Returns:
**		Returns length of char stored in dst
**	Arguments:
**		dst  - UTF8 encoded data
**		chr  - Unicode character.
*/

#define RL_MEM_ALLOC(a,b)           RL->mem_alloc(a,b)
/*
**	void* RL_Mem_Alloc(void *opaque, size_t size)
**
**	Allocates memory either using a memory pool or standard dynamic memory allocation.
**	It keeps info about this memory and checks if memory use is not over policy.
**	Such allocated memory must be freed using `RL_Mem_Free` function!
**
**	Returns:
**		Pointer to the newly allocated memory
**	Arguments:
**		opaque  - unused
**		size    - required memory size
*/

#define RL_MEM_FREE(a,b)            RL->mem_free(a,b)
/*
**	void RL_Mem_Free(void* opaque, void* address)
**
**	Frees memory allocated using the `RL_Mem_Alloc` function.
**
**	Returns:
**		nothing
**	Arguments:
**		opaque  - unused
**		address - memory address to be freed
*/

#define RL_REGISTER_COMPRESS_METHOD(a,b,c) RL->register_compress_method(a,b,c)
/*
**	int RL_Register_Compress_Method(const REBYTE* name, COMPRESS_FUNC encoder, DECOMPRESS_FUNC decoder)
**
**	Register external compression functions.
**
**	Returns:
**		TRUE if succesfully registered.
**	Arguments:
**		name - null ternimanted name of the compression method
**		encoder - external compress function
*/

#define RL_STRUCT_INFO(a,b)         RL->struct_info(a,b)
/*
**	REBFLG RL_Struct_Info(RXIARG *arg, RXISTRU *out)
**
**	Resolve a struct argument into a directly usable description.
**
**	A struct value is a view into a shared data series, so its size and
**	flags live in the specification, not in the series. This performs the
**	spec lookup once and validates that the view fits into the data.
**
**	Returns:
**		TRUE when the argument describes a usable struct, else FALSE
**		(no data series, unknown spec id, or the view runs past the data).
**	Arguments:
**		arg - struct argument as received in a command frame
**		out - filled with data pointer, size, count, id, flags, field list
*/

#define RL_MAKE_STRUCT(a,b,c)       RL->make_struct(a,b,c)
/*
**	REBFLG RL_Make_Struct(RXIARG *out, REBCNT id, RXISTRU *info)
**
**	Create a new struct value of an already registered specification.
**
**	The specification must already exist in system/catalog/structs, where
**	Prepare_Struct interns it when Rebol code evaluates `make struct!
**	[...]` - so an extension instantiates a shape its own module declared
**	rather than defining one in C. The data series is allocated zeroed and
**	is owned by the struct.
**
**	The zeroing is required, not incidental: a zeroed `rebval!` field reads
**	as END, which get_scalar reports as none and Mark_Struct_Fields skips.
**	Uninitialized bytes there would be marked by the GC as garbage values.
**
**	Like RL_Make_String and RL_Make_Block, the result is protected from the
**	GC only as a recently allocated series. Store it into a command frame
**	argument and return it; do not hold it across a large number of other
**	allocations.
**
**	Returns:
**		TRUE when the struct was created, else FALSE (unknown spec id or
**		a zero-sized specification).
**	Arguments:
**		out  - command frame argument which receives the new struct
**		id   - spec id (hash of the specification block)
**		info - filled like RL_Struct_Info; may be NULL
*/

#define RL_ALLOC(a)                 RL->alloc(a)
/*
**	void* RL_Alloc(size_t size)
**
**	Allocate memory that the interpreter itself may free.
**
**	Unlike RL_Mem_Alloc, this is the interpreter's plain accounted
**	allocator - the same one used internally by Make_Mem - so a buffer
**	handed back to the core (for example a codec's output, freed by
**	DO-CODEC with a known size) must come from here, not from
**	RL_Mem_Alloc, whose result carries a hidden header and may live in
**	a memory pool. Free with RL_Free and the size passed to RL_Alloc.
**
**	Returns:
**		Pointer to uninitialized memory, or 0 on failure.
**	Arguments:
**		size - number of bytes
*/

#define RL_FREE(a,b)                RL->free(a,b)
/*
**	void RL_Free(void *mem, size_t size)
**
**	Frees memory allocated with RL_Alloc. The size must match.
**
**	Returns:
**		nothing
**	Arguments:
**		mem  - pointer to initialized memory
**		size - number of bytes
*/

#define RL_REGISTER_DEVICE(a,b)     RL->register_device(a,b)
/*
**	int RL_Register_Device(REBDEV *dev, u32 dev_size)
**
**	Add a device to the device table at run time.
**
**	Returns:
**		The assigned device id (>= RDI_MAX), or a negative RDR_ code:
**		RDR_TABLE_FULL, RDR_BAD_DEVICE or RDR_BAD_REBDEV.
**	Arguments:
**		dev      - device structure, usually from DEFINE_DEV
**		dev_size - sizeof(REBDEV) as the caller sees it
**	Notes:
**		The device and its command table must stay valid for the life
**		of the process - devices cannot be unregistered, so a library
**		which registers one must not be unloaded.
**
**		Pass the id as req->device to reach the device; events are
**		posted with EVM_DEVICE and evt.req, as for built-in devices.
*/

#define RL_PORT_STATE(a,b)          RL->port_state(a,b)
/*
**	REBREQ *RL_Port_State(REBSER *port, REBCNT device)
**
**	Get the native request of a port, creating it if it does not exist.
**
**	This is the handle the event machinery recognizes: the request it
**	returns has req->port set, so an EVM_DEVICE event carrying it wakes
**	that port. An extension must not improvise one - Pending_Port reads
**	port/state as a REBREQ without checking the handle type, so a handle
**	of any other type parked there is read as a request.
**
**	Returns:
**		The port's request, or 0 if `port` is not a usable port frame.
**	Arguments:
**		port   - port frame, as received in a port! command argument
**		device - device id from RL_Register_Device
**	Notes:
**		The state is sized sizeof(REBREQ) for every device (see
**		Init_Ports), so a device needing more must keep it behind
**		req->handle, the way dev-midi.c does.
**
**		Allocating the handle can trap on out of memory, which longjmps
**		out of the extension - do not hold resources that only a return
**		would release across this call.
*/

#define RL_DO_DEVICE(a,b)           RL->do_device(a,b)
/*
**	int RL_Do_Device(REBREQ *req, REBCNT command)
**
**	Run a device command, as OS_Do_Device does for the built-in ports.
**
**	Returns:
**		=0 command succeeded
**		>0 command is pending (the request was attached for polling)
**		<0 command failed; req->error holds the code
**	Arguments:
**		req     - request, usually from RL_Port_State
**		command - RDC_ command code
**	Notes:
**		Only devices added by RL_Register_Device can be driven this way.
**		The built-in devices are reached through their own port schemes,
**		which apply the security policy (Secure_Port) before dispatching;
**		an extension must not route around that.
*/



#define RL_MAKE_BINARY(s) RL_MAKE_STRING(s, FALSE)

#ifndef REB_EXT // not extension lib, use direct calls to r3lib

RL_API void RL_Version(REBYTE vers[]);
RL_API REBARGS* RL_Init(int argc, char **argv);
RL_API void RL_Dispose(void);
RL_API int RL_Start(REBYTE *bin, REBINT len, REBCNT flags);
RL_API void RL_Reset(void);
RL_API void *RL_Extend(REBYTE *source, RXICAL call);
RL_API void RL_Escape(REBINT reserved);
RL_API int RL_Do_String(REBYTE *text, REBCNT flags, RXIARG *result);
RL_API int RL_Do_Binary(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result);
RL_API int RL_Do_Block(REBSER *blk, REBCNT flags, RXIARG *result);
RL_API void RL_Do_Commands(REBSER *blk, REBCNT flags, REBCEC *context);
RL_API void RL_Print(const REBYTE *fmt, ...);
RL_API void RL_Print_TOS(REBCNT flags, REBYTE *marker);
RL_API int RL_Event(REBEVT *evt);
RL_API int RL_Update_Event(REBEVT *evt);
RL_API void *RL_Make_Block(u32 size);
RL_API void RL_Expand_Series(REBSER *series, REBCNT index, REBCNT delta);
RL_API void *RL_Make_String(u32 size, int unicode);
RL_API void *RL_Make_Image(u32 width, u32 height);
RL_API int RL_Make_Vector(RXIARG *out, REBCNT type, REBINT cols, REBINT rows);
RL_API void RL_Protect_GC(REBSER *series, u32 flags);
RL_API int RL_Get_String(REBSER *series, u32 index, void **str, REBOOL needs_wide);
RL_API int RL_Get_UTF8_String(REBSER *series, u32 index, void **str);
RL_API u32 RL_Map_Word(REBYTE *string);
RL_API u32 *RL_Map_Words(REBSER *series);
RL_API REBYTE *RL_Word_String(u32 word);
RL_API u32 RL_Find_Word(u32 *words, u32 word);
RL_API REBUPT RL_Series(REBSER *series, REBCNT what);
RL_API REBU32 RL_Get_Char(REBSER *series, u32 index);
RL_API u32 RL_Set_Char(REBSER *series, u32 index, u32 chr);
RL_API int RL_Get_Value_Resolved(REBSER *series, u32 index, RXIARG *result);
RL_API int RL_Get_Value(REBSER *series, u32 index, RXIARG *result);
RL_API int RL_Set_Value(REBSER *series, u32 index, RXIARG val, int type);
RL_API u32 *RL_Words_Of_Object(REBSER *obj);
RL_API int RL_Get_Field(REBSER *obj, u32 word, RXIARG *result);
RL_API int RL_Set_Field(REBSER *obj, u32 word, RXIARG val, int type);
RL_API int RL_Callback(RXICBI *cbi);
RL_API REBCNT RL_Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts);
RL_API REBSER* RL_Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts);
RL_API REBSER* RL_Decode_UTF_String(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni);
RL_API REBCNT RL_Register_Handle(const REBYTE *name, REBCNT size, void* free_func);
RL_API REBHOB* RL_Make_Handle_Context(REBCNT sym);
RL_API void RL_Free_Handle_Context(REBHOB *hob);
RL_API REBCNT RL_Decode_UTF8_Char(const REBYTE *str, REBCNT *len);
RL_API REBCNT RL_Register_Handle_Spec(const REBYTE *name, REBHSP *spec);
RL_API REBSER* RL_To_Local_Path(RXIARG *file, REBFLG full, REBFLG utf8);
RL_API REBSER* RL_To_Rebol_Path(void *src, REBCNT len, REBINT uni);
RL_API REBSER* RL_Struct_Spec(REBCNT id);
RL_API REBCNT RL_Encode_UTF8_Char(REBYTE *dst, REBU32 chr);
RL_API void* RL_Mem_Alloc(void *opaque, size_t size);
RL_API void RL_Mem_Free(void* opaque, void* address);
RL_API int RL_Register_Compress_Method(const REBYTE* name, COMPRESS_FUNC encoder, DECOMPRESS_FUNC decoder);
RL_API REBFLG RL_Struct_Info(RXIARG *arg, RXISTRU *out);
RL_API REBFLG RL_Make_Struct(RXIARG *out, REBCNT id, RXISTRU *info);
RL_API void* RL_Alloc(size_t size);
RL_API void RL_Free(void *mem, size_t size);
RL_API int RL_Register_Device(REBDEV *dev, u32 dev_size);
RL_API REBREQ *RL_Port_State(REBSER *port, REBCNT device);
RL_API int RL_Do_Device(REBREQ *req, REBCNT command);

#endif


// File: reb-ext-common.h
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012-2026 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
************************************************************************
**
**  Summary: Convenience macros shared by extension command sources
**  Module:  reb-ext-common.h
**  Author:  Oldes
**  Notes:
**      Deliberately includes nothing. The extension decides which
**      header provides the API - `reb-host.h` for an extension built
**      inside this repository (either embedded or as a library), or
**      the amalgamated `rebol-extension.h` for one built out of tree.
**      List both in the specification's `c-include:`, API header first.
**
***********************************************************************/

#ifndef REB_EXT_COMMON_H
#define REB_EXT_COMMON_H

// Return type of every command function. The dispatch table requires
// them all to share this signature; see the generated commands table.
#define COMMAND int


/***********************************************************************
**  Argument access
**
**  `frm` is the command frame; argument numbering is 1-based and counts
**  refinements as slots of their own, with a refinement's own arguments
**  following it. For `foo: command [a /part len]`, `a` is 1, `/part`
**  is 2 and `len` is 3.
***********************************************************************/

#define ARG_Double(n)         RXA_DEC64(frm,n)
#define ARG_Float(n)          (float)RXA_DEC64(frm,n)
#define ARG_Int32(n)          RXA_INT32(frm,n)
#define ARG_Int64(n)          RXA_INT64(frm,n)
#define ARG_Handle_Series(n)  RXA_HANDLE_CONTEXT(frm, n)->series

// Series argument which may legitimately be none.
#define OPT_SERIES(n)         (RXA_TYPE(frm,n) == RXT_NONE ? NULL : RXA_SERIES(frm, n))

// Tests that argument `n` is a handle of the registered type `t`.
#define FRM_IS_HANDLE(n, t)   (RXA_TYPE(frm,n) == RXT_HANDLE && RXA_HANDLE_TYPE(frm, n) == t)


/***********************************************************************
**  Returning
***********************************************************************/

// Returns an error described by a null terminated C string.
//
// NOTE: on this path args[1].series is NOT a series - the interpreter
// reads it back as a `const REBYTE*`, measures it with LEN_BYTES and
// copies it into a fresh Rebol string, so a static string is correct
// and its lifetime is not an issue. The cast only silences the type.
#define RETURN_ERROR(err) \
	do { RXA_SERIES(frm, 1) = (REBSER*)(err); return RXR_ERROR; } while(0)

#define RETURN_HANDLE(hob)                   \
	RXA_HANDLE(frm, 1)       = hob;          \
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;     \
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;   \
	RXA_TYPE(frm, 1) = RXT_HANDLE;           \
	return RXR_VALUE


/***********************************************************************
**  String building
**
**  Appends printf-formatted text to a Rebol string series, expanding it
**  when needed. Requires an `int len` (or REBLEN/size_t) in scope.
***********************************************************************/

#define APPEND_STRING(str, ...) \
	len = snprintf(NULL,0,__VA_ARGS__);\
	if (len > (int)(SERIES_REST(str)-SERIES_LEN(str))) {\
		RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
		SERIES_TAIL(str) -= len;\
	}\
	len = snprintf( \
		SERIES_TEXT(str)+SERIES_TAIL(str),\
		SERIES_REST(str)-SERIES_TAIL(str),\
		__VA_ARGS__\
	);\
	SERIES_TAIL(str) += len;


/***********************************************************************
**  Tracing (compiled out unless USE_TRACES is defined)
***********************************************************************/

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str)
#endif

#endif // REB_EXT_COMMON_H
