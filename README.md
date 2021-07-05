![cppserdes_icon](images/cppserdes_icon.png)

***CppSerdes*** is a serialization/deserialization library designed with embedded systems in mind

[![Build Status](https://travis-ci.com/DarrenLevine/cppserdes.svg?branch=main)](https://travis-ci.com/DarrenLevine/cppserdes) ![license](https://img.shields.io/badge/license-MIT-informational) ![version](https://img.shields.io/badge/version-1.0-blue)

## Features

* Bitpacking support - any bit alignment, any bit padding.
* Portability - uses bit shifting and masking, with no dynamic memory allocation (avoids most std containers).
* Serializes to/from 8-bit arrays, ***AND*** 16-bit, 32-bit, and 64-bit arrays. With no API changes to your code, to work with hardware drivers directly in their native bit widths.
* Common Embedded Systems types - floating point, unsigned, signed, classes, enums, arrays, custom types, etc. While avoiding pulling in atypically embedded std types such as std::string, std::vector, etc.
* Arrays - fixed sized arrays, dynamically sized arrays, and ***delimited*** arrays.
* Memory safety - Bound checking for both serial buffers and array fields.
* Choose from both high abstraction level (object-oriented & streams) and low level (memcpy-like) APIs.
* Custom formatter types, such as optional validation checks, attached right to a field.
* Virtual fields, and pure virtual fields, allowing you to easily change formats at runtime.
* Header only, and works with C++11 and greater.
* "constexpr" support for C++14 or greater using bitcpy function.
* Compiles with high warning levels (pedantic, Wall, etc.).

## Documentation

[darrenlevine.github.io/cppserdes](https://darrenlevine.github.io/cppserdes/)

## Quick Start

1. Check out the examples:
    * Compile and run "examples/01_simple_example.cpp".
    * Read through the examples/ folder for various usage cases.
2. Use the library in your project:
    * *Manual inclusion:* Make sure "cppserdes/include/" is added to your project's path and use as desired.
    * *With CMake:* Add the following lines to your CmakeLists.txt project file:

```cmake
add_subdirectory(cppserdes/)
target_link_libraries(your_project_name cppserdes)
```

## Object Oriented Example (using serdes::packet_base)

```cpp
#include "serdes.h"

struct my_packet : serdes::packet_base {
    int8_t x = 6, y = 7, z = 8;

    // define a format (used for BOTH serialization and deserialization)
    void format(serdes::packet &serdes_process) {
        serdes_process + x + y + serdes::pad(1) + serdes::bitpack(z, 7);
    }
};

int main() {
    uint16_t serial_data[] = {0x0102, 0x7B00};

    my_packet obj1;
    obj1.load(serial_data); // loads serial data into obj1 x == 1, y == 2, z == -5

    my_packet obj2;
    obj2.store(serial_data); // stores {0x0607, 0x0800} into serial_data from obj2
}
```

## Functional Example (using serdes::bitcpy)

```cpp
#include "bitcpy.h"

int main() {
    uint16_t serial_data[] = {0x0102, 0xFB00};
    int8_t x, y, z;

    // deserialization
    serdes::bitcpy(x, serial_data, 0, 8);
    serdes::bitcpy(y, serial_data, 8, 8);
    serdes::bitcpy(z, serial_data, 16, 8);

    // serialization
    serdes::bitcpy(serial_data, x, 0, 8);
    serdes::bitcpy(serial_data, y, 8, 8);
    serdes::bitcpy(serial_data, z, 16, 8);
}
```

## Streams Example (using serdes::packet)

```cpp
#include "serdes.h"

int main() {
    uint16_t serial_data[] = {0x0102, 0x0304, 0x0506};
    int8_t x, y, z;

    // take some serial data and place it into variables
    serdes::packet(serial_data) >> x >> y >> z;

    // take same variables and place it into serial data
    serdes::packet(serial_data) << x << y << z;
}
```

## Known Limitations

* Little endian "serialization" is not yet supported (Note: little and big endian "platforms" ARE both supported)
* Cannot bitcpy more than 0.25GB of data in a single function call (due to a memory-usage/speed design decision)
* The examples are written using C++17 syntax for succinctness and simplicity, so if you're using an older compiler, templated fields might need to have their templates explicitly specified, for example:

```cpp
serdes::bitpack(123, serdes::bit_length(2)) // works in >= C++17
serdes::bitpack<int>(123, serdes::bit_length(2)) // needed in < C++17
```

## Planned Features

* Little endian serialization support (compile time only is planned)
* The ability to avoid serialization/deserialization by setting pointers to point to the appropriate spot within the serial buffer, instead of making a copy into a new buffer, thus saving time and memory if the serial buffer's lifetime persists longer than the reference, and the memory is aligned appropriately to allow this optimization. Similar to how capnproto and flatbuffers work.

## Requirements

* C++11 or greater
