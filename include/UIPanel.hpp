#pragma once

#include "Simulation.hpp"
#include "raylib.h"
#include <cstdint>

class UIPanel {
public:
    UIPanel(Simulation &simulation);
    void render();

private:
    Simulation &_simulation;

    int32_t _spawnResolution = 20;
    float _spawnRadius = 60.0f;
    Color _spawnColor = { 220, 70, 70, 255 };
    bool _showColorPicker = false;
    bool _showGravityDropdown = false;
    bool _showAdvancedDropdown = false;
};
