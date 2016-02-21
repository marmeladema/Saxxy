cmake_minimum_required(VERSION 2.6)
project(Saxxy)

include_directories("include")
include_directories("src")
include_directories("examples")

set(LIBSAXXY_INCLUDE_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_CPP_FLAGS "-g -Wall -Wextra")
set(CMAKE_C_FLAGS "-g -Wall -Wextra")

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
