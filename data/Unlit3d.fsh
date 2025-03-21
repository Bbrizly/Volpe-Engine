in vec3 v_normal;
in vec2 v_uv1;

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;  // Toggle texture
uniform vec3 u_color;

void main()
{

//  if (useTexture) {baseColor *= texture(u_texture, vTexCoord);}

    FragColor = vec4(u_color, 1.0);
    //FragColor = vec4(v_normal, 1.0);
    //FragColor = vec4(v_normal * 0.5 + 0.5, 1.0);
    //FragColor = vec4(v_normal.x, 0.0, 0.0, 1.0);  // Red = X normal

    //FragColor = vec4(v_uv1, 0.0, 1.0); // UV mapped to Red (U) and Green (V)
}
