// computes the acceleration of a particle i resulting from
// the interaction with another particle j
// using a smoothed gravity model found in gpuGmes 3
// "Plummer point masses" normally used for interactions between galaxys
vec4 interaction(vec4 iPos, vec4 jPos, float jMass, float ES2)
{
    vec4 rij = jPos - iPos;
    float inv_dist_3_2 = 1.0f / sqrt(pow(dot(rij,rij) + ES2,3));
    return jMass * rij * inv_dist_3_2;
}