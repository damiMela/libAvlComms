# CMakeLists.txt for the main project directory

# Set minimum CMake version
cmake_minimum_required(VERSION 3.1)

# Project name
project(OpenAvl_lib CXX)

# Add source files for the library
set (LIB_SRC src)
set (OPENAVL_SOURCES
    ${LIB_SRC}/NetClient.cpp
    ${LIB_SRC}/NetSocket.cpp
)

# Create the library
add_library(AvlComms ${OPENAVL_SOURCES})

set_property(TARGET AvlComms PROPERTY CXX_STANDARD 98)

# Include directories for the library header files
target_include_directories(AvlComms 
    PUBLIC inc
    PUBLIC lib/CUtils
)


# Add the executable
add_executable(example examples/Main.cpp)

# Link the executable with the LcdMenuBuilder library
target_link_libraries(example AvlComms)

# Clean target to remove extra files
add_custom_target(clean_extra_files
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.txt
    # Add any other file extensions you want to remove
    COMMENT "Cleaning extra generated files"
)
