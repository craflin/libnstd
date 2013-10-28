
#pragma once

typedef bool bool_t;
typedef void void_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef unsigned char byte_t;
typedef char char_t;
typedef unsigned char uchar_t;
typedef int int_t;
typedef unsigned int uint_t;

#if defined(_M_AMD64) || defined(__amd64__)
typedef unsigned long long int size_t;
typedef long long int ssize_t;
#else
typedef unsigned int size_t;
typedef int ssize_t;
#endif

typedef int64_t time_t;

void_t* operator new(size_t size);
void_t* operator new [](size_t size);
void_t operator delete(void_t* buffer);
void_t operator delete[](void_t* buffer);
