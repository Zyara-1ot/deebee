cmake_minimum_required(VERSION 3.16)

project(deebee
    VERSION 0.1
    LANGUAGES CXX
)

# -----------------------
# C++ standard
# -----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------
# Sources
# -----------------------
file(GLOB_RECURSE CORE_SOURCES
    core/*.cpp
)

file(GLOB_RECURSE PARSER_SOURCES
    parser/*.cpp
)

file(GLOB_RECURSE MAIN_SOURCES
    cli_client/*.cpp
)
set(SOURCES
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${PARSER_SOURCES}
)

# -----------------------
# Executable
# -----------------------
add_executable(deebee ${SOURCES})

# -----------------------
# Include directories
# -----------------------
target_include_directories(deebee PRIVATE
    core
    parser
    libduckdb-linux-amd64
)

# -----------------------
# DuckDB library
# -----------------------
target_link_directories(deebee PRIVATE
    ${CMAKE_SOURCE_DIR}/libduckdb-linux-amd64
)

target_link_libraries(deebee PRIVATE
    duckdb
)

# -----------------------
# RPATH (so .so is found at runtime)
# -----------------------
set_target_properties(deebee PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/libduckdb-linux-amd64"
)