
#pragma once

#if (defined(_M_AMD64) || defined(__amd64__)) && !defined(_AMD64)
#define _AMD64
#endif
#if defined(__arm__)  && !defined(_ARM)
#define _ARM
#endif
#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

typedef signed char int8;
typedef short int16;
typedef int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned int uint;

#ifdef _AMD64
#ifdef _WIN32
typedef long long int int64;
typedef unsigned long long int uint64;
typedef unsigned long long int usize;
typedef long long int ssize;
#else
typedef long int int64;
typedef unsigned long int uint64;
typedef unsigned long int usize;
typedef long int ssize;
#endif
#else
typedef long long int int64;
typedef unsigned long long int uint64;
typedef unsigned int usize;
typedef int ssize;
#endif

#ifdef _UNICODE
typedef wchar_t tchar;
typedef unsigned short utchar;
#ifndef _T
#define  _T(text) L ## text
#endif
#else
typedef char tchar;
typedef unsigned char utchar;
#ifndef _T
#define  _T(text) text
#endif
#endif

void* operator new(usize size);
void* operator new [](usize size);
void operator delete(void* buffer);
void operator delete[](void* buffer);

inline void* operator new(usize, void* buffer) {return buffer;}
inline void operator delete(void* p, void*) {}

#ifdef _UNICODE
#define __WFILE__T(x) _T(x)
#define __WFILE__ __WFILE__T(__FILE__)
#define __TFILE__ __WFILE__
#else
#define __TFILE__ __FILE__
#endif

inline usize hash(int8 v) {return (usize)v;}
inline usize hash(uint8 v) {return (usize)v;}
inline usize hash(int16 v) {return (usize)v;}
inline usize hash(uint16 v) {return (usize)v;}
inline usize hash(int32 v) {return (usize)v;}
inline usize hash(uint32 v) {return (usize)v;}
inline usize hash(int64 v) {return (usize)v;}
inline usize hash(uint64 v) {return (usize)v;}
inline usize hash(void* v) {return (usize)v >> (sizeof(void*) / 4 + 1);}
#if defined(_WIN32) || !defined(_AMD64)
inline usize hash(long v) {return (usize)v;}
inline usize hash(unsigned long v) {return (usize)v;}
#endif
