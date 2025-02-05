#version 330 core

in vec4 v_color;
in vec2 v_uv1;
in vec4 v_normal; // not used in final, but passed

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;  // Toggle texture

void main()
{
    // If you want the texture:
    // vec4 texColor = texture(u_texture, vec3(v_uv1, 0.0));
    // FragColor = v_color * texColor;

    FragColor = v_color;
}
