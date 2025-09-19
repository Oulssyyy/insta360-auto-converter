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

# Find required system libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(PNG REQUIRED libpng16)
find_package(jsoncpp REQUIRED)

# Common libraries and settings
set(COMMON_LIBRARIES
    MediaSDK
    ${PNG_LIBRARIES}
    pthread
    dl
    stdc++fs
)

set(COMMON_COMPILE_OPTIONS ${PNG_CFLAGS_OTHER})
set(COMMON_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
set(COMMON_LIBRARY_DIRS ${PNG_LIBRARY_DIRS})

# Single file converter (original functionality)
add_executable(insta360_converter main.cpp)
target_link_libraries(insta360_converter ${COMMON_LIBRARIES})
target_include_directories(insta360_converter PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_directories(insta360_converter PRIVATE ${COMMON_LIBRARY_DIRS})
target_compile_options(insta360_converter PRIVATE ${COMMON_COMPILE_OPTIONS})

# Batch processor for Synology NAS
add_executable(insta360_batch_processor batch_processor.cpp)
target_link_libraries(insta360_batch_processor 
    ${COMMON_LIBRARIES}
    jsoncpp_lib
)
target_include_directories(insta360_batch_processor PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_directories(insta360_batch_processor PRIVATE ${COMMON_LIBRARY_DIRS})
target_compile_options(insta360_batch_processor PRIVATE ${COMMON_COMPILE_OPTIONS})

# Install both executables
install(TARGETS insta360_converter insta360_batch_processor
    RUNTIME DESTINATION bin
)
