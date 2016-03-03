cmake_minimum_required(VERSION 2.8)
project("Konker LED Matrix Library")
set(CMAKE_BUILD_TYPE Debug)

set(DEFAULT_HEXFONT_DIR "${CMAKE_SOURCE_DIR}/../hexfont")
set(DEFAULT_TINYUTF8_DIR "${CMAKE_SOURCE_DIR}/../tinyutf8")

option(KLM_DRIVER "Which LED panel specific driver to use" OFF)
option(KLM_SHARED_LIBRARY "Build a shared library" OFF)
option(KLM_NON_GPIO_MACHINE "Build target is a non-GPIO machine such a laptop" OFF)
option(KLM_NO_DOUBLE_BUFFER "Disable double buffering" OFF)

option(HEXFONT_DIR "Location of the hexfont library" OFF)
option(TINYUTF8_DIR "Location of the tinyutf8 library" OFF)

if(NOT KLM_DRIVER)
    message(FATAL_ERROR "No KLM_DRIVER specified. Aborting.")
else()
    message("Using KLM_DRIVER: ${KLM_DRIVER}")
endif()

if(NOT HEXFONT_DIR)
    message(WARNING "No HEXFONT_DIR specified. Using default: ${DEFAULT_HEXFONT_DIR}")
    set(HEXFONT_DIR ${DEFAULT_HEXFONT_DIR})
endif()

if(NOT TINYUTF8_DIR)
    message(WARNING "No TINYUTF8_DIR specified. Using default: ${DEFAULT_TINYUTF8_DIR}")
    set(TINYUTF8_DIR ${DEFAULT_TINYUTF8_DIR})
endif()

include_directories(include)

include_directories(${HEXFONT_DIR}/include)
include_directories(${TINYUTF8_DIR}/include)

link_directories(${HEXFONT_DIR}/build)
link_directories(${TINYUTF8_DIR}/build)

if(KLM_NON_GPIO_MACHINE)
    add_definitions(-DKLM_NON_GPIO_MACHINE)
endif()

if(KLM_NO_DOUBLE_BUFFER)
    add_definitions(-DKLM_NO_DOUBLE_BUFFER)
endif()

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

add_executable(klm_example examples/klm_example.c)
target_link_libraries(klm_example klm ${KLM_DRIVER} hexfont tinyutf8)

add_executable(klm_example_simple examples/klm_example_simple.c)
target_link_libraries(klm_example_simple klm ${KLM_DRIVER} hexfont tinyutf8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
add_executable(klm_example_test examples/klm_example_test.c)
target_link_libraries(klm_example_test klm ${KLM_DRIVER} hexfont tinyutf8)

