cmake_minimum_required(VERSION 3.16)

project(dns_cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third-party)

include_directories(src)

file(GLOB
  SRC_FILES
  ${PROJECT_SOURCE_DIR}/src/*.cpp 
)

add_executable(dns_cache ${SRC_FILES} main.cpp)
target_link_libraries(dns_cache absl::flat_hash_map)
