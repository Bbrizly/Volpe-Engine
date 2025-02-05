uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform mat4 worldIT;

in vec3 a_position;
in vec4 a_color;
in vec2 a_uv1;
in vec3 a_normal;

out vec4 v_normal;
// out vec3 v_pos;
out vec4 v_color;
out vec2 v_uv1;

void main()
{
    gl_Position = projection * view * world * vec4(a_position, 1.0);
    // gl_Position = projection * view * world * vec4(a_position,1.0f);
    v_color = a_color;
    v_uv1 = a_uv1;
	// v_pos = (world * a_position).xyz;
    v_normal = worldIT * vec4(a_normal, 0.0);
}