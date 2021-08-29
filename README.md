# Oakum
*Oakum* is a lightweight cross platform memory leak detection library. It achieves this by overriding `new` and `delete` operators in their different flavors to track memory allocations and deallocations. Oakum provides a binary *yes/no* answer to the question "Do I have any leaks?" as well as detailed information including allocation size, parameters and precise stack trace in which the leaked allocation happened. *Oakum* is compiled as a static library, which applications should link to.

# Building
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
  - `-D OAKUM_BUILD_UNIT_TESTS=1` - builds unit tests for the *Oakum* library*.
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

### The API
The simplest use-case of the *Oakum* library is simply a binary information, whether leaks are detected or not. This can be achieved with just 3 API calls. Refer to [example/example_simple.cpp](example/example_simple.cpp] for a simple example performing this.

The user can also gather more detailed such as stack traces of all leaked allocations. Refer to [example/example_stack_traces.cpp](example/example_stack_traces.cpp) for a more complex example, which queries all of that data.

The whole API is documented Doxygen-style in [oakum_api.h](source/include/oakum/oakum_api.h) file.
