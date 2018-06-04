cmake_minimum_required(VERSION 2.8)
project("Konker LED Matrix Library")
set(CMAKE_BUILD_TYPE Debug)

set(DEFAULT_TINYHEXFONT_DIR "${CMAKE_SOURCE_DIR}/../tinyhexfont")
set(DEFAULT_TINYUTF8_DIR "${CMAKE_SOURCE_DIR}/../tinyutf8")

option(KLM_DRIVER "Which LED panel specific driver to use" OFF)
option(KLM_WIRING_PI "Build target is a Raspberry Pi using the WiringPi library" OFF)
option(KLM_NO_DOUBLE_BUFFER "Disable double buffering" OFF)
option(KLM_NATIVE_ANIMATION "Enable native animation loop" OFF)

option(TINYHEXFONT_DIR "Location of the hexfont library" OFF)
option(TINYUTF8_DIR "Location of the tinyutf8 library" OFF)

if(NOT KLM_DRIVER)
    message(FATAL_ERROR "No KLM_DRIVER specified. Aborting.")
else()
    message("Using KLM_DRIVER: ${KLM_DRIVER}")
endif()

if(NOT TINYHEXFONT_DIR)
    message(WARNING "No TINYHEXFONT_DIR specified. Using default: ${DEFAULT_TINYHEXFONT_DIR}")
    set(TINYHEXFONT_DIR ${DEFAULT_TINYHEXFONT_DIR})
endif()

if(NOT TINYUTF8_DIR)
    message(WARNING "No TINYUTF8_DIR specified. Using default: ${DEFAULT_TINYUTF8_DIR}")
    set(TINYUTF8_DIR ${DEFAULT_TINYUTF8_DIR})
endif()

include_directories(include)

include_directories(${TINYHEXFONT_DIR}/include)
include_directories(${TINYUTF8_DIR}/include)

link_directories(${TINYHEXFONT_DIR}/build)
link_directories(${TINYUTF8_DIR}/build)

if(KLM_WIRING_PI)
    add_definitions(-DKLM_WIRING_PI)
endif()

if(KLM_NO_DOUBLE_BUFFER)
    add_definitions(-DKLM_NO_DOUBLE_BUFFER)
endif()

if(KLM_NATIVE_ANIMATION)
    add_definitions(-DKLM_NATIVE_ANIMATION)
endif()

file(GLOB LIBSOURCES "src/*.c")
file(GLOB DRIVERSOURCES "drivers/*.c")

# Create a library for the common code
add_library(klm ${LIBSOURCES})

# Create a library for each driver
foreach(DRIVER ${DRIVERSOURCES})
    # Get the file name part without extension into DRIVER_ID
    get_filename_component(DRIVER_ID ${DRIVER} NAME_WE)
    message("DRIVER ID: ${DRIVER_ID}")

    add_library(${DRIVER_ID} ${DRIVER})
endforeach()

add_executable(klm_example examples/klm_example.c)
if(KLM_WIRING_PI)
    target_link_libraries(klm_example klm ${KLM_DRIVER} hexfont tinyutf8 wiringPi)
else()
    target_link_libraries(klm_example klm ${KLM_DRIVER} hexfont tinyutf8)
endif()

add_executable(klm_example_simple examples/klm_example_simple.c)
if(KLM_WIRING_PI)
    target_link_libraries(klm_example_simple klm ${KLM_DRIVER} hexfont tinyutf8 wiringPi)
else()
    target_link_libraries(klm_example_simple klm ${KLM_DRIVER} hexfont tinyutf8)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
add_executable(klm_example_test examples/klm_example_test.c)
if(KLM_WIRING_PI)
    target_link_libraries(klm_example_test klm ${KLM_DRIVER} hexfont tinyutf8 wiringPi)
else()
    target_link_libraries(klm_example_test klm ${KLM_DRIVER} hexfont tinyutf8)
endif()

