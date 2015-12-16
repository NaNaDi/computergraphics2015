#version 150

uniform sampler2D texSampler;
in vec2 pass_textureCoord;
out vec4 FragColor;
uniform vec2 size;

uniform int greyscale = 0;

vec4 greyscaleMultiply (vec4 inColor)
{
    return vec4(vec3(inColor.r * 0.2126f + inColor.g * 0.7152f + inColor.b * 0.0722f), inColor.a);
}

void main() {
    vec4 inColor = texture(texSampler, pass_textureCoord);
     if (greyscale == 1)
    {
        
        inColor = greyscaleMultiply(inColor);

    }
    
     else {
         FragColor = inColor;
     }
    
}




