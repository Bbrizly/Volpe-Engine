uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform mat4 worldIT;

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv1;

out vec3 v_normal;
out vec2 v_uv1;
out vec3 v_fragPosition;

void main()
{
    gl_Position = projection * view * world * vec4(a_position, 1.0);

    // v_normal = normalize(mat3(world) * a_normal);
    v_normal = normalize(mat3(worldIT) * a_normal);

    v_uv1 = a_uv1;
    
    vec4 worldPos = world * vec4(a_position, 1.0);
    v_fragPosition = worldPos.xyz;
}
