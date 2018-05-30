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
// the vector posi - posj must be manually multiplied to the result
// r2 the quared distance of the two particles
// factor is the result of the function poly6GradFactor below
// h2 is the square of h
float Wpoly6Grad( float r2, float factor, float h2)
{
    return (r2 < h2) ? factor * (h2-r2)*(h2-r2)  : 0;
}

// calculate the factor for use in the poly 6 kernel gradient
float poly6GradFactor(float h)
{
    return (-945 / (32* PI * pow(h,9)));
}

// use the gradient of the poly 6 kernel to perform smoothing
// h2 and the factor will be computed internally
// the vector posi - posj must be manually multiplied to the result
// hint: use this function if a gradient is only computed once for each value of h
//       if not use the above functions to reuse h2 and the factor
// r2 the quared distance of the two particles
// h is the smoothing length
float Wpoly6Grad(float r2, float h)
{
    const float h2 = h*h;
    return (r2 < h2) ? (-945 / (32* PI * pow(h,9))) * (h2-r2)*(h2-r2)  : 0;
}

// -----------------------------------------------------------------------------------------------------
// WspikyGrad

vec3 WspikyGrad(vec3 rij, float dist, float h, float factor)
{
    float hdist = h-dist;
    return (dist < h && dist != 0) ? factor * hdist*hdist * rij/dist : vec3(0,0,0);
}

// calculate the factor for use in the spiky gradient
float spikyGradFactor(float h)
{
    return (-45 / (PI * pow(h,6)));
}

// perform smoothing using the gradient of the spiky kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
vec3 WspikyGrad(vec3 rij, float dist  ,float h)
{
    float hdist = h-dist;
    return (dist < h && dist != 0) ? (-45 / (PI * pow(h,6))) * hdist*hdist * rij/dist : vec3(0,0,0);
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

// -----------------------------------------------------------------------------------------------------
// Wspline

// perform smoothing using the Monaghan spline kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
float Wspline(float r, float h)
{
    float q = r/h;
    return (q <= 0.5) ? 8.0f/(PI*pow(h,3)) * (1.0f- 6*q*q + 6*pow(q,3))
          :(q <= 1) ? 16.0f/(PI*pow(h,3)) * pow(1-q,3)
          :0;
}

// perform smoothing using the gradient of Monaghan spline kernel
// rij is a vector posi - posj
// dist is ||rij||
// and h the smoothing length
vec3 WsplineGrad(vec3 rij, float dist, float h)
{

    return (dist*2 <= h) ? 48.0f/(PI*pow(h,6)) * ( 3*dist -2*h ) * rij
          :(dist <= h) ? 48.0f/(PI*pow(h,6)) * (h-dist)*(h-dist) * rij/dist
          :vec3(0);
}

