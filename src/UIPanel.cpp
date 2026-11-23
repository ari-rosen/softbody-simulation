#include "UIPanel.hpp"
#include "Simulation.hpp"
#include "SoftBody.hpp"
#include <algorithm>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

UIPanel::UIPanel(Simulation &simulation)
    : _simulation(simulation)
{}

void UIPanel::render() {
    float screenWidth = static_cast<float>(GetScreenWidth());

    Color baseColor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
    Color toolbarColor = Color{ 
        static_cast<unsigned char>(std::min(255, baseColor.r + 25)), 
        static_cast<unsigned char>(std::min(255, baseColor.g + 25)), 
        static_cast<unsigned char>(std::min(255, baseColor.b + 25)), 
        baseColor.a 
    };
    DrawRectangle(0, 0, static_cast<int>(screenWidth), 55, toolbarColor);

    if (GuiButton((Rectangle){ 15, 12, 115, 30 }, "Add Soft Body")) {
        SoftBody newBody(Vector2{ screenWidth / 2.0f, 150.0f }, _spawnRadius, _spawnResolution, _spawnColor);
        _simulation.addSoftBody(newBody);
    }

    GuiLabel((Rectangle){ 138, 17, 75, 20 }, TextFormat("Radius: %.0f", _spawnRadius));
    GuiSliderBar((Rectangle){ 215, 17, 85, 20 }, "", "", &_spawnRadius, 20.0f, 400.0f);

    GuiLabel((Rectangle){ 308, 17, 42, 20 }, "Color:");
    Rectangle colorBoxRect = { 352, 15, 26, 24 };
    DrawRectangleRec(colorBoxRect, _spawnColor);
    DrawRectangleLinesEx(colorBoxRect, 1, BLACK);

    if (CheckCollisionPointRec(GetMousePosition(), colorBoxRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        _showColorPicker = !_showColorPicker;
        if (_showColorPicker) {
            _showGravityDropdown = false;
            _showAdvancedDropdown = false;
        }
    }

    if (GuiButton((Rectangle){ 395, 12, 110, 30 }, "Gravity")) {
        _showGravityDropdown = !_showGravityDropdown;
        if (_showGravityDropdown) {
            _showColorPicker = false;
            _showAdvancedDropdown = false;
        }
    }

    if (GuiButton((Rectangle){ 515, 12, 90, 30 }, "Load Demo")) {
        _simulation.clearSoftBodies();
        _simulation.addSoftBody(SoftBody(Vector2{800, 200}, 100, _spawnResolution, BLUE));
        _simulation.addSoftBody(SoftBody(Vector2{825, 600}, 150, _spawnResolution, RED));
        _simulation.addSoftBody(SoftBody(Vector2{300, 100}, 250, _spawnResolution, PURPLE));
        _simulation.addSoftBody(SoftBody(Vector2{1200, 200}, 200, _spawnResolution, GREEN));
        _simulation.addSoftBody(SoftBody(Vector2{700, 400}, 50, _spawnResolution, ORANGE));
        _simulation.addSoftBody(SoftBody(Vector2{1300, 600}, 75, _spawnResolution, BLACK));
    }

    if (GuiButton((Rectangle){ screenWidth - 215, 12, 95, 30 }, "Advanced")) {
        _showAdvancedDropdown = !_showAdvancedDropdown;
        if (_showAdvancedDropdown) {
            _showColorPicker = false;
            _showGravityDropdown = false;
        }
    }

    if (GuiButton((Rectangle){ screenWidth - 110, 12, 95, 30 }, "Clear All")) {
        _simulation.clearSoftBodies();
    }

    if (_showColorPicker) {
        GuiPanel((Rectangle){ 308, 60, 200, 210 }, "Body Color");
        GuiColorPicker((Rectangle){ 318, 85, 180, 145 }, nullptr, &_spawnColor);

        if (GuiButton((Rectangle){ 318, 235, 180, 25 }, "Done")) {
            _showColorPicker = false;
        }
    }

    if (_showGravityDropdown) {
        GuiPanel((Rectangle){ 395, 60, 210, 160 }, "Gravity Settings");

        GuiLabel((Rectangle){ 405, 90, 60, 20 }, TextFormat("X: %.0f", _simulation.gravity.x));
        GuiSliderBar((Rectangle){ 465, 92, 125, 16 }, "", "", &_simulation.gravity.x, -2000.0f, 2000.0f);

        GuiLabel((Rectangle){ 405, 120, 60, 20 }, TextFormat("Y: %.0f", _simulation.gravity.y));
        GuiSliderBar((Rectangle){ 465, 122, 125, 16 }, "", "", &_simulation.gravity.y, 0.0f, 2000.0f);

        if (GuiButton((Rectangle){ 405, 155, 185, 25 }, "Reset to Default")) {
            _simulation.gravity = { 0.0f, 980.0f };
        }

        if (GuiButton((Rectangle){ 405, 188, 185, 25 }, "Close")) {
            _showGravityDropdown = false;
        }
    }

    if (_showAdvancedDropdown) {
        float panelWidth = 300.0f;
        float panelX = screenWidth - panelWidth - 15;
        GuiPanel((Rectangle){ panelX, 60, panelWidth, 360 }, "Advanced Options");

        float labelWidth = 130.0f;
        float controlX = panelX + labelWidth + 15;
        float controlWidth = panelWidth - labelWidth - 30;

        GuiLabel((Rectangle){ panelX + 15, 90, labelWidth, 20 }, TextFormat("Resolution: %d", _spawnResolution));
        GuiSpinner((Rectangle){ controlX, 90, controlWidth, 20 }, nullptr, &_spawnResolution, 3, 64, false);

        GuiLabel((Rectangle){ panelX + 15, 120, labelWidth, 20 }, TextFormat("Substeps: %d", _simulation.substeps));
        GuiSpinner((Rectangle){ controlX, 120, controlWidth, 20 }, nullptr, &_simulation.substeps, 1, 10, false);

        GuiLabel((Rectangle){ panelX + 15, 150, labelWidth, 20 }, TextFormat("Iterations: %d", _simulation.constraintIterations));
        GuiSpinner((Rectangle){ controlX, 150, controlWidth, 20 }, nullptr, &_simulation.constraintIterations, 1, 30, false);

        GuiLabel((Rectangle){ panelX + 15, 180, labelWidth, 20 }, TextFormat("Col Radius: %.1f", _simulation.collisionRadius));
        GuiSliderBar((Rectangle){ controlX, 182, controlWidth, 16 }, "", "", &_simulation.collisionRadius, 1.0f, 20.0f);

        GuiLabel((Rectangle){ panelX + 15, 210, labelWidth, 20 }, TextFormat("Relaxation: %.2f", _simulation.collisionRelaxation));
        GuiSliderBar((Rectangle){ controlX, 212, controlWidth, 16 }, "", "", &_simulation.collisionRelaxation, 0.0f, 1.0f);

        GuiLabel((Rectangle){ panelX + 15, 240, labelWidth, 20 }, TextFormat("Max Correc: %.1f", _simulation.maxCorrection));
        GuiSliderBar((Rectangle){ controlX, 242, controlWidth, 16 }, "", "", &_simulation.maxCorrection, 0.1f, 20.0f);

        bool fanToggle = _simulation.renderFan;
        GuiCheckBox((Rectangle){ panelX + 15, 272, 20, 20 }, "Render Fan", &fanToggle);
        _simulation.renderFan = fanToggle;

        bool pointsToggle = _simulation.renderPoints;
        GuiCheckBox((Rectangle){ panelX + 15, 302, 20, 20 }, "Render Points", &pointsToggle);
        _simulation.renderPoints = pointsToggle;

        float btnWidth = (panelWidth - 45.0f) / 2.0f;
        if (GuiButton((Rectangle){ panelX + 15, 335, btnWidth, 25 }, "Reset Defaults")) {
            _simulation.substeps = 2;
            _simulation.constraintIterations = 6;
            _simulation.collisionRadius = 1.5f;
            _simulation.collisionRelaxation = 0.1f;
            _simulation.maxCorrection = 3.0f;
            _simulation.renderFan = false;
            _simulation.renderPoints = false;
            _spawnResolution = 20;
        }

        // Close Button
        if (GuiButton((Rectangle){ panelX + 30 + btnWidth, 335, btnWidth, 25 }, "Close")) {
            _showAdvancedDropdown = false;
        }
    }
}
