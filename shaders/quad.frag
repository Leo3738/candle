
#include "common.frag"

layout (location = 0) out vec4 FragColor;

void main()
{
	/* FragColor = ssr(diffuse.texture); */
	FragColor = texture2D(diffuse.texture, texcoord);
	/* FragColor = vec4(texcoord, 0.0f, 1.0f); */
}  

// vim: set ft=c:
