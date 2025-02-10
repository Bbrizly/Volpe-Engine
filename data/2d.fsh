in vec4 v_color;
in vec2 v_uv1;
in float v_layer;

out vec4 FragColor;

uniform sampler2DArray u_texture;

void main()
{
    if(v_layer < 0)
    {
        FragColor = v_color;
        return;
    }
    //from pixel shader Documenttation
    //vec4 pixel = texture(u_texture, v_uv1);
    vec4 pixel = texture(u_texture, vec3(v_uv1, v_layer));

    vec4 v_chnl = vec4(1,0,0,0);

    if (dot(vec4(1.0), v_chnl) > 0.0) {
        float val = dot(pixel, v_chnl);

        pixel.rgb = (val > 0.5) ? vec3(2.0 * val - 1.0) : vec3(0.0);
        pixel.a = (val > 0.5) ? 1.0 : 2.0 * val;
    }

    FragColor = pixel * v_color;
}