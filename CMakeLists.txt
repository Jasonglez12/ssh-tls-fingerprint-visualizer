cmake_minimum_required(VERSION 3.15)
project(ssh_tls_fingerprint_visualizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find packages
find_package(OpenSSL REQUIRED)

# Source files for common library
set(COMMON_SOURCES
    src/utils.cpp
    src/fingerprint_common.cpp
)

set(COMMON_HEADERS
    include/utils.h
    include/fingerprint_common.h
)

# Create common library
add_library(fingerprint_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
target_link_libraries(fingerprint_common 
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# TLS fingerprint collector
add_executable(fingerprint_tls src/fingerprint_tls.cpp)
target_link_libraries(fingerprint_tls fingerprint_common)

# SSH fingerprint collector
add_executable(fingerprint_ssh src/fingerprint_ssh.cpp)
target_link_libraries(fingerprint_ssh fingerprint_common)

# Baseline diff tool
add_executable(baseline_diff src/baseline_diff.cpp)
target_link_libraries(baseline_diff fingerprint_common)

# Visualization tool
add_executable(visualize src/visualize.cpp)
target_link_libraries(visualize fingerprint_common)

# Evaluation dataset generator
add_executable(generate_eval_set src/generate_eval_set.cpp)
target_link_libraries(generate_eval_set fingerprint_common)

# Installation
install(TARGETS fingerprint_tls fingerprint_ssh baseline_diff visualize generate_eval_set
    RUNTIME DESTINATION bin
)

# Create data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

