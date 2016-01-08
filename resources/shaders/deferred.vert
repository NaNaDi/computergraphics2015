#version 150

layout(location=0) in vec3 inputPosition;
layout(location=1)in vec3 inputNormal;
layout(location=2) in vec2 in_textureCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec2 textureCoord;
out vec3 normal;
out vec3 worldPos;

void main()
{
    gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(inputPosition, 1.0);
    textureCoord = in_textureCoord;
    normal = vec3(NormalMatrix * vec4(inputNormal, 0.0)).xyz;
    WorldPos0 = (NormalMatrix * vec4(inputPosition, 1.0)).xyz;
}