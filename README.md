libnstd
=======

[![Build Status](http://iocp.hopto.org:8080/buildStatus/icon?job=craflin%2Flibnstd%2Fmaster)](http://iocp.hopto.org:8080/job/craflin/job/libnstd/job/master/)

libnstd is a cross platform non-standard standard library replacement for C++. In contrast to the Standard Template
Library (STL) its aim is to provide frequently used features with minimalistic classes and as little usage of templates
as possible. Code written with libnstd should be easily understandable for everyone with basic knowledge of imperative 
programming and common design patterns. Right now it consists of following headers:

* [Base.hpp](include/nstd/Base.hpp): Basic data types and allocation operators.
* [Debug.hpp](include/nstd/Debug.hpp): Low level debugging tools.
* [Memory.hpp](include/nstd/Memory.hpp): Memory allocation and other memory tools.
* [Error.hpp](include/nstd/Error.hpp): Abstraction layer of operating system error reporting.
* [Math.hpp](include/nstd/Math.hpp): Basic mathematics functions.
* Data types
    * [String.hpp](include/nstd/String.hpp): A lazy copying string class.
    * [Variant.hpp](include/nstd/Variant.hpp): A lazy copying variable data type for strings, lists, maps, integers and floats.
    * [Buffer.hpp](include/nstd/Buffer.hpp): A dynamic byte buffer.
    * [Time.hpp](include/nstd/Time.hpp): Date and time functions.
* Containers
    * [List.hpp](include/nstd/List.hpp): A double linked list.
    * [HashMap.hpp](include/nstd/HashMap.hpp): An associative container based on a hash map with fixed table size.
    * [HashSet.hpp](include/nstd/HashSet.hpp):  A container for unique elements based on a hash set with fixed table size.
    * [Map.hpp](include/nstd/Map.hpp), [MultiMap.hpp](include/nstd/MultiMap.hpp): An associative container based on a balanced tree.
    * [Array.hpp](include/nstd/Array.hpp): A dynamically growing container based on an array.
    * [PoolList.hpp](include/nstd/PoolList.hpp): An iterable object pool for non-copyable objects.
    * [PoolMap.hpp](include/nstd/PoolMap.hpp): An associative object pool for non-copyable objects.
    * [RefCount.hpp](include/nstd/RefCount.hpp): Tools for object reference counting.
* Concurrency
    * [Thread.hpp](include/nstd/Thread.hpp): Abstraction layer of native multi-threading features.
    * [Atomic.hpp](include/nstd/Atomic.hpp): Abstraction layer of atomic functions.
    * [Mutex.hpp](include/nstd/Mutex.hpp): Abstraction layer of a native mutual exclusion feature.
    * [Semaphore.hpp](include/nstd/Semaphore.hpp): Abstraction layer of a native semaphore construct.
    * [Signal.hpp](include/nstd/Signal.hpp): Abstraction layer of a native thread signal construct.
    * [Monitor.hpp](include/nstd/Monitor.hpp): Abstraction layer of a native monitor construct.
    * [Future.hpp](include/nstd/Future.hpp): Asynchronous function calls with global thread pool.
* Input/Output
    * [File.hpp](include/nstd/File.hpp): Abstraction layer of file input/output.
    * [Directory.hpp](include/nstd/Directory.hpp): Abstraction layer to access directories.
    * [Console.hpp](include/nstd/Console.hpp): Helper function for Console I/O and command prompting.
    * [Log.hpp](include/nstd/Log.hpp): Console logging helper functions.
* Processes
    * [Process.hpp](include/nstd/Process.hpp): Abstraction layer of process launching with or without I/O redirecting.
    * [Library.hpp](include/nstd/Library.hpp): Abstraction layer of DLL / shared object loading.
* Signals/Slots
    * [Callback.hpp](include/nstd/Callback.hpp): Base classes for objects that emit or receive signals.
* Sockets (optional)
    * [Socket.hpp](include/nstd/Socket/Socket.hpp): An abstraction layer of native sockets.
    * [Server.hpp](include/nstd/Socket/Server.hpp): A TCP/IP server based on non-blocking I/O.
* Documents (optional)
    * [Json.hpp](include/nstd/Document/Json.hpp): A JSON document parser.
    * [Xml.hpp](include/nstd/Document/Xml.hpp): A basic XML document parser.
* Cryptography (optional)
    * [Sha256.hpp](include/nstd/Crypto/Sha256.hpp): A Sha256 hash tool.

Design Principles
-----------------

* Be as much optimized as currently possible.
* Do not use macros except for code that should be excluded in optimized builds.
* Use templates only when it is required to improve performance or convenience.
* Do not include system header files in public header files.
* Design class interfaces to be minimalistic and easy to use.
* Do not use exceptions.
* Avoid dynamic memory allocation when possible.
* Do not depend on c++11 (or newer) language features.
 
Supported Platforms
-------------------

* Windows x86/x86_64 (since Windows Vista / Server 2008)
* Windows x86/x86_64 Unicode (since Windows Vista / Server 2008)
* Linux i686/x86_64
* Linux armv7l

Motivation
----------

Having used and seen various libraries, I consider STL to have some serious flaws. 
* Error messages that you get from the compiler from code that tries to use STL are very difficult to read because of all the template types.
* It is nearly impossible to use STL if you want to combine components that were compiled with different compilers.
* It is unnecessarily complex to do very basic things (like case insensitive string comparisons for example) with STL.
* The stream API that has to be used for simple file IO is very cryptic because of all the involved classes (fstream, iostream, ostream, istream, ios and ios_base).
* The interface of STL containers would be a lot better if insert functions would return the inserted element or an iterator. The interface of associative containers based on std::pair is quite cumbersome. 'unordered' containers could easily remember the insertion order without significant additional costs.
* The C++11 std::thread class features thread detaching, which cannot safely be combined with dynamic library unloading.
* The C++14 std::variant class requires complex typedefs to be used and it does not help you to convert between types.
* It is not low-level enough. There are some atomic data types, but how do you use them on shared memory for instance without in-place new operations that cannot even grantee a well defined memory layout. Using STL you get little insight of how it translates to operating system API calls.
* Compilation of code that uses STL is slow.
* IDEs have a hard time understanding it. Helpers from IDEs like tool tips for method signatures are barely helpful because of the complexity from the template types.
* Debugging STL using code is complicated. The STL template source code is barely readable.
* Code written with STL does not describe itself very well (partly because of obstructing abbreviations). (cout streams to the console, fstream to a file, algorithms on iterators...)
* STL feels more like feasibility study and C++ core language feature demonstrator than an actual standard library that is supposed to help application developers.
* It is not getting significantly better with the new iterations of the C++ standard.
* It fails to provide frequently used functionality like `getopt`, process control, file system management (finally added in C++17) that is commonly required to write basic applications.
* The coding style with all lower case and underscore looks and feels outdated.
