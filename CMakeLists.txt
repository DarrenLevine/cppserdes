cmake_minimum_required(VERSION 3.14)

project(
    cppserdes
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "C++ serialization/deserialization library for embedded systems"
)

add_library(cppserdes INTERFACE)

target_include_directories(cppserdes INTERFACE include/
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# To include cppserdes in your project, just add the following two lines to your CMakeLists.txt file
#   add_subdirectory(cppserdes/)
#   target_link_libraries(your_project_name cppserdes)
