cmake_minimum_required(VERSION 3.16)
project(nanomarket LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
  # MSVC equivalent warning and optimization flags
  add_compile_options(/O2 /W4 /permissive-)
else()
  add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)


# Build a library from src/ (exclude main.cpp) so tests can link to implementation
file(GLOB_RECURSE ALL_SRC_FILES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)
set(MAIN_CPP ${CMAKE_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM ALL_SRC_FILES ${MAIN_CPP})

add_library(nanomarket_core STATIC ${ALL_SRC_FILES})
target_include_directories(nanomarket_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(nanomarket_core PRIVATE NANOMARKET_BUILD=1)

add_executable(nanomarket ${MAIN_CPP})
target_link_libraries(nanomarket PRIVATE nanomarket_core)
target_include_directories(nanomarket PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Replay executable
add_executable(main_replay src/main_replay.cpp)
target_link_libraries(main_replay PRIVATE nanomarket_core)
target_include_directories(main_replay PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()

# Tests
file(GLOB_RECURSE TEST_SRC_FILES
  ${CMAKE_SOURCE_DIR}/tests/*.cpp
)

foreach(test_src ${TEST_SRC_FILES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(${test_name} PRIVATE nanomarket_core)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
