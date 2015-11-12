#version 150
#extension GL_ARB_explicit_attrib_location : enable


layout(location=0) in vec3 inputPosition;
//attribute vec2 inputTexCoord;
layout(location=1)in vec3 inputNormal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec3 normalInterp;
out vec3 vertPos;


void main(){
    
    gl_Position = ProjectionMatrix * ModelMatrix * ViewMatrix * vec4(inputPosition, 1.0);
    vec4 vertPos4 = ModelMatrix * ViewMatrix * vec4(inputPosition, 1.0);
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = vec3(NormalMatrix * vec4(inputNormal, 0.0));
    
  
}