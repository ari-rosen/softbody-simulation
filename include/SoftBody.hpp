#pragma once

#include <vector>

#include "Particle.hpp"
#include "Constraint.hpp"


class SoftBody {
public:
    SoftBody(Vector2 center, float radius, int32_t resolution, Color color);

    void applyForces(Vector2 globalAcceleration);
    void updatePositions(float dt);
    void solveConstraints(int iterations, float stiffness);
    void resolveBoundaries(float left, float right, float floorY, float bounce, float friction);

    std::vector<Particle>& getParticles() { return _particles; }
    const std::vector<Particle>& getParticles() const { return _particles; }
    int32_t getResolution() const { return _resolution; }

    Rectangle getBoundingBox() const;

    void draw(bool renderFan, bool renderPoints) const;

private:
    int32_t _resolution;
    Color _color;
    float _targetArea;
    std::vector<Particle> _particles;
    std::vector<Constraint> _constraints;

    void calculateArea();
    Vector2 calculateCenterOfMass() const;
};
