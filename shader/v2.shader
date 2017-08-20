#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec3 normalInfo;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform vec3 lightPos;
uniform mat4 normMat;
uniform vec3 viewPos;

uniform vec3 fragColor;

out float normalWeight;
out float specWeight;
out vec3 fColor;

void main() {
	gl_Position = MVP * vec4(vertexPosition, 1.0);
	vec3 wPos = vec3(modelMat * vec4(vertexPosition, 1.0));

	// 对法向量进行变化，变化到世界坐标系

	vec3 norm = normalize(mat3(normMat) * normalInfo);
	vec3 inVec = normalize(lightPos - wPos);

	vec3 viewDir = normalize(viewPos - wPos);
	vec3 reflectDir = normalize(reflect(-inVec, norm));

	normalWeight = max(dot(norm, inVec), 0.0);

	specWeight = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 表示高光的反光度

	fColor = fragColor;
}