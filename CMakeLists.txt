# Set minimum CMake version
cmake_minimum_required(VERSION 3.1)

# Project name
project(AvlComms_lib CXX)

#update git dependencies
execute_process(
    COMMAND git submodule update --recursive --remote
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add source files for the library
set (LIB_SRC src)
set (OPENAVL_SOURCES
    AvlNetClient.cpp
    ${LIB_SRC}/NetSocket.cpp
)

# Create the library
add_library(AvlComms ${OPENAVL_SOURCES})

set_property(TARGET AvlComms PROPERTY CXX_STANDARD 98)

# Include directories for the library header files
target_include_directories(AvlComms 
    PUBLIC .
    PUBLIC inc
    PUBLIC lib/CUtils
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)

add_custom_target(copy_header_files ALL
    DEPENDS AvlComms
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:AvlComms> ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_STATIC_LIBRARY_PREFIX}AvlComms${CMAKE_STATIC_LIBRARY_SUFFIX}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/AvlNetClient.hpp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    COMMENT "Copying header files to output directory"
)


# Clean target to remove extra files
add_custom_target(clean_extra_files
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.txt
    # Add any other file extensions you want to remove
    COMMENT "Cleaning extra generated files"
)