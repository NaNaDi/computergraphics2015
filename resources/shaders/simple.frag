#version 150

uniform sampler2D texSampler;
in vec2 pass_textureCoord;
out vec4 FragColor;
uniform vec2 size;

uniform int greyscale = 0;
uniform int horizMirror = 0;
uniform int verticMirror = 0;
uniform int gausBlur = 0;
uniform int clearAll = 0;

uniform float offset_a[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float offset_b[5] = float[]( 4.0, 3.0, 2.0, 1.0, 0.0 );
uniform float weight_a[4] = float[]( 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
uniform float weight_b[4] = float[]( 0.0162162162, 0.0540540541, 0.1216216216, 0.1945945946 );

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

vec4 gaussian_blur (vec4 inColor, vec2 textureCoord)
{
    inColor = texture(texSampler, vec2(textureCoord.x, textureCoord.y)) * 0.2270270270;
    for (int i=0; i<4; i++) {
        inColor += texture(texSampler, (vec2(textureCoord.x-(offset_a[i]/600), textureCoord.y-(offset_a[i]/600))))* weight_a[i];
        inColor += texture(texSampler, (vec2(textureCoord.x+(offset_b[i]/600), textureCoord.y+(offset_b[i]/600))))* weight_b[i];
    }
    return inColor;
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
    
    if(gausBlur == 1)
    {
        inColor = gaussian_blur(inColor, textureCoord);
    }
    
        //FragColor = inColor;
    
    if (greyscale == 1)
    {
        inColor = greyscaleMultiply(inColor);
    }
    
        FragColor = inColor;
    
//    if (clearAll == 1)
//    {
//        textureCoord = pass_textureCoord;
//        FragColor =  texture(texSampler, textureCoord);
//    }
    
}




