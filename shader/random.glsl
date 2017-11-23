#pragma once

#define PI          3.141592653589793238462643383279502884197169399375105820974
#define PI_INV      1/PI
#define UINT_MAX_F 4294967295.0f

// ----------------------------------------------------------------------------
// some helper functions

//Method to generate a pseudo-random seed.
uint WangHash(in uint a)
{
	a = (a ^ uint(61)) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}

// xor shift as part of the rnd
uint rng_state = 0;
uint rand_xorshift()
{
	// Xorshift algorithm from George Marsaglia's paper
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return rng_state;
}

// ----------------------------------------------------------------------------
// functions you actually want to use

// generate a random number from the seed number will be inbetween 0 and 1
float rand(uint seed)
{
	rng_state += WangHash(seed);
	return float(rand_xorshift()) * (1.0f / UINT_MAX_F);
}

vec2 rand2(uint seed)
{
    return vec2(rand(seed),rand(seed));
}

vec3 rand3(uint seed)
{
    return vec3(rand(seed),rand(seed),rand(seed));
}

// random positions on the surface of a spere with radius 1
// pass a uniform random value [0,1] to u and v
vec3 randUniformSphere(float u, float v)
{
    float theta = 2.f * PI * u;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);

    float cosPhi = 2*v-1.f;
    float sinPhi = sqrt(max(0, 1- cosPhi*cosPhi));

    return vec3( cosTheta*sinPhi, sinTheta*sinPhi, cosPhi);
}

// random positions on the surface of a spere with radius = radius
// pass a uniform random value [0,1] to u,v pass 1 for r to generate points on the surface
// or pass a uniform random value [0,1] to u,v,r to generate points inside the sphere with a max radius of radius
vec3 randSphere(float u, float v, float r, float radius)
{
    float theta = 2.f * PI * u;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);

    float cosPhi = 2*v-1.f;
    float sinPhi = sqrt(max(0, 1- cosPhi*cosPhi));

    return vec3( r*radius*cosTheta*sinPhi, r*radius*sinTheta*sinPhi, r*radius*cosPhi);
}