in vec4 v_color;
in vec2 v_uv1;

out vec4 FragColor;

uniform sampler2DArray u_texture;

void main()
{
    FragColor = v_color;
}
