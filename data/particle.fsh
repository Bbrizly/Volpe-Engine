in vec2 v_uv1;
in float v_layer;
in vec4 v_color;

out vec4 FragColor;

//uniform sampler2DArray u_texture;
uniform sampler2D u_texture;
uniform bool useTexture;  // Toggle texture
uniform vec3 u_color;
uniform float u_glowIntensity;

float discardAlpha = 0.1f;
//vec4

void main()
{
  vec4 baseColor = v_color;
  
  //if(useTexture) {baseColor *= texture(u_texture, vec3(v_uv1.x, v_uv1.y, v_layer));}
  
  if(useTexture) {
    baseColor *= texture(u_texture, v_uv1.xy);
    baseColor = vec4(baseColor.rgb * baseColor.a, baseColor.a); //premultiplied shits
  }
  if(u_glowIntensity != 0.0f) 
  {
    baseColor.rgb *= u_glowIntensity;
  }

//  if(baseColor.a <= discardAlpha)
//    discard;

  FragColor = baseColor;

  //FragColor = vec4(v_uv1, 0.0, 1.0); // UV mapped to Red (U) and Green (V)
}
