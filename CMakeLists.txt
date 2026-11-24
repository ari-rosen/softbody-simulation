cmake_minimum_required(VERSION 3.22)
project(SoftBodySimulation CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# 1. Fetch core raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch raygui (Header-only library)
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        5.0
)
FetchContent_GetProperties(raygui)
if(NOT raygui_POPULATED)
    FetchContent_Populate(raygui)
    
    # Create an INTERFACE target so raygui exports its include path automatically
    add_library(raygui INTERFACE)
    target_include_directories(raygui INTERFACE ${raygui_SOURCE_DIR}/src)
endif()

add_executable(soft_body_sim 
    src/main.cpp
    src/Application.cpp
    src/SoftBody.cpp
    src/Particle.cpp
    src/Simulation.cpp
    src/UIPanel.cpp
)

# 3. Link both raylib and raygui as targets
target_link_libraries(soft_body_sim 
    PRIVATE 
        raylib
        raygui
)

target_include_directories(soft_body_sim 
    PRIVATE 
        include
)
