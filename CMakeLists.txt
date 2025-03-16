cmake_minimum_required(VERSION 3.25)

project(Clox VERSION 1.0 LANGUAGES "C")

set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tutorial_compiler_flags INTERFACE)

# target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)

# add compiler warning flags just when building this project via
# the BUILD_INTERFACE genex
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)


aux_source_directory(./src SOURCES) 

# add the executable
add_executable(Clox ${SOURCES})
set_target_properties(Clox PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# link libs
target_link_libraries(Clox PUBLIC tutorial_compiler_flags)

# add the binary tree to the search path for include files
# so that we will find CloxConfig.h
target_include_directories(Clox PUBLIC "${PROJECT_BINARY_DIR}")
