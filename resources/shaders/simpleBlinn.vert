#version 150
#extension GL_ARB_explicit_attrib_location : require


layout(location=0) in vec3 inputPosition;
layout(location=1)in vec3 inputNormal;
layout(location=2) in vec2 in_textureCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec3 normalInterp;
out vec3 vertPos;

//out vec3 LightDirection;

out vec2 pass_textureCoord;

void main(){
    
    gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(inputPosition, 1.0);
    vec4 vertPos4 = ViewMatrix * ModelMatrix * vec4(inputPosition, 1.0);
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = vec3(NormalMatrix * vec4(inputNormal, 0.0));
    
    //    LightDirection = vec3(0.0f,0.0f,0.0f) ;
    pass_textureCoord = in_textureCoord;
    
    
}