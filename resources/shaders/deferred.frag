#version 150

in vec2 textureCoord;
in vec3 normal;
in vec3 worldPos;

layout (location = 0) out vec3 outputPosition;
layout (location = 1) out vec3 diffuseOut;
layout (location = 2) out vec3 normalOut;
layout (location = 3) out vec3 pass_textureCoord;

uniform sampler2D texSampler;

void main()
{
    outputPosition = worldPos;
    diffuseOut = texture(texSampler, textureCoord).xyz;
    normalOut = normalize(normal);
    pass_textureCoord = vec3(textureCoord, 0.0);
}