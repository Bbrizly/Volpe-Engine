#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform mat4 worldIT;

uniform vec3 uLightDir;       
uniform vec3 uLightColor;     
uniform float uLightIntensity;

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
    vec4 worldPos = world * vec4(a_position, 1.0);
    gl_Position   = projection * view * worldPos;

    vec3 normalWS = normalize((worldIT * vec4(a_normal, 0.0)).xyz);

    vec3 L = normalize(-uLightDir);
    float NdotL = max(dot(normalWS, L), 0.0);

    vec3 diffuse = NdotL * uLightColor * uLightIntensity;

    vec3 baseColor = a_color.rgb * diffuse;
    float alpha    = a_color.a;

    v_color = vec4(baseColor, alpha);

    v_uv1    = a_uv1;
    v_normal = vec4(normalWS, 0.0);
    // v_pos  = worldPos.xyz; 
}
