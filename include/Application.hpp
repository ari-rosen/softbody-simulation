#pragma once

#include <string_view>
#include <string>

#include "Simulation.hpp"

class Application {
public:
    Application(std::string_view title, int32_t width, int32_t height);

    void run();

private:
    void start();
    void update();
    void render();
private:
    Simulation _simulation;

    std::string _title;
    int32_t _width, _height;

    bool _running;

};
