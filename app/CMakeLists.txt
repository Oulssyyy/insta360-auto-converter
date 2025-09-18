cmake_minimum_required(VERSION 3.10)
project(insta360_converter)

set(CMAKE_CXX_STANDARD 17)

# Inclure les headers du SDK
include_directories(${CMAKE_SOURCE_DIR}/../sdk/include)

# Lier les libs du SDK
link_directories(${CMAKE_SOURCE_DIR}/../sdk/lib)

add_executable(insta360_converter converter.cpp)

# Attention : adapte le nom de la lib selon le SDK (ex: MediaSDK, ins_media, etc.)
target_link_libraries(insta360_converter MediaSDK)
