
#include "common.frag"
#line 4

layout (location = 0) out vec4 FragColor;

uniform pass_t buf;

void main()
{
	float brightPassThreshold = 1.0f;

	vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
	vec3 c = pass_sample(buf, texcoord).rgb;

    float luminance = dot(luminanceVector, c);
    luminance = max(0.0, luminance - brightPassThreshold);
    c *= sign(luminance);

    FragColor = vec4(c, 1.0f);
}

// vim: set ft=c:
