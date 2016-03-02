cmake_minimum_required(VERSION 2.8)
project("Konker LED Matrix Library")
set(CMAKE_BUILD_TYPE Debug)

option(KLM_DRIVER "Which LED panel specific driver to use" OFF)
option(KLM_SHARED_LIBRARY "Build a shared library" OFF)
option(KLM_NON_GPIO_MACHINE "Build target is a non-GPIO machine such a laptop" OFF)

if(NOT KLM_DRIVER)
    message(FATAL_ERROR "No KLM_DRIVER specified. Aborting.")
endif()

include_directories(include)
include_directories(../hexfont/include)
include_directories(../tinyutf8/include)

link_directories(${CMAKE_SOURCE_DIR}/../hexfont/build)
link_directories(${CMAKE_SOURCE_DIR}/../tinyutf8/build)

file(GLOB LIBSOURCES "src/*.c")
set(DRIVERSOURCE "${CMAKE_CURRENT_SOURCE_DIR}/drivers/klm_driver_${KLM_DRIVER}.c")

if(KLM_SHARED_LIBRARY)
    add_library(klm SHARED ${LIBSOURCES} ${DRIVERSOURCE})
else()
    add_library(klm STATIC ${LIBSOURCES} ${DRIVERSOURCE})
endif()

if(KLM_NON_GPIO_MACHINE)
    add_definitions(-DKLM_NON_GPIO_MACHINE)
endif()

add_executable(example examples/example.c)
target_link_libraries(example klm hexfont tinyutf8)
