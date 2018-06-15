#version 450

in vec2 center;
in float radius;

uniform float brightness;

out vec4 fragment_color;
flat in vec4 particle_color;

void main()
{
    vec2 coord = (gl_FragCoord.xy - center) / radius;
    float distFromCenter = length(coord);

#ifdef PARTICLES_ROUND
    // make it round
    if(distFromCenter > 1.0)
        discard;
#endif

    vec4 color = particle_color;

    vec4 falloffColor;
    PARTICLE_FALLOFF(); // this is defined via preprocessor macros when compiling
	fragment_color = falloffColor*brightness;
}