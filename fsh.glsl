#version 330

in vec4 vertexColor;
in vec2 texCoord;
in vec3 vertexNormal;

out vec4 fragColor;

uniform sampler2D teximg;

vec4 directionalLight()
{
	vec4 lightColor = vec4(1.0f, 1.0f, 0.98f, 1.0f);

	// ambient
	float ambient = 0.2f;

	vec3 normal = normalize(vertexNormal);
	vec3 lightDirection = normalize(vec3(0.72857, 0.594765, 0.22457));
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	return texture(teximg, texCoord) * ((diffuse + ambient) * lightColor);
}

void main()
{
	fragColor = directionalLight();
}
