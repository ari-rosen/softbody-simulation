#include "SoftBody.hpp"
#include "raymath.h"
#include <cmath>

SoftBody::SoftBody(Vector2 center, float radius, int32_t resolution, Color color) 
    : _resolution(resolution), _targetArea(0.0f), _color(color)
{
    for (int i = 0; i < resolution; i++) {
        float angle = (float)i / resolution * 2.0f * PI;
        Vector2 pos = {
            center.x + std::cos(angle) * radius,
            center.y + std::sin(angle) * radius
        };
        _particles.emplace_back(pos, 1.0f);
    }

    for (int i = 0; i < resolution; ++i) {
        int next = (i + 1) % resolution;
        float dist = Vector2Distance(_particles[i].position, _particles[next].position);
        _constraints.emplace_back(i, next, dist);
    }

    calculateArea();
}

void SoftBody::calculateArea() {
    _targetArea = 0.0f;
    for (int i = 0; i < _resolution; ++i) {
        const Vector2& p1 = _particles[i].position;
        const Vector2& p2 = _particles[(i + 1) % _resolution].position;
        _targetArea += (p1.x * p2.y - p2.x * p1.y);
    }
    _targetArea = std::abs(_targetArea) * 0.5f;
}

Vector2 SoftBody::calculateCenterOfMass() const {
    Vector2 center = {0.0f, 0.0f};
    for (const auto& p : _particles) {
        center.x += p.position.x;
        center.y += p.position.y;
    }
    center.x /= _resolution;
    center.y /= _resolution;
    return center;
}

void SoftBody::applyForces(Vector2 globalAcceleration) {
    // 1. Calculate current area & pressure
    float currentArea = 0.0f;
    for (int i = 0; i < _resolution; ++i) {
        const Vector2& p1 = _particles[i].position;
        const Vector2& p2 = _particles[(i + 1) % _resolution].position;
        currentArea += (p1.x * p2.y - p2.x * p1.y);
    }
    currentArea = std::abs(currentArea) * 0.5f;

    float areaDiff = _targetArea - currentArea;
    float pressureStrength = 16.0f;
    Vector2 centerOfMass = calculateCenterOfMass();

    // 2. Apply external forces + internal pressure
    for (auto& p : _particles) {
        p.applyForce(globalAcceleration);

        Vector2 dir = {p.position.x - centerOfMass.x, p.position.y - centerOfMass.y};
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 0.0001f) {
            dir.x /= dist;
            dir.y /= dist;
            Vector2 pressureForce = {
                dir.x * areaDiff * pressureStrength, 
                dir.y * areaDiff * pressureStrength
            };
            p.applyForce(pressureForce);
        }
    }
}

void SoftBody::updatePositions(float dt) {
    for (auto& p : _particles) {
        p.update(dt);
    }
}

void SoftBody::solveConstraints(int constraintIterations, float springStiffness) {
    for (int iter = 0; iter < constraintIterations; ++iter) {
        for (auto& c : _constraints) {
            Particle& p1 = _particles[c.p1Index];
            Particle& p2 = _particles[c.p2Index];

            float dx = p2.position.x - p1.position.x;
            float dy = p2.position.y - p1.position.y;
            float currentDist = std::sqrt(dx * dx + dy * dy);
        
            if (currentDist < 0.0001f) continue;

            float diff = (currentDist - c.restLength) / currentDist;
            if (diff > 1.5f) diff = 1.5f; 
            if (diff < -1.5f) diff = -1.5f;

            p1.position.x += dx * springStiffness * diff;
            p1.position.y += dy * springStiffness * diff;
            p2.position.x -= dx * springStiffness * diff;
            p2.position.y -= dy * springStiffness * diff;
        }
    }
}

void SoftBody::resolveBoundaries(float leftWall, float rightWall, float floorY, float bounce, float friction) {
    for (auto& p : _particles) {
        if (p.position.y > floorY) {
            float velocityY = p.position.y - p.oldPosition.y;
            p.position.y = floorY; 
            p.oldPosition.y = p.position.y + velocityY * bounce;
            
            float velocityX = p.position.x - p.oldPosition.x;
            p.oldPosition.x = p.position.x - velocityX * friction;
        }
        if (p.position.x < leftWall) {
            float velocityX = p.position.x - p.oldPosition.x;
            p.position.x = leftWall;
            p.oldPosition.x = p.position.x + velocityX * bounce;
        }
        if (p.position.x > rightWall) {
            float velocityX = p.position.x - p.oldPosition.x;
            p.position.x = rightWall;
            p.oldPosition.x = p.position.x + velocityX * bounce;
        }
    }
}

Rectangle SoftBody::getBoundingBox() const {
    if (_particles.empty()) return {0, 0, 0, 0};

    float minX = _particles[0].position.x;
    float maxX = _particles[0].position.x;
    float minY = _particles[0].position.y;
    float maxY = _particles[0].position.y;

    for (const auto& p : _particles) {
        minX = std::min(minX, p.position.x);
        maxX = std::max(maxX, p.position.x);
        minY = std::min(minY, p.position.y);
        maxY = std::max(maxY, p.position.y);
    }

    return {minX, minY, maxX - minX, maxY - minY};
}


void SoftBody::draw(bool renderFan, bool renderPoints) const {
    if (renderFan) {
        if (_particles.size() < 3) return;

        Vector2 center = calculateCenterOfMass();
        
        // Render slice triangles with swapped p2/p1 order to fix screen-space winding/visibility
        for (int i = 0; i < _resolution; ++i) {
            Vector2 p1 = _particles[i].position;
            Vector2 p2 = _particles[(i + 1) % _resolution].position;
            DrawTriangle(center, p2, p1, _color);
        }
    } else {
        // Render as Catmull-Rom Spline
        std::vector<Vector2> outerPoints;
        outerPoints.push_back(_particles[_resolution - 1].position);
        for (int i = 0; i < _resolution; ++i) {
            outerPoints.push_back(_particles[i].position);
        }
        outerPoints.push_back(_particles[0].position);
        outerPoints.push_back(_particles[1].position);

        DrawSplineCatmullRom(outerPoints.data(), outerPoints.size(), 10.0f, _color);
    }
    if (renderPoints) {
        for (const auto& p : _particles) {
            DrawCircleV(p.position, 3.0f, BLACK);
        }
    }
}
