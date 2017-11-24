#version 450

in vec2 center;
in float radius;

out vec4 fragment_color;

void main()
{
    vec2 coord = (gl_FragCoord.xy - center) / radius;
    float distFromCenter = length(coord);

    // make it round
    if(distFromCenter > 1.0)
        discard;

    //float b = 1-distFromCenter;
	fragment_color = vec4(0.9,0.3,0.1,1);
}