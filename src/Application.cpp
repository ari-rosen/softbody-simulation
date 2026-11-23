#include "Application.hpp"
#include "raylib.h"

Application::Application(std::string_view title, int32_t width, int32_t height)
    : title(title), width(width), height(height), running(true)
{}

void Application::run() {
    start();
    while (running) {
        update();
    }
}

void Application::start() {
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
}

void Application::update() {
    running = !WindowShouldClose();

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
}
