cmake_minimum_required(VERSION 3.16)
enable_testing()

project(server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system coroutine)
find_package(Threads REQUIRED)

# Add source files
add_executable(server main.cpp)

# Link Boost.Beast with other Boost libraries
target_link_libraries(server
    Boost::boost
    Boost::system
    Boost::coroutine
    Threads::Threads
)

add_subdirectory(test)
