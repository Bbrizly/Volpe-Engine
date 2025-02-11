#version 150

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;

in vec4 a_position;
in vec2 a_uv1;
in vec3 a_normal;

out vec2 v_uv1;
out vec3 v_normal;
out vec3 v_fragPosition;

void main()
{
    vec4 worldPos = world * a_position;
    gl_Position   = projection * view * worldPos;

    v_uv1         = a_uv1;
    // transform normal (approx for uniform scale)
    v_normal      = normalize(mat3(world) * a_normal);
    v_fragPosition= worldPos.xyz;
}

/*#version 150

// Matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;

// Vertex attributes
in vec4 a_position;
in vec2 a_uv1;
in vec3 a_normal;

// Pass to fragment shader
out vec2 v_uv1;
out vec3 v_normal;
out vec3 v_fragPosition;

void main()
{
    // Transform position into world space
    vec4 worldPosition = world * a_position;
    gl_Position = projection * view * worldPosition;

    // Pass UVs
    v_uv1 = a_uv1;

    // Transform the normal by the world matrix (approx)
    // For a perfect approach, use inverse-transpose of the upper-left 3x3
    v_normal = normalize(mat3(world) * a_normal);

    // We'll need the fragment's world-space position for lighting
    v_fragPosition = worldPosition.xyz;
}
*/