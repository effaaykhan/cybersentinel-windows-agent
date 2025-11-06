# CyberSentinel DLP Windows Agent - CMake Build Configuration
cmake_minimum_required(VERSION 3.20)

project(CyberSentinelAgent VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dependencies
find_package(CURL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json/include
    ${CURL_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/agent.cpp
    src/file_monitor.cpp
    src/clipboard_monitor.cpp
    src/usb_monitor.cpp
    src/http_client.cpp
    src/classifier.cpp
    src/config.cpp
    src/logger.cpp
)

# Header files
set(HEADERS
    include/agent.h
    include/file_monitor.h
    include/clipboard_monitor.h
    include/usb_monitor.h
    include/http_client.h
    include/classifier.h
    include/config.h
    include/logger.h
)

# Executable
add_executable(CyberSentinelAgent ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(CyberSentinelAgent
    ${CURL_LIBRARIES}
    ws2_32
    wbemuuid
    ole32
    oleaut32
)

# Compiler flags
if(MSVC)
    target_compile_options(CyberSentinelAgent PRIVATE /W4 /WX)
    target_compile_definitions(CyberSentinelAgent PRIVATE _WIN32_WINNT=0x0601)
else()
    target_compile_options(CyberSentinelAgent PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
install(TARGETS CyberSentinelAgent DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/agent_config.json DESTINATION bin)
