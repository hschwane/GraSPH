#pragma once

#include "mathConst.glsl"

// -----------------------------------------------------------------------------------------------------
// Wpoly6

// use the poly 6 kernel to perform smoothing
// r2 the quared distance of the two particles
// factor is the result of the function poly6Factor below
// h2 is the square of h
float Wpoly6(float r2, float factor, float h2)
{
    return (r2 < h2) ? factor * pow(h2 - r2,3) : 0;
}

// calculate the factor for use in the poly 6 kernel
float poly6Factor(float h)
{
    return (315 / (64* PI * pow(h,9)));
}

// use the poly 6 kernel to perform smoothing
// h2 and the factor will be computed internally
// hint: use this function if a gradient is only computed once for each value of h
//       if not use the above functions to reuse h2 and the factor
// r2 the quared distance of the two particles
// h is the smoothing length
float Wpoly6(float r2, float h)
{
    const float h2 = h*h;
    return (r2 < h2) ? (315 / (64* PI * pow(h,9))) * pow(h2 - r2,3) : 0;
}

// ----------------
// Wpoly6 gradient

// use the gradient of the poly 6 kernel to perform smoothing
// rij is a vector posi - posj
// r2 the quared distance of the two particles
// factor is the result of the function poly6GradFactor below
// h2 is the square of h
vec3 Wpoly6Grad(vec3 rij, float r2, float factor, float h2)
{
    return (r2 < h2) ? factor * pow(h2 - r2,2) * rij  : vec3(0);
}

// calculate the factor for use in the poly 6 kernel gradient
float poly6GradFactor(float h)
{
    return (945 / (32* PI * pow(h,9)));
}

// use the gradient of the poly 6 kernel to perform smoothing
// h2 and the factor will be computed internally
// hint: use this function if a gradient is only computed once for each value of h
//       if not use the above functions to reuse h2 and the factor
// rij is a vector posi - posj
// r2 the quared distance of the two particles
// h is the smoothing length
vec3 Wpoly6Grad(vec3 rij, float r2, float h)
{
    const float h2 = h*h;
    return (r2 < h2) ? (945 / (32* PI * pow(h,9))) * pow(h2 - r2,2) * rij  : vec3(0);
}

// -----------------------------------------------------------------------------------------------------
// WspikyGrad

// perform smoothing using the gradient of the spiky kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
vec3 WspikyGrad(vec3 rij, float dist  ,float h)
{
    return (dist < h && dist != 0) ? (-45 / (PI * pow(h,6))) * pow(h - dist,2)* rij/dist : vec3(0,0,0);
}

// -----------------------------------------------------------------------------------------------------
// WviscLap

// perform smoothing using the laplacian of the viscosity kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
float WviscLap(float dist, float h)
{
    return (dist < h) ? (45 / (PI * pow(h,6))) * (h - dist) :0;
}