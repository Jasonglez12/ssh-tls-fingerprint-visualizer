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

# Optional: libpcap for PCAP parsing
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBPCAP QUIET libpcap)
endif()

# Optional: libcurl for webhook alerts
find_package(CURL QUIET)

# Define compile options
if(LIBPCAP_FOUND)
    add_definitions(-DHAVE_PCAP)
    message(STATUS "Found libpcap: ${LIBPCAP_LIBRARIES}")
else()
    message(WARNING "libpcap not found. PCAP parsing will be disabled.")
endif()

if(CURL_FOUND)
    add_definitions(-DHAVE_CURL)
    message(STATUS "Found libcurl: ${CURL_LIBRARIES}")
else()
    message(WARNING "libcurl not found. Webhook alerts will be disabled.")
endif()

# Source files for common library
set(COMMON_SOURCES
    src/utils.cpp
    src/fingerprint_common.cpp
    src/ja3.cpp
    src/pcap_reader.cpp
    src/alert_manager.cpp
)

set(COMMON_HEADERS
    include/utils.h
    include/fingerprint_common.h
    include/ja3.h
    include/pcap_reader.h
    include/alert_manager.h
)

# Create common library
add_library(fingerprint_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
target_link_libraries(fingerprint_common 
    OpenSSL::SSL 
    OpenSSL::Crypto
)
if(WIN32)
    target_link_libraries(fingerprint_common ws2_32)
endif()

# Link optional libraries if found
if(LIBPCAP_FOUND)
    target_link_libraries(fingerprint_common ${LIBPCAP_LIBRARIES})
    target_include_directories(fingerprint_common PRIVATE ${LIBPCAP_INCLUDE_DIRS})
endif()

if(CURL_FOUND)
    target_link_libraries(fingerprint_common ${CURL_LIBRARIES})
endif()

# TLS fingerprint collector
add_executable(fingerprint_tls src/fingerprint_tls.cpp)
target_link_libraries(fingerprint_tls fingerprint_common)
if(WIN32)
    target_link_libraries(fingerprint_tls ws2_32)
endif()

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

# PCAP fingerprint extractor (Week 16)
add_executable(fingerprint_pcap src/fingerprint_pcap.cpp)
target_link_libraries(fingerprint_pcap fingerprint_common)

# Installation
install(TARGETS fingerprint_tls fingerprint_ssh baseline_diff visualize generate_eval_set fingerprint_pcap
    RUNTIME DESTINATION bin
)

# Create data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

