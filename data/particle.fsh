in vec2 v_uv1;
in float v_layer;
in vec4 v_color;

out vec4 FragColor;

//uniform sampler2DArray u_texture;
uniform sampler2D u_texture;
uniform bool useTexture;  // Toggle texture
uniform vec3 u_color;

float discardAlpha = 0.1f;
//vec4

void main()
{
  vec4 baseColor = v_color;
  
  //if(useTexture) {baseColor *= texture(u_texture, vec3(v_uv1.x, v_uv1.y, v_layer));}
  
  if(useTexture) {
    baseColor *= texture(u_texture, v_uv1.xy);
  }

//  if(baseColor.a <= discardAlpha)
//    discard;

  FragColor = baseColor;

  //FragColor = vec4(v_uv1, 0.0, 1.0); // UV mapped to Red (U) and Green (V)
}
