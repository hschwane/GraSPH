// computes the acceleration of a particle i resulting from
// the interaction with another particle j
// using a smoothed gravity model found in gpuGmes 3
// "Plummer point masses" normally used for interactions between galaxys
// the particles mass is stored in the fourth element of the position
vec3 interaction(vec4 iPos, vec4 jPos, float ES2)
{
    vec3 rij = jPos.xyz - iPos.xyz;
    float inv_dist_3_2 = 1.0f / sqrt(pow(dot(rij,rij) + ES2,3));
    return jPos.w * rij * inv_dist_3_2;
}