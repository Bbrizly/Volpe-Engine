in vec2 v_uv1;
in vec3 v_normal;
in vec3 v_fragPosition;

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;  // Toggle texture

uniform vec3 u_color;

uniform float fade;

struct PointLight{
    vec4 PositionRange; // .xyz is position, .w is range
    vec3 Color;
    float Strength;
};

const int maximumLights = 5;
uniform int lightsInRange;
uniform PointLight pointLights[maximumLights];

float calculateAttenuation(vec3 lightPos, float lightRange, vec3 fragPos){
    float distance = length(lightPos - fragPos);
    if (distance > lightRange)
        return 0.0; // No contribution if outside the light's range
    // Linear attenuation
    float attenuation = 1.0 - (distance / lightRange);
    return attenuation * attenuation; // Smooth shits
}

void main()
{
    vec4 baseColor = vec4(u_color,1.0);
    vec3 totalLighting = vec3(0.3); // Base ambient lighting

    for(int i = 0; i < lightsInRange; i++){
        float attenuation = calculateAttenuation(pointLights[i].PositionRange.xyz, pointLights[i].PositionRange.w, v_fragPosition);
        vec3 lightDir = normalize(pointLights[i].PositionRange.xyz - v_fragPosition);
        float diff = max(dot(v_normal, lightDir), 0.0);
        totalLighting += attenuation * diff * pointLights[i].Color * pointLights[i].Strength;
    }

    FragColor = vec4(baseColor.rgb * totalLighting, baseColor.a * fade);

//  if (useTexture) {baseColor *= texture(u_texture, vTexCoord);}

    //FragColor = vec4(u_color, 1.0);
    //FragColor = vec4(v_normal,1.0);
    //FragColor = vec4(v_fragPosition,1.0);
}