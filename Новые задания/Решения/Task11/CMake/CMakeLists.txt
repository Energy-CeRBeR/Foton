cmake_minimum_required(VERSION 3.5)

project(Task11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") 
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
  add_compile_options(
    -Werror=return-type
    -Wfatal-errors
    -Wall
    -Wextra
    -Wwrite-strings
    -Winit-self
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") 
  add_compile_options("/W4")
endif()

add_executable(${PROJECT_NAME}
  main.cpp
)
