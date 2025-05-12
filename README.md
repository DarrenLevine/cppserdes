![cppserdes_icon](images/cppserdes_icon.png)

***CppSerdes*** is a serialization/deserialization library designed with embedded systems in mind - where bit-level manipulation and low-level protocol design tooling are first class citizens

![license](https://img.shields.io/badge/license-MIT-informational) ![version](https://img.shields.io/badge/version-1.1-blue)

## Features

* Bitpacking support - any bit alignment, any bit padding. Even support for variable bit lengths.
* Portability - will produce identical serialized data structures on any platform.
* Doesn't just convert to/from bytes like other serializers. ***Also supports*** converting to/from 16-bit, 32-bit, and 64-bit serial data. With no changes to your format description code, to work with hardware drivers directly in their native bit widths.
* Common Embedded Systems types - floating point, unsigned, signed, chars, classes, enums, arrays, delimited arrays, atomics, bit-packed anything, custom types, etc. While not forcing you to use atypically embedded dynamically-allocated std types such as std::string, std::vector, etc.
* Arrays - fixed sized arrays, dynamically sized arrays, and ***delimited*** arrays (by any delimiter - even programmatic ones).
* Memory safety - Bounds checking for everything! Serial buffers, array fields, etc.
* Choose from both high abstraction level (object-oriented & streams) and low level (memcpy-like) APIs.
* Custom formatter types, such as optional validation checks, attached right to a field.
* Virtual fields, and pure virtual fields, allowing you to easily change formats at runtime.
* Header only, and works with C++11 and greater.
* "constexpr" support for C++14 or greater using bitcpy function (depends on compiler support).
* Compiles with high warning levels (pedantic, Wall, etc.).
* Support for custom types (see [examples/14_custom_types.cpp](examples/14_custom_types.cpp)).
* Uses C++11 reflection to automatically support any std-lib and/or custom types if they have common methods such as `T load() const` & `void store(T)`, or `T* data()` & `size_t size()`.

## Documentation

[darrenlevine.github.io/cppserdes](https://darrenlevine.github.io/cppserdes/)

## Quick Start

1. Check out the examples:
    * Compile and run any of the examples - no installation needed. Here's how:

        ```sh
        cd cppserdes/examples/
        g++ 01_simple_example.cpp && ./a.out
        ```

    * Read through the examples/ folder for various usage cases, such as [examples/07_delimited_arrays.cpp](examples/07_delimited_arrays.cpp).
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
    void format(serdes::packet &serial_data) {
        serial_data + x + y + serdes::pad(1) + serdes::bitpack(z, 7);
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

## Functional Example (using serdes::bitcpy - **It's just memcpy but with bits**!)

```cpp
#include "bitcpy.h"

int main() {
    uint16_t serial_data[] = {0x0102, 0xFB00};
    int8_t x, y, z;

    // deserialization
    serdes::bitcpy(x, serial_data, 0, 8); // at MSbit 0, copy 8 bits into x
    serdes::bitcpy(y, serial_data, 8, 8); // at MSbit 8, copy 8 bits into y
    serdes::bitcpy(z, serial_data, 16, 5); // at MSbit 16, copy 5 bits into z

    // serialization
    serdes::bitcpy(serial_data, x, 0, 8); // at MSbit 0, copy 8 bits into serial_data
    serdes::bitcpy(serial_data, y, 8, 8); // at MSbit 8, copy 8 bits into serial_data
    serdes::bitcpy(serial_data, z, 16, 5); // at MSbit 16, copy 5 bits into serial_data
}
```

## Streams Example (using serdes::packet)

```cpp
#include "serdes.h"

int main() {
    uint16_t serial_data[] = {0x0102, 0x0304, 0x0506};
    int8_t x, y, z;

    // take some serial data and place it into variables (deserializing) left to right
    serdes::packet(serial_data) >> x >> y >> serdes::pad(2) >> serdes::bitpack(z, 6);

    // take same variables and place it into serial data (serializing) left to right
    serdes::packet(serial_data) << x << y << serdes::pad(2) << serdes::bitpack(z, 6);
}
```

## What makes CppSerdes unique

You might have tried using other serializers such as protobuf or boost-serial, only to learn that they don't offer enough control over how serial data is described for your needs. If you're trying to design your own transfer protocol, or implement an existing one from a hardware device spec - CppSerdes will offer you the safety and control you've been looking for!

Here's an example meant to show off the sort of things you can do with CppSerdes:

```cpp
#include "serdes.h"
#include <stdio.h>

// inheriting from packet_base will add helpful methods with no memory overhead (optional)
struct coordinate_list : serdes::packet_base {

    struct coordinates_type {
        uint8_t bits_per_coordinate{};
        int64_t x{}, y{}, z{};

        // how to format the class into/out-of serial data
        void format(serdes::packet &serial_data) {
            // describes an 8 bit field "bits_per_coordinate" conveying how many
            // bits are in each of the next three bit-packed "x, y, z" fields
            serial_data +
                bits_per_coordinate +
                serdes::bitpack(x, bits_per_coordinate) +
                serdes::bitpack(y, bits_per_coordinate) +
                serdes::bitpack(z, bits_per_coordinate);
        }
    };

    uint16_t num_coordinates{};
    coordinates_type coordinates[100]{};
    uint16_t crc16{};

    // how to format the class into/out-of serial data
    void format(serdes::packet &serial_data) final {

        // describes a 16 bit "num_coordinates" length, followed by a
        // variable-sized array of coordinate objects of that length
        serial_data +
            num_coordinates +
            serdes::array(coordinates, num_coordinates);

        // pick a CRC algorithm and calculate a checksum across all the bytes up to this point,
        // and if in serialization/storing mode also save the value into the &crc16 field
        auto calculated_crc = serial_data.calculate_crc<CRC16::CCITT_FALSE>(&crc16);

        // then while serializing/deserializing the crc16 field here, also validate it
        serial_data + serdes::validate(crc16, [&] { return crc16 == calculated_crc; });

        // prints out the values if the crc was invalid
        if (serial_data.status == serdes::status_e::INVALID_FIELD)
            printf("Got an invalid crc! Got 0x%04X, calculated 0x%04X\n", crc16, calculated_crc);
    }
};
```

## Known Limitations

* Little endian "serialization" is not yet supported (Note: little and big endian "platforms" ARE both supported)
* Constexpr bitcpy of enums and floating point types is not supported since the constexpr interpretation of their memory is undefined behavior (i.e. non-constexpr memcpy must be used behind the scenes)
* Cannot bitcpy more than 0.25GB of data in a single function call (due to a memory-usage/speed design decision)
* The examples are written using C++17 syntax for succinctness and simplicity, so if you're using an older compiler, templated fields might need to have their templates explicitly specified, for example:

```cpp
serdes::bitpack(123, serdes::bit_length(2)) // works in >= C++17
serdes::bitpack<int, int>(123, serdes::bit_length(2)) // needed in < C++17
```

## Future Features

* Little endian "serialization" support (compile time only). Note that little endian "platforms" are already supported. Inclusion of this feature is complete but pending licensing review.

## Requirements

* C++11 or greater
