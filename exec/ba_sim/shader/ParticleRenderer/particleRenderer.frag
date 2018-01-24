#version 450

in vec2 center;
in float radius;

uniform vec4 color;
uniform float brightness;

out vec4 fragment_color;
flat in int isSink;

void main()
{
    vec2 coord = (gl_FragCoord.xy - center) / radius;
    float distFromCenter = length(coord);

#ifdef PARTICLES_ROUND
    // make it round
    if(distFromCenter > 1.0)
        discard;
#endif

//    if(isSink>0)
//    {
//        fragment_color = vec4(0,0,50,1);
//        return;
//    }

    vec4 falloffColor;
    PARTICLE_FALLOFF(); // this is defined via preprocessor macros when compiling
	fragment_color = falloffColor*brightness;
}