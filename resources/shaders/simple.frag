#version 150

in  vec4 pass_Normal;
out vec4 out_Color;

void main(void)
{
    out_Color = vec4(abs(pass_Normal.xyz), 1.0f);
}
