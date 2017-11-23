
struct Particle
{
    vec4 position;
    vec4 velocity;
    float mass;
    float pad[3];
};

#define PARTICLE_BUFFER_BINDING 2
#define SPAWNER_BUFFER_BINDING 3
