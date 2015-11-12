#version 150

precision mediump float;

in vec3 normalInterp;
in vec3 vertPos;

out vec4 FragColor;

//uniform int mode;

const vec3 lightPos = vec3(0.0,0.0,0.0);
const vec3 ambientColor = vec3(0.1, 0.1, 0.2);
const vec3 diffuseColor = vec3(0.5, 0.0, 0.5);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

void main() {
    
    vec3 normal = normalize(normalInterp);
    vec3 lightDir = normalize(lightPos - vertPos);
    
    float lambertian = max(dot(lightDir,normal), 0.0);
    float specular = 0.0;
    
    if(lambertian > 0.0) {
        
        vec3 viewDir = normalize(-vertPos);
        
        // this is blinn phong
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        specular = pow(specAngle, 16.0);
    }
    
    FragColor = vec4(ambientColor +
                        lambertian * diffuseColor +
                        specular * specColor, 1.0);
//      FragColor = vec4(1.0f,0.64f,0.37f, 1.0f);
}
