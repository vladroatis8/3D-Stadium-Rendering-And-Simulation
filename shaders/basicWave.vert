#version 410 core

layout(location=0) in vec2 vTexCoords;
layout(location=1) in vec2 gridXY;

out vec3 fPosition;
out vec2 fTexCoords;
out vec3 fNormal;

uniform mat4 apamodel;
uniform mat4 apaview;
uniform mat4 apaprojection;
uniform mat3 apanormalMatrix;

uniform float time;
uniform vec2 gridDimensions;
uniform vec2 gridSize;

void main() 
{
	fTexCoords = vTexCoords;

	// compute uv coordinates
	float u = gridXY.x / (gridSize.x - 1);
	float v = gridXY.y / (gridSize.y - 1);

	u = u * 2.0f - 1.0f;
	v = v * 2.0f - 1.0f;

	// compute simple sine wave pattern y coordinate
	float freq = 7.0f;
	float ampl = 0.01f;

	//compute position
	//evaluate the function of the sine surface (f(u,v) = x * (1, 0, 0) + y * (0, 1, 0) + z * (0, 0, 1))
	float x = - (gridDimensions.x / 2.0f) + (u + 1.0f) / 2.0f * gridDimensions.x;
	float y = sin(u * freq + time) * cos(v * freq + time) * ampl;
	float z = - (gridDimensions.y / 2.0f) + (v + 1.0f) / 2.0f * gridDimensions.y;

	//compute tangent (partial derivative of f over u)
	vec3 tangent = vec3(gridDimensions.x / 2.0f, ampl * freq * cos(v * freq + time) * cos(u * freq + time), 0.0f);

	//compute bi-tangent (partial derivative of f over v)
	vec3 bitangent = vec3(0.0f, - ampl * freq * sin(u * freq + time) * sin(v * freq + time), gridDimensions.y / 2.0f);

	//compute normal
	fNormal = normalize(apanormalMatrix * cross(bitangent, tangent));

	//compute position in eye space
	fPosition = vec3(apaview * apamodel * vec4(x, y, z, 1.0f));

	gl_Position =apaprojection * apaview * apamodel * vec4(x, y, z, 1.0f);
}
