cmake_minimum_required(VERSION 3.16)
enable_testing()

project(server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find Boost libraries
find_package(Boost REQUIRED ALL)
find_package(Threads REQUIRED)

# Fetch some deps
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.1.2
)
FetchContent_MakeAvailable(fmt)

# Add source files
add_executable(server main.cpp)

# Link Boost.Beast with other Boost libraries
target_link_libraries(server
    fmt::fmt
    nlohmann_json::nlohmann_json
    ${Boost_ALL_TARGETS}
    Threads::Threads
)

add_subdirectory(test)
