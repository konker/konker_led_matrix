cmake_minimum_required(VERSION 2.8)
project("Konker LED Matrix Library")
set(CMAKE_BUILD_TYPE Debug)

option(KLM_DRIVER "Which LED panel specific driver to use" OFF)
option(KLM_SHARED_LIBRARY "Build a shared library" OFF)
option(KLM_NON_GPIO_MACHINE "Build target is a non-GPIO machine such a laptop" OFF)

if(NOT KLM_DRIVER)
    message(FATAL_ERROR "No KLM_DRIVER specified. Aborting.")
else()
    message("Using KLM_DRIVER: ${KLM_DRIVER}")
endif()

include_directories(include)
include_directories(../hexfont/include)
include_directories(../tinyutf8/include)

link_directories(${CMAKE_SOURCE_DIR}/../hexfont/build)
link_directories(${CMAKE_SOURCE_DIR}/../tinyutf8/build)

file(GLOB LIBSOURCES "src/*.c")
file(GLOB DRIVERSOURCES "drivers/*.c")

# Create a library for the common code
if(KLM_SHARED_LIBRARY)
    add_library(klm SHARED ${LIBSOURCES})
else()
    add_library(klm STATIC ${LIBSOURCES})
endif()

# Create a library for each driver
foreach(DRIVER ${DRIVERSOURCES})
    # Get the file name part without extension into DRIVER_ID
    get_filename_component(DRIVER_ID ${DRIVER} NAME_WE)
    message("DRIVER ID: ${DRIVER_ID}")

    if(KLM_SHARED_LIBRARY)
        add_library(${DRIVER_ID} SHARED ${DRIVER})
    else()
        add_library(${DRIVER_ID} STATIC ${DRIVER})
    endif()
endforeach()

if(KLM_NON_GPIO_MACHINE)
    add_definitions(-DKLM_NON_GPIO_MACHINE)
endif()

add_executable(example examples/example.c)
target_link_libraries(example klm ${KLM_DRIVER} hexfont tinyutf8)

add_executable(example_simple examples/example_simple.c)
target_link_libraries(example_simple klm ${KLM_DRIVER} hexfont tinyutf8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
add_executable(example_test examples/example_test.c)
target_link_libraries(example_test klm ${KLM_DRIVER} hexfont tinyutf8)

