#include "Application.hpp"
#include "raylib.h"


#include "raygui.h"


Application::Application(std::string_view title, int32_t width, int32_t height)
    : _title(title), _width(width), _height(height), _running(true), _uiPanel(_simulation)
{}

void Application::run() {
    start();
    while (_running) {
        update();
        render();
    }
}

void Application::start() {

    InitWindow(_width, _height, _title.c_str());

    GuiSetStyle(DEFAULT, TEXT_SIZE, 12);
    SetTargetFPS(60);
    
    _simulation.addSoftBody({{800, 200}, 100, 32, BLUE});
    _simulation.addSoftBody({{825, 600}, 150, 32, RED});
    _simulation.addSoftBody({{300, 100}, 250, 32, PURPLE});
    _simulation.addSoftBody({{1200, 200}, 200, 32, GREEN});
    _simulation.addSoftBody({{700, 400}, 50, 32, ORANGE});
    _simulation.addSoftBody({{1300, 600}, 75, 32, BLACK});
}

void Application::update() {
    _running = !WindowShouldClose();

    float dt = GetFrameTime();

    _simulation.handleMouseInput(GetMousePosition(), IsMouseButtonDown(MOUSE_BUTTON_LEFT), 20.0f);
    _simulation.update(dt);
}

void Application::render() {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    _simulation.draw();
    _uiPanel.render();

    EndDrawing();
}
