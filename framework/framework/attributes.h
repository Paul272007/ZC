#pragma once

#ifdef __cplusplus
  #define ZC_BEGIN_DECLS extern "C" {
  #define ZC_END_DECLS   }
#else
  #define ZC_BEGIN_DECLS
  #define ZC_END_DECLS
#endif

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif

#ifndef __has_c_attribute
  #define __has_c_attribute(x) 0
#endif

#ifndef __has_cpp_attribute
  #define __has_cpp_attribute(x) 0
#endif

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#ifdef __cplusplus
  #define ZC_HAS_STD_ATTR(x) __has_cpp_attribute(x)
#else
  #define ZC_HAS_STD_ATTR(x) __has_c_attribute(x)
#endif

#if ZC_HAS_STD_ATTR(nodiscard)
  #define ZC_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_NODISCARD __attribute__((__warn_unused_result__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
  #define ZC_NODISCARD _Check_return_
#else
  #define ZC_NODISCARD
#endif

#if ZC_HAS_STD_ATTR(maybe_unused)
  #define ZC_MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_MAYBE_UNUSED __attribute__((__unused__))
#else
  #define ZC_MAYBE_UNUSED
#endif

#if ZC_HAS_STD_ATTR(deprecated)
  #define ZC_DEPRECATED          [[deprecated]]
  #define ZC_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_DEPRECATED          __attribute__((__deprecated__))
  #define ZC_DEPRECATED_MSG(msg) __attribute__((__deprecated__(msg)))
#elif defined(_MSC_VER)
  #define ZC_DEPRECATED          __declspec(deprecated)
  #define ZC_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#else
  #define ZC_DEPRECATED
  #define ZC_DEPRECATED_MSG(msg)
#endif

#if ZC_HAS_STD_ATTR(noreturn)
  #define ZC_NORETURN [[noreturn]]
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_NORETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
  #define ZC_NORETURN __declspec(noreturn)
#else
  #define ZC_NORETURN
#endif

#if ZC_HAS_STD_ATTR(fallthrough)
  #define ZC_FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_FALLTHROUGH __attribute__((__fallthrough__))
#else
  #define ZC_FALLTHROUGH ((void)0)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define ZC_CONSTRUCTOR         __attribute__((__constructor__))
  #define ZC_CONSTRUCTOR_PRIO(p) __attribute__((__constructor__(p)))
  #define ZC_DESTRUCTOR          __attribute__((__destructor__))
  #define ZC_DESTRUCTOR_PRIO(p)  __attribute__((__destructor__(p)))
#else
  #define ZC_CONSTRUCTOR
  #define ZC_CONSTRUCTOR_PRIO(p)
  #define ZC_DESTRUCTOR
  #define ZC_DESTRUCTOR_PRIO(p)
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) ||      \
  (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L))
  #define ZC_ALIGNED(n) alignas(n)
#elif defined(__GNUC__) || defined(__clang__)
  #define ZC_ALIGNED(n) __attribute__((__aligned__(n)))
#elif defined(_MSC_VER)
  #define ZC_ALIGNED(n) __declspec(align(n))
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  #include <stdalign.h>
  #define ZC_ALIGNED(n) alignas(n)
#else
  #define ZC_ALIGNED(n)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define ZC_PACKED __attribute__((__packed__))
#elif defined(_MSC_VER)
  #define ZC_PACKED /* With MSVC, prefer #pragma pack(push, 1) around struct */
#else
  #define ZC_PACKED
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define ZC_EXPORTED      __attribute__((__visibility__("default")))
  #define ZC_HIDDEN        __attribute__((__visibility__("hidden")))
  #define ZC_SECTION(name) __attribute__((__section__(name)))
  #define ZC_ALIAS(target) __attribute__((__alias__(#target)))
  #define ZC_USED          __attribute__((__used__))
  #define ZC_ALWAYS_INLINE inline __attribute__((__always_inline__))
  #define ZC_NOINLINE      __attribute__((__noinline__))
#elif defined(_MSC_VER)
  #define ZC_EXPORTED __declspec(dllexport)
  #define ZC_HIDDEN
  #define ZC_SECTION(name) __declspec(allocate(name))
  #define ZC_ALIAS(target)
  #define ZC_USED
  #define ZC_ALWAYS_INLINE __forceinline
  #define ZC_NOINLINE      __declspec(noinline)
#else
  #define ZC_EXPORTED
  #define ZC_HIDDEN
  #define ZC_SECTION(name)
  #define ZC_ALIAS(target)
  #define ZC_USED
  #define ZC_ALWAYS_INLINE inline
  #define ZC_NOINLINE
#endif

#if __has_attribute(pure) || defined(__GNUC__) || defined(__clang__)
  #define ZC_PURE __attribute__((__pure__))
#else
  #define ZC_PURE
#endif

#if __has_attribute(const) || defined(__GNUC__) || defined(__clang__)
  #define ZC_CONST_FN __attribute__((__const__))
#else
  #define ZC_CONST_FN
#endif

#if __has_attribute(cold) || defined(__GNUC__) || defined(__clang__)
  #define ZC_COLD __attribute__((__cold__))
#else
  #define ZC_COLD
#endif

#if __has_attribute(returns_nonnull) || (defined(__GNUC__) && (__GNUC__ >= 5))
  #define ZC_RETURNS_NONNULL __attribute__((__returns_nonnull__))
#else
  #define ZC_RETURNS_NONNULL
#endif

#if __has_attribute(format) || defined(__GNUC__) || defined(__clang__)
  #define ZC_FORMAT_PRINTF(fmt_idx, arg_idx) __attribute__((__format__(__printf__, fmt_idx, arg_idx)))
#else
  #define ZC_FORMAT_PRINTF(fmt_idx, arg_idx)
#endif

#if __has_builtin(__builtin_expect) || defined(__GNUC__) || defined(__clang__)
  #define ZC_LIKELY(x)   __builtin_expect(!!(x), 1)
  #define ZC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define ZC_LIKELY(x)   (x)
  #define ZC_UNLIKELY(x) (x)
#endif

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__) || defined(__clang__)
  #define ZC_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
  #define ZC_UNREACHABLE() __assume(0)
#else
  #define ZC_UNREACHABLE() ((void)0)
#endif

#define ZC_CONCAT_IMPL(x, y) x##y
#define ZC_CONCAT(x, y)      ZC_CONCAT_IMPL(x, y)
#define ZC_STRING_IMPL(x)    #x
#define ZC_STRING(x)         ZC_STRING_IMPL(x)
