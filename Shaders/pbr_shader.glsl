
#version 330

#ifdef VERTEX_SHADER
uniform mat4 mvpMatrix;
uniform mat4 trsMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out vec2 vtexcoord;
out vec3 worldPos;
out vec3 worldNormal;

void main( )
{
	gl_Position = mvpMatrix * vec4(position, 1);
	vtexcoord = texcoord;
	worldPos = (trsMatrix * vec4(position, 1)).xyz;
	worldNormal = (trsMatrix * vec4(normal, 0)).xyz;
}
#endif




#ifdef FRAGMENT_SHADER
uniform vec4 color;

uniform sampler2D albedoTex;
uniform sampler2D roughTex;
uniform sampler2D metalTex;

in vec2 vtexcoord;
in vec3 worldPos;
in vec3 worldNormal;

void main()
{
	vec4 diffuseColor = color * texture(albedoTex, vtexcoord);
	float roughness = texture(roughTex, vtexcoord).r;
	float metalness = texture(metalTex, vtexcoord).r;

	gl_FragData[0] = vec4(diffuseColor.rgb, roughness);
	gl_FragData[1] = vec4(worldNormal.xyz, metalness);
}
#endif
