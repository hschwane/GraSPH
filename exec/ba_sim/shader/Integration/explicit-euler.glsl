void integrate(inout Particle p, vec4 acc, float dt)
{
    p.position += p.velocity * dt;
    p.velocity += acc * dt;
}