#pragma once

struct Constraint {
    int p1Index;
    int p2Index;
    float restLength;

    Constraint(int p1, int p2, float length) 
        : p1Index(p1), p2Index(p2), restLength(length) {}
};
