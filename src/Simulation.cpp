#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

float Simulation::inverseMass(const Particle& particle) {
    return particle.mass > epsilon ? 1.0f / particle.mass : 0.0f;
}

bool Simulation::isPointInside(Vector2 point, const std::vector<Particle>& particles, int resolution) {
    bool inside = false;

    for (int i = 0, j = resolution - 1; i < resolution; j = i++) {
        Vector2 a = particles[i].position;
        Vector2 b = particles[j].position;

        if ((a.y > point.y) != (b.y > point.y)) {
            float intersectionX = (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x;

            if (point.x < intersectionX) {
                inside = !inside;
            }
        }
    }

    return inside;
}

Vector2 Simulation::closestPointOnSegment(Vector2 point, Vector2 a, Vector2 b, float& t) {
    Vector2 edge = { b.x - a.x, b.y - a.y };
    float lengthSq = edge.x * edge.x + edge.y * edge.y;

    if (lengthSq < epsilon) {
        t = 0.0f;
        return a;
    }

    Vector2 toPoint = { point.x - a.x, point.y - a.y };
    t = std::clamp((toPoint.x * edge.x + toPoint.y * edge.y) / lengthSq, 0.0f, 1.0f);

    return { a.x + t * edge.x, a.y + t * edge.y };
}

void Simulation::addSoftBody(const SoftBody& body) {
    _softBodies.push_back(body);
}

void Simulation::update(float dt, Vector2 gravity) {
    if (dt <= 0.0f) return;

    const float subDt = dt / substeps;

    for (int step = 0; step < substeps; ++step) {
        integratePositions(subDt, gravity);
        solveAllConstraints();
        resolveWorldBoundaries();
    }
}

void Simulation::integratePositions(float subDt, Vector2 gravity) {
    for (auto& body : _softBodies) {
        body.applyForces(gravity);
        body.updatePositions(subDt);
    }
}

void Simulation::solveAllConstraints() {
    for (int iteration = 0; iteration < constraintIterations; ++iteration) {
        applyMouseConstraint(0.25f);

        for (auto& body : _softBodies) {
            body.solveConstraints(1, 0.8f);
        }

        handleInterBodyCollisions();
    }
}

void Simulation::resolveWorldBoundaries() {
    for (auto& body : _softBodies) {
        body.resolveBoundaries(0.0f, 1600.0f, 900.0f, 1.0f, 0.0f);
    }
}

void Simulation::handleMouseInput(Vector2 mousePos, bool isDown, float grabRadius) {
    if (!isDown) {
        resetGrabState();
        return;
    }

    _grab.targetPos = mousePos;

    if (_grab.active) {
        return;
    }

    findAndGrabBody(mousePos, grabRadius);
}

void Simulation::resetGrabState() {
    _grab.active = false;
    _grab.bodyIndex = -1;
    _grab.particleIndices.clear();
    _grab.weights.clear();
}

void Simulation::findAndGrabBody(Vector2 mousePos, float grabRadius) {
    const float radiusSq = grabRadius * grabRadius;

    for (int i = static_cast<int>(_softBodies.size()) - 1; i >= 0; --i) {
        auto& particles = _softBodies[i].getParticles();
        int res = _softBodies[i].getResolution();

        bool clickedInside = (res >= 3) && isPointInside(mousePos, particles, res);
        
        std::vector<int> candidateIndices;
        std::vector<float> candidateWeights;

        collectCandidateParticles(particles, mousePos, grabRadius, radiusSq, candidateIndices, candidateWeights);

        if (clickedInside && candidateIndices.empty() && !particles.empty()) {
            addFallbackNearestParticle(particles, mousePos, candidateIndices, candidateWeights);
        }

        if (!candidateIndices.empty()) {
            activateGrab(i, std::move(candidateIndices), std::move(candidateWeights));
            break;
        }
    }
}

void Simulation::collectCandidateParticles(const std::vector<Particle>& particles, Vector2 mousePos, float grabRadius, float radiusSq, std::vector<int>& candidateIndices, std::vector<float>& candidateWeights) {
    for (int p = 0; p < static_cast<int>(particles.size()); ++p) {
        if (particles[p].mass <= epsilon) continue;

        float dx = particles[p].position.x - mousePos.x;
        float dy = particles[p].position.y - mousePos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= radiusSq) {
            float weight = 1.0f - (std::sqrt(distSq) / grabRadius);
            candidateIndices.push_back(p);
            candidateWeights.push_back(weight);
        }
    }
}

void Simulation::addFallbackNearestParticle(const std::vector<Particle>& particles, Vector2 mousePos, std::vector<int>& candidateIndices, std::vector<float>& candidateWeights) {
    int nearestIdx = -1;
    float minDistSq = std::numeric_limits<float>::max();

    for (int p = 0; p < static_cast<int>(particles.size()); ++p) {
        if (particles[p].mass <= epsilon) continue;
        float dx = particles[p].position.x - mousePos.x;
        float dy = particles[p].position.y - mousePos.y;
        float dSq = dx * dx + dy * dy;
        if (dSq < minDistSq) {
            minDistSq = dSq;
            nearestIdx = p;
        }
    }

    if (nearestIdx != -1) {
        candidateIndices.push_back(nearestIdx);
        candidateWeights.push_back(1.0f);
    }
}

void Simulation::activateGrab(int bodyIndex, std::vector<int> indices, std::vector<float> weights) {
    _grab.active = true;
    _grab.bodyIndex = bodyIndex;
    _grab.particleIndices = std::move(indices);
    _grab.weights = std::move(weights);
}

void Simulation::applyMouseConstraint(float stiffness) {
    if (!_grab.active || _grab.bodyIndex < 0 || _grab.bodyIndex >= static_cast<int>(_softBodies.size())) {
        return;
    }

    auto& particles = _softBodies[_grab.bodyIndex].getParticles();

    for (size_t i = 0; i < _grab.particleIndices.size(); ++i) {
        int idx = _grab.particleIndices[i];
        float weight = _grab.weights[i];
        Particle& p = particles[idx];

        Vector2 delta = { _grab.targetPos.x - p.position.x, _grab.targetPos.y - p.position.y };

        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        const float maxPull = 25.0f; 
        if (dist > maxPull) {
            delta.x = (delta.x / dist) * maxPull;
            delta.y = (delta.y / dist) * maxPull;
        }

        p.position.x += delta.x * stiffness * weight;
        p.position.y += delta.y * stiffness * weight;
    }
}

void Simulation::handleInterBodyCollisions() {
    const int bodyCount = static_cast<int>(_softBodies.size());

    if (bodyCount < 2) return;

    for (int i = 0; i < bodyCount; ++i) {
        for (int j = i + 1; j < bodyCount; ++j) {
            SoftBody& bodyA = _softBodies[i];
            SoftBody& bodyB = _softBodies[j];

            if (!CheckCollisionRecs(bodyA.getBoundingBox(), bodyB.getBoundingBox())) {
                continue;
            }

            resolveBodyCollision(bodyA, bodyB);
            resolveBodyCollision(bodyB, bodyA);
        }
    }
}

float Simulation::calculatePolygonWinding(const std::vector<Particle>& particles, int resolution) {
    float signedArea = 0.0f;

    for (int i = 0; i < resolution; ++i) {
        int next = (i + 1) % resolution;

        const Vector2& a = particles[i].position;
        const Vector2& b = particles[next].position;

        signedArea += a.x * b.y - b.x * a.y;
    }

    return signedArea;
}

void Simulation::resolveBodyCollision(SoftBody& bodyA, SoftBody& bodyB) {
    auto& particlesA = bodyA.getParticles();
    auto& particlesB = bodyB.getParticles();

    const int resB = bodyB.getResolution();

    if (resB < 3) return;

    float signedArea = calculatePolygonWinding(particlesB, resB);

    for (auto& pa : particlesA) {
        float nearestDistanceSq = std::numeric_limits<float>::max();
        Vector2 nearestPoint = {};
        int nearestEdge = -1;
        float nearestT = 0.0f;

        findNearestEdgeForParticle(pa, particlesB, resB, nearestPoint, nearestEdge, nearestT, nearestDistanceSq);

        if (nearestEdge < 0) continue;

        bool inside = isPointInside(pa.position, particlesB, resB);
        float distance = std::sqrt(nearestDistanceSq);

        if (!inside && distance >= collisionRadius) {
            continue;
        }

        Vector2 normal = calculateCollisionNormal(pa, particlesB, nearestEdge, nearestPoint, distance, inside, signedArea);
        if (normal.x == 0.0f && normal.y == 0.0f) continue;

        float penetration = inside ? distance + collisionRadius : collisionRadius - distance;

        if (penetration <= 0.0f) continue;

        applyParticleCorrection(pa, particlesB[nearestEdge], particlesB[(nearestEdge + 1) % resB], normal, penetration, nearestT);
    }
}

void Simulation::findNearestEdgeForParticle(const Particle& pa, const std::vector<Particle>& particlesB, int resB, Vector2& nearestPoint, int& nearestEdge, float& nearestT, float& nearestDistanceSq) {
    for (int i = 0; i < resB; ++i) {
        int next = (i + 1) % resB;
        float t = 0.0f;

        Vector2 closest = closestPointOnSegment(pa.position, particlesB[i].position, particlesB[next].position, t);

        float dx = pa.position.x - closest.x;
        float dy = pa.position.y - closest.y;
        float distanceSq = dx * dx + dy * dy;

        if (distanceSq < nearestDistanceSq) {
            nearestDistanceSq = distanceSq;
            nearestPoint = closest;
            nearestEdge = i;
            nearestT = t;
        }
    }
}

Vector2 Simulation::calculateCollisionNormal(const Particle& pa, const std::vector<Particle>& particlesB, int nearestEdge, Vector2 nearestPoint, float distance, bool inside, float signedArea) {
    int next = (nearestEdge + 1) % particlesB.size();
    Vector2 normal = {};

    if (distance > epsilon) {
        float direction = inside ? -1.0f : 1.0f;
        normal = {
            direction * (pa.position.x - nearestPoint.x) / distance,
            direction * (pa.position.y - nearestPoint.y) / distance
        };
    } else {
        Vector2 edge = {
            particlesB[next].position.x - particlesB[nearestEdge].position.x,
            particlesB[next].position.y - particlesB[nearestEdge].position.y
        };

        float length = std::sqrt(edge.x * edge.x + edge.y * edge.y);

        if (length < epsilon) return { 0.0f, 0.0f };

        if (signedArea > 0.0f) {
            normal = { edge.y / length, -edge.x / length };
        } else {
            normal = { -edge.y / length, edge.x / length };
        }
    }

    return normal;
}

void Simulation::applyParticleCorrection(Particle& pa, Particle& b1, Particle& b2, Vector2 normal, float penetration, float nearestT) {
    float wA = inverseMass(pa);
    float wB1 = inverseMass(b1);
    float wB2 = inverseMass(b2);

    float weightB1 = wB1 * (1.0f - nearestT);
    float weightB2 = wB2 * nearestT;

    float totalWeight = wA + weightB1 + weightB2;

    if (totalWeight < epsilon) return;

    float correction = std::min(penetration * collisionRelaxation, maxCorrection);
    correction /= totalWeight;

    pa.position.x += normal.x * correction * wA;
    pa.position.y += normal.y * correction * wA;

    b1.position.x -= normal.x * correction * weightB1;
    b1.position.y -= normal.y * correction * weightB1;

    b2.position.x -= normal.x * correction * weightB2;
    b2.position.y -= normal.y * correction * weightB2;
}

void Simulation::draw() const {
    for (const auto& body : _softBodies) {
        body.draw();
    }
}
