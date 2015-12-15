#version 150

uniform sampler2D texSampler;
in vec2 pass_textureCoord;
out vec4 FragColor;
uniform vec2 size;



void main() {
    
    vec4 FragColor = texture(texSampler, pass_textureCoord);
    
}


