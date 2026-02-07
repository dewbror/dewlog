DEWLOG 0.2
===========

C/C++ header-only logging library.

TODO
----
- Write "Documentation" section in header
- Add thread safety (pthreads, c11-threads, SDL threads?)

Dependencies
------------

  - [cmocka](https://github.com/clibs/cmocka) 1.1.5 (for tests only)

How to Use
----------
In exactly ONE C/C++ file, do this:
```
#define DEWLOG_IMPLEMENTATION
#include <dewlog.h>
```
This will include all the function prototypes and definitions into that C/C++ file.

How to Build and Run Tests (with cmake)
---------------------------------------

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
./Debug/dewlog_tests
```

License
-------

The source code in this repository is licensed under the MIT License, see [LICENSE.txt](https://github.com/dewbror/dewlog/blob/master/LICENSE.txt). This license applies only to dewlog.h.

--- end of README ---
