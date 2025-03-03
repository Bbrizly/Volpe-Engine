uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform mat4 worldIT;

in vec3 a_position;
in vec3 a_uv1; // (u, v, layer)
in vec4 a_color;

out vec2 v_uv1;
out float v_layer;
out vec4 v_color;

void main()
{
    gl_Position = projection * view * world * vec4(a_position, 1.0);
    v_uv1 = vec2(a_uv1.x,a_uv1.y);
    v_layer = a_uv1.z;
    v_color = a_color;
}