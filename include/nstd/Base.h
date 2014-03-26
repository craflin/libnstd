
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

typedef bool bool_t;
typedef void void_t;

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef unsigned char byte_t;
typedef char char_t;
typedef unsigned char uchar_t;
typedef int int_t;
typedef unsigned int uint_t;

#ifdef _AMD64
#ifdef _WIN32
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef unsigned long long int size_t;
typedef long long int ssize_t;
#else
typedef long int int64_t;
typedef unsigned long int uint64_t;
typedef unsigned long int size_t;
typedef long int ssize_t;
#endif
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef unsigned int size_t;
typedef int ssize_t;
#endif

#ifdef _UNICODE
typedef wchar_t tchar_t;
#ifndef _T
#define  _T(text) L ## text
#endif
#else
typedef char_t tchar_t;
#ifndef _T
#define  _T(text) text
#endif
#endif

typedef long long int timestamp_t;

void_t* operator new(size_t size);
void_t* operator new [](size_t size);
void_t operator delete(void_t* buffer);
void_t operator delete[](void_t* buffer);

inline void_t* operator new(size_t, void_t* buffer) {return buffer;}
inline void_t operator delete(void_t* p, void_t*) {}

#ifdef _UNICODE
#define __WFILE__T(x) _T(x)
#define __WFILE__ __WFILE__T(__FILE__)
#define __TFILE__ __WFILE__
#else
#define __TFILE__ __FILE__
#endif
