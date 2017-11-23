#pragma once

#define UINT_MAX_F 4294967295.0f

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
