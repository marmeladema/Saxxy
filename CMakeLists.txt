cmake_minimum_required(VERSION 2.6)
project(Saxxy)

include_directories("include")
include_directories("src")

add_library(saxxy SHARED
include/saxxy.h
src/saxxy.c)

add_library(saxxy_static STATIC
include/saxxy.h
src/saxxy.c)
