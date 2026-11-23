#pragma once
#include <raylib.h>

struct Particle {
    Vector2 position;
    Vector2 oldPosition;
    Vector2 acceleration;
    
    float mass;

    Particle(Vector2 pos, float m = 1.0f);
   
    void update(float dt);

    void applyForce(Vector2 force);
};
