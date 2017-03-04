libnstd
=======

libnstd is a cross platform non-standard standard library for C++. In contrast to the Standard Template
Library (STL) its aim is to provide frequently used features with minimalistic classes and as little usage of templates
as possible. Right now it consists of following headers:

* [Base.h](include/nstd/Base.h): Basic data types and allocation operators.
* [Debug.h](include/nstd/Debug.h): Low level debugging tools.
* [Memory.h](include/nstd/Memory.h): Memory allocation and other memory tools.
* [Error.h](include/nstd/Error.h): Abstraction layer of operating system error reporting.
* [Math.h](include/nstd/Math.h): Basic mathematics functions.
* Data types
    * [String.h](include/nstd/String.h): A lazy copying string class.
    * [Variant.h](include/nstd/Variant.h): A lazy copying variable data type for strings, lists, maps, integers and floats.
    * [Buffer.h](include/nstd/Buffer.h): A dynamic byte buffer.
    * [Time.h](include/nstd/Time.h): Date and time functions.
* Containers
    * [List.h](include/nstd/List.h): A double linked list.
    * [HashMap.h](include/nstd/HashMap.h): An associative container based on a hash map with fixed table size.
    * [HashSet.h](include/nstd/HashSet.h):  A container for unique elements based on a hash set with fixed table size.
    * [Map.h](include/nstd/Map.h), MultiMap.h: An associative container based on a balanced tree.
    * [Array.h](include/nstd/Array.h): A dynamically growing container based on an array.
    * [PoolList.h](include/nstd/PoolList.h): An iterable object pool for non-copyable objects.
    * [PoolMap.h](include/nstd/PoolMap.h): An associative object pool for non-copyable objects.
* Concurrency
    * [Thread.h](include/nstd/Thread.h): Abstraction layer of native multi-threading features.
    * [Atomic.h](include/nstd/Atomic.h): Abstraction layer of atomic functions.
    * [Mutex.h](include/nstd/Mutex.h): Abstraction layer of a native mutual exclusion feature.
    * [Semaphore.h](include/nstd/Semaphore.h): Abstraction layer of a native semaphore construct.
    * [Signal.h](include/nstd/Signal.h): Abstraction layer of a native signal construct.
    * [Monitor.h](include/nstd/Monitor.h): Abstraction layer of a native monitor construct.
* Input/Output
    * [File.h](include/nstd/File.h): Abstraction layer of file input/output.
    * [Directory.h](include/nstd/Directory.h): Abstraction layer to access directories.
    * [Console.h](include/nstd/Console.h): Helper function for Console I/O and command prompting.
    * [Log.h](include/nstd/Log.h): Console logging helper functions.
* Processes
    * [Process.h](include/nstd/Process.h): Abstraction layer of process launching with or without I/O redirecting.
    * [Library.h](include/nstd/Library.h): Abstraction layer of dll / shared object loading.
* Signals/Slots
    * [Event.h](include/nstd/Event.h): Base classes for objects that emit or receive signals.
* Sockets (optional)
    * [Socket.h](include/nstd/Socket/Socket.h): An abstraction layer of native sockets.
    * [Server.h](include/nstd/Socket/Server.h): An asynchronous TCP/IP server multiplexer.
* Documents (optional)
    * [JSON.h](include/nstd/Document/JSON.h): A JSON document parser.
    * [XML.h](include/nstd/Document/XML.h): A basic XML document parser.

Design Principles
-----------------

 * Be as much optimized as currently possible. (Unfortunately, I do not have the time to optimize the shit out of each function, but some classes/functions are already pretty optimized and faster than their STL counterpart.)
 * Do not use macros except for code that should be excluded in optimized builds.
 * Use templates only when it is required to improve performance.
 * Do not include system header files in libnstd header files.
 * Design class interfaces to be minimalistic and easy to use.
 * Do not use exceptions.
 * Avoid dynamic memory allocation when possible.
 
Supported Platforms
-------------------

 * Windows x86/x86_64 (since Windows Vista / Server 2008)
 * Windows x86/x86_64 Unicode (since Windows Vista / Server 2008)
 * Cygwin i686 (since Windows Vista / Server 2008)
 * Linux i686/x86_64
 * Linux armv61
