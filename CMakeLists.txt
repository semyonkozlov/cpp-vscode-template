cmake_minimum_required(VERSION 3.16)
enable_testing()

project(server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system coroutine)
find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Add source files
add_executable(server main.cpp)

# Link Boost.Beast with other Boost libraries
target_link_libraries(server
    nlohmann_json::nlohmann_json
    Boost::boost
    Boost::system
    Boost::coroutine
    Threads::Threads
)

add_subdirectory(test)
