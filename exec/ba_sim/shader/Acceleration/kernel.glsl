#pragma once

#include "mathConst.glsl"

// use the poly 6 kernel to perform smoothing
// r2 the quared distance of the two particles
// h is the smoothing length
float Wpoly6(float r2, float h)
{
    float h2 = h*h;

    return (r2 < h2) ? (315 / (64* PI * pow(h,9))) * pow(h2 - r2,3) : 0;
}

// perform smoothing using the gradient of the spiky kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
vec3 WspikyGrad(vec3 rij, float dist  ,float h)
{
    return (dist < h && dist != 0) ? (-45 / (PI * pow(h,6))) * pow(h - dist,2)* rij/dist : vec3(0,0,0);
}


// perform smoothing using the laplacian of the viscosity kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
float WviscLap(float dist, float h)
{
    return (dist < h) ? (45 / (PI * pow(h,6))) * (h - dist) :0;
}