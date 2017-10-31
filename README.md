libnstd
=======

libnstd is a cross platform non-standard standard library replacement for C++. In contrast to the Standard Template
Library (STL) its aim is to provide frequently used features with minimalistic classes and as little usage of templates
as possible. Code written with libnstd should be easily understandable for everyone with basic knowledge of imperative 
programming and common design patterns. Right now it consists of following headers:

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
    * [Map.h](include/nstd/Map.h), [MultiMap.h](include/nstd/MultiMap.h): An associative container based on a balanced tree.
    * [Array.h](include/nstd/Array.h): A dynamically growing container based on an array.
    * [PoolList.h](include/nstd/PoolList.h): An iterable object pool for non-copyable objects.
    * [PoolMap.h](include/nstd/PoolMap.h): An associative object pool for non-copyable objects.
    * [RefCount.h](include/nstd/RefCount.h): Tools for object reference counting.
* Concurrency
    * [Thread.h](include/nstd/Thread.h): Abstraction layer of native multi-threading features.
    * [Atomic.h](include/nstd/Atomic.h): Abstraction layer of atomic functions.
    * [Mutex.h](include/nstd/Mutex.h): Abstraction layer of a native mutual exclusion feature.
    * [Semaphore.h](include/nstd/Semaphore.h): Abstraction layer of a native semaphore construct.
    * [Signal.h](include/nstd/Signal.h): Abstraction layer of a native signal construct.
    * [Monitor.h](include/nstd/Monitor.h): Abstraction layer of a native monitor construct.
    * [Future.h](include/nstd/Future.h): Asynchronous function calls with global thread pool.
* Input/Output
    * [File.h](include/nstd/File.h): Abstraction layer of file input/output.
    * [Directory.h](include/nstd/Directory.h): Abstraction layer to access directories.
    * [Console.h](include/nstd/Console.h): Helper function for Console I/O and command prompting.
    * [Log.h](include/nstd/Log.h): Console logging helper functions.
* Processes
    * [Process.h](include/nstd/Process.h): Abstraction layer of process launching with or without I/O redirecting.
    * [Library.h](include/nstd/Library.h): Abstraction layer of DLL / shared object loading.
* Signals/Slots
    * [Callback.h](include/nstd/Callback.h): Base classes for objects that emit or receive signals.
* Sockets (optional)
    * [Socket.h](include/nstd/Socket/Socket.h): An abstraction layer of native sockets.
    * [Server.h](include/nstd/Socket/Server.h): An asynchronous TCP/IP server multiplexer.
* Documents (optional)
    * [JSON.h](include/nstd/Document/JSON.h): A JSON document parser.
    * [XML.h](include/nstd/Document/XML.h): A basic XML document parser.
* Cryptography (optional)
    * [Sha256.h](include/nstd/Crypto/Sha256.h): A Sha256 hash tool.

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
* Cygwin i686 (since Windows Vista / Server 2008) (might not work anymore)
* Linux i686/x86_64
* Linux armv61

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
* It is not low-level enough. There are some atomic data types, but how do you use them on shared memory for instance without in-place new operations that cannot even grantee a simple a well defined memory layout. Using STL you get little sense for how it translate to operating system API calls.
* Compilation of code that uses STL is slow.
* IDEs have a hard time understanding it. Helpers from IDEs like tool tips for method signatures are barely helpful because of the complexity from the template types.
* Debugging STL using code is complicated. The STL template source code is barely readable.
* Code written with STL does not describe itself very well (partly because of obstructing abbreviations). (cout streams to the console, fstream to a file, algorithms on iterators...)
* STL feels more like feasibility study and C++ core language feature demonstrator than an actual standard library that is supposed to help application developers.
* It is not getting significantly better with the new iterations of the C++ standard.
* It fails to provide frequently used functionality like `getopt`, process control, file system management (fixed in C++17?) that is commonly required to write basic applications.
* The coding style with all lower case and underscore looks an feels outdated.

