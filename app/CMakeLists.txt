cmake_minimum_required(VERSION 3.10)
project(insta360_converter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SDK paths
set(SDK_ROOT_DIR ${CMAKE_SOURCE_DIR}/../sdk)
set(SDK_INCLUDE_DIR ${SDK_ROOT_DIR}/include)
set(SDK_LIB_DIR ${SDK_ROOT_DIR}/lib)

# Check if SDK directories exist
if(NOT EXISTS ${SDK_INCLUDE_DIR})
    message(FATAL_ERROR "SDK include directory not found: ${SDK_INCLUDE_DIR}")
endif()

if(NOT EXISTS ${SDK_LIB_DIR})
    message(FATAL_ERROR "SDK lib directory not found: ${SDK_LIB_DIR}")
endif()

# Include SDK headers
include_directories(${SDK_INCLUDE_DIR})

# Link SDK libraries
link_directories(${SDK_LIB_DIR})

add_executable(insta360_converter main.cpp)

# Find required system libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(PNG REQUIRED libpng16)

# Link the MediaSDK library and its dependencies
target_link_libraries(insta360_converter 
    MediaSDK
    ${PNG_LIBRARIES}
    pthread
    dl
)

# Add filesystem library for C++17
target_link_libraries(insta360_converter stdc++fs)

# Add library search paths
target_link_directories(insta360_converter PRIVATE ${PNG_LIBRARY_DIRS})

# Add include directories
target_include_directories(insta360_converter PRIVATE ${PNG_INCLUDE_DIRS})

# Add compile flags
target_compile_options(insta360_converter PRIVATE ${PNG_CFLAGS_OTHER})
