#pragma once

#include <vector>
#include "SoftBody.hpp"
#include "raylib.h"

class Simulation {
public:
    int substeps = 2;
    int constraintIterations = 6;
    float collisionRadius = 5.0f;
    float collisionRelaxation = 0.1f;
    float maxCorrection = 3.0f;
    float epsilon = 0.0001f;

    void addSoftBody(const SoftBody& body);
    void update(float dt, Vector2 gravity);
    void handleMouseInput(Vector2 mousePos, bool isDown, float grabRadius);
    void draw() const;

private:
    std::vector<SoftBody> _softBodies;

    struct GrabState {
        bool active = false;
        int bodyIndex = -1;
        Vector2 targetPos = { 0.0f, 0.0f };
        std::vector<int> particleIndices;
        std::vector<float> weights;
    } _grab;

    float inverseMass(const Particle& particle);
    bool isPointInside(Vector2 point, const std::vector<Particle>& particles, int resolution);
    Vector2 closestPointOnSegment(Vector2 point, Vector2 a, Vector2 b, float& t);

    void integratePositions(float subDt, Vector2 gravity);
    void solveAllConstraints();
    void resolveWorldBoundaries();

    void resetGrabState();
    void findAndGrabBody(Vector2 mousePos, float grabRadius);
    void collectCandidateParticles(const std::vector<Particle>& particles, Vector2 mousePos, float grabRadius, float radiusSq, std::vector<int>& candidateIndices, std::vector<float>& candidateWeights);
    void addFallbackNearestParticle(const std::vector<Particle>& particles, Vector2 mousePos, std::vector<int>& candidateIndices, std::vector<float>& candidateWeights);
    void activateGrab(int bodyIndex, std::vector<int> indices, std::vector<float> weights);
    void applyMouseConstraint(float stiffness);

    void handleInterBodyCollisions();
    float calculatePolygonWinding(const std::vector<Particle>& particles, int resolution);
    void resolveBodyCollision(SoftBody& bodyA, SoftBody& bodyB);
    void findNearestEdgeForParticle(const Particle& pa, const std::vector<Particle>& particlesB, int resB, Vector2& nearestPoint, int& nearestEdge, float& nearestT, float& nearestDistanceSq);
    Vector2 calculateCollisionNormal(const Particle& pa, const std::vector<Particle>& particlesB, int nearestEdge, Vector2 nearestPoint, float distance, bool inside, float signedArea);
    void applyParticleCorrection(Particle& pa, Particle& b1, Particle& b2, Vector2 normal, float penetration, float nearestT);
};
