# Oakum
*Oakum* is a lightweight cross platform memory leak detection library. It achieves this by overriding `new` and `delete` operators in their different flavors to track memory allocations and deallocations. Oakum provides a binary *yes/no* answer to the question "Do I have any leaks?" as well as detailed information including allocation size, parameters and precise stack trace in which the leaked allocation happened. *Oakum* is compiled as a static library, which applications should link to.

# Building
*Oakum* requires CMake 3.10.0 or newer and a C++ compiler supporting C++17 standard.

Run CMake to generate build scripts:
```
git clone https://github.com/DziubanMaciej/Oakum
cd Oakum
mkdir build
cd build
cmake ..
```
Additional optional arguments can be passed to the `cmake` command:
  - `-D OAKUM_BUILD_EXAMPLES=1` - builds example applications, which use the *Oakum* library and ilustrate its capabilities.
  - `-D OAKUM_BUILD_TESTS=1` - builds tests for the *Oakum* library.
  - `-D OAKUM_MAX_STACK_FRAMES_COUNT=<value>` - overrides maximum number stack frames captured in stack traces. Default is 10.
  - `-D OAKUM_GENERATE_DOCS=1` - generate HTML documentation from [oakum_api.h](source/include/oakum/oakum_api.h) file using Doxygen.
  - `-D OAKUM_DOXYGEN_COMMAND=/path/to/doxygen` - overrides command used to run Doxygen. By default the docs build scripts rely on PATH variable.

Compile the project
```
cmake --build .
```

# Using the library

### Configuring project
Applications should statically link to the *Oakum* library (`Oakum.lib` on Windows, `libOakum.a` on Linux) and add [source/include](source/include) to their include paths. Both of these can be handled automatically in CMake projects, which add *Oakum* library to their source tree:
```
add_subdirectory(Oakum)

...

add_executable(MyApplication ${SOURCES})
target_link_libraries(MyApplication PRIVATE Oakum)
```

### Library API
Although *Oakum* library is aimed at C++ project, its API is a set of C-style functions to provide better compatibility. The whole API is documented Doxygen-style in [oakum_api.h](source/include/oakum/oakum_api.h) file.

### Examples

The simplest use-case of the *Oakum* library is simply a binary information, whether leaks were detected or not. This can be achieved with just 3 API calls. Refer to [example/example_simple.cpp](example/example_simple.cpp) for a simple example performing this.

The user can also gather more detailed information such as stack traces of all leaked allocations. Refer to [example/example_stack_traces.cpp](example/example_stack_traces.cpp) for a more complex example, which queries all of that data.

It may be useful to use *Oakum* in your unit tests. Example application [example/example_gtest.cpp](example/example_gtest.cpp) registers an event listener in Googletest testing framework. The listener uses *Oakum* to check for memory leaks after each individual executed unit test.
