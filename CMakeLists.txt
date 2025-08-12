cmake_minimum_required(VERSION 3.10)
project(particle_sim C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Source files
set(SOURCES main.c)

# Find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(particle_sim ${SOURCES})

target_include_directories(particle_sim PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
)

target_link_libraries(particle_sim
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        m
)