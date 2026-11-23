#include "Particle.hpp"

Particle::Particle(Vector2 pos, float m)
    : position(pos), oldPosition(pos), acceleration({0.0f, 0.0f}), mass(m)
{}

void Particle::update(float dt) {
    Vector2 velocity = {
        position.x - oldPosition.x,
        position.y - oldPosition.y
    };

    oldPosition = position;

    position.x += velocity.x + acceleration.x * (dt * dt);
    position.y += velocity.y + acceleration.y * (dt * dt);

    acceleration = {0.0f, 0.0f};
}

void Particle::applyForce(Vector2 force) {
    acceleration.x += force.x / mass;
    acceleration.y += force.y / mass;
}
