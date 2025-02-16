//in vec4 v_color;
in vec2 v_uv1;
in vec4 v_normal;

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;  // Toggle texture
uniform vec3 u_color;

void main()
{

//  if (useTexture) {baseColor *= texture(u_texture, vTexCoord);}

    FragColor = vec4(u_color, 1.0);
}
