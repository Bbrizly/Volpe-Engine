uniform vec3 u_color;

in vec4 v_normal;
in vec2 v_uv1;
in vec3 v_pos;
out vec4 PixelColor;

void main()
{
	PixelColor = vec4(u_color,1.0);
}
