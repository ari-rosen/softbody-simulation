cmake_minimum_required(VERSION 3.22)
project(SoftBodySimulation CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        5.0
)

FetchContent_MakeAvailable(raylib)

add_executable(soft_body_sim 
    src/main.cpp
    src/Application.cpp
    src/SoftBody.cpp
    src/Particle.cpp
    src/Simulation.cpp
    src/UIPanel.cpp
)

target_link_libraries(soft_body_sim 
    PRIVATE 
        raylib
)

target_include_directories(soft_body_sim 
    PRIVATE 
        include
        ${raygui_SOURCE_DIR}/src
)
