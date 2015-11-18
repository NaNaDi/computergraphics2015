#version 150

precision mediump float;


in vec3 normalInterp;
in vec3 vertPos;


out vec4 FragColor;

//const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 ambientColor = vec3(0.1, 0.1, 0.2);
//const vec3 diffuseColor = vec3(0.5, 0.0, 0.5);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

uniform vec3 shaderColor;
uniform vec3 lightPos;

void main() {
    
    vec3 normal = normalize(normalInterp);
    vec3 lightDir = normalize(lightPos - vertPos);
    
    float lambertian = max(dot(lightDir,normal), 0.0);
    float specular = 0.0;
    
    if(lambertian > 0.0) {
        
        vec3 viewDir = normalize(-vertPos);
        
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        specular = pow(specAngle, 16.0);
    }
    
    
    //exchanged shaderColor and diffuseColor
    FragColor = vec4(ambientColor +
                        lambertian * shaderColor +
                        specular * specColor, 1.0);
    
}
