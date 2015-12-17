#version 150

uniform sampler2D texSampler;
in vec2 pass_textureCoord;
out vec4 FragColor;
uniform vec2 size;

uniform int greyscale = 0;
uniform int horizMirror = 0;
uniform int verticMirror = 0;

vec4 greyscaleMultiply (vec4 inColor)
{
    return vec4(vec3(dot(vec3(0.2126f, 0.7152f, 0.0722f), inColor.rgb)), inColor.a);
}

vec2 horizontal_mirror (vec2 coordinates)
{
    return vec2(1.0f-coordinates.x, coordinates.y);
}

vec2 vertical_mirror (vec2 coordinates)
{
    return vec2(coordinates.x, 1.0f-coordinates.y);
}

void main() {
    
    vec2 textureCoord = pass_textureCoord;
    
    if (horizMirror == 1)
    {
        textureCoord = horizontal_mirror(textureCoord);
    }
    
    if (verticMirror == 1)
    {
        textureCoord = vertical_mirror(textureCoord);
    }
    
    vec4 inColor = texture(texSampler, textureCoord);
    
    if (greyscale == 1)
    {
        inColor = greyscaleMultiply(inColor);
    }
    FragColor = inColor;
    
}




