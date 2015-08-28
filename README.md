libnstd
=======

libnstd is a cross platform non-standard standard library for C++. In contrast to the Standard Template
Library its aim is to provide frequently used features with minimalistic classes and as little usage of templates
as possible. Right now it consists of following headers:

 * Base.h: Basic data types and allocation operators.
 * Debug.h: Low level debugging tools.
 * Memory.h: Memory allocation and other memory tools.
 * Error.h: Abstraction layer for operating system errors.
 * Math.h: Basic mathematics functions.
 * Data types:
    * String.h: A lazy copying string class.
    * Variant.h: A variable data type for strings, lists, maps, integers and floats.
    * Buffer.h: A dynamic byte buffer.
    * Time.h: Date and time functions.
 * Containers
    * List.h: A double linked list.
    * HashMap.h: An associative container based on a hash map with fixed table size.
    * HashSet.h: 
    * Map.h, MultiMap.h: An associative container based on a balanced tree.
    * Array.h: A dynamically growing container based on an array.
 * Concurrency
    * Thread.h: Abstraction layer for native multi-threading.
    * Atomic.h: Abstraction layer for atomic functions.
    * Mutex.h: Abstraction layer for native mutual exclusion features.
    * Semaphore.h: Abstraction layer for a native semaphore.
    * Signal.h: Abstraction layer for native signals.
 * I/O
    * File.h: Abstraction layer for file I/O.
    * Directory.h: Abstraction layer for directory access.
    * Console.h: 
 * Processes:
    * Process.h:
    * Library.h:

Design Principles
-----------------

 * Be as much optimized as currently possible. (Unfortunately I do not have the time to optimize the shit out of each function, but some functions area already pretty optimized.)
 * Do not use macros except for code that should be excluded in release builds.
 * Use templates only when it is required to improve performance.
 * Do not include system header files in libnstd header files.
 * Design class interfaces to be minimalistic and easy to use.
 * Do not use exceptions.
 * Avoid dynamic memory allocation when possible.
 
Supported Platforms
-------------------

 * Windows x86
 * Windows x86 Unicode
 * Windows x86_64
 * Windows x86_64 Unicode
 * Cygwin i686
 * Linux i686
 * Linux x86_64
 * Linux armv61
