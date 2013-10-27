
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

/*
static inline size_t hashCode(int8_t val) {return val;}
static inline size_t hashCode(int16_t val) {return val;}
static inline size_t hashCode(int32_t val) {return val;}
#if defined(_M_AMD64) || defined(__amd64__)
static inline size_t hashCode(int64_t val) {return val;}
#else
static inline size_t hashCode(int64_t val) {return (size_t)(val >> 32) ^ (size_t)val;}
#endif

static inline size_t hashCode(uint8_t val) {return val;}
static inline size_t hashCode(uint16_t val) {return val;}
static inline size_t hashCode(uint32_t val) {return val;}
#if defined(_M_AMD64) || defined(__amd64__)
static inline size_t hashCode(uint64_t val) {return val;}
#else
static inline size_t hashCode(uint64_t val) {return (size_t)(val >> 32) ^ (size_t)val;}
#endif

static inline size_t hashCode(bool_t val) {return val;}
*/