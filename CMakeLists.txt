cmake_minimum_required(VERSION 2.6)
project(Saxxy)

include_directories("include")
include_directories("src")
include_directories("examples")

add_library(saxxy SHARED
include/saxxy.h
src/saxxy.c)

add_library(saxxy_static STATIC
include/saxxy.h
src/saxxy.c)

add_executable(printer
examples/printer.c)
target_link_libraries(printer saxxy)

add_executable(javascript
examples/javascript.c)
target_link_libraries(javascript saxxy)
