in vec3 v_normal;
in vec2 v_uv1;
in vec3 v_fragPosition;

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;
uniform vec3 u_color;
uniform float fade;

struct PointLight {
    vec4 PositionRange;  // .xyz = position in world space, .w = range
    vec3 Color;
    float Strength;
};

const int maximumLights = 5;
uniform int lightsInRange;
uniform PointLight pointLights[maximumLights];

float calculateAttenuation(vec3 lightPos, float lightRange, vec3 fragPos)
{
    float distance = length(lightPos - fragPos);
    if (distance > lightRange)
        return 0.0;

    float ratio = 1.0 - (distance / lightRange);
    return ratio * ratio;
}

void main()
{
    vec4 baseColor = vec4(u_color, 1.0);
    vec3 totalLighting = vec3(0.3);

    for (int i = 0; i <= lightsInRange; i++) { //lightsInRange

        vec3 lightPos = pointLights[i].PositionRange.xyz;
        float lightRange = pointLights[i].PositionRange.w;

        float attenuation = calculateAttenuation(lightPos, lightRange, v_fragPosition);
        vec3 lightDir = normalize(lightPos - v_fragPosition);

        float diff = max(dot(v_normal, lightDir), 0.0);

        // Accumulate contribution
        totalLighting += attenuation * diff * pointLights[i].Color * pointLights[i].Strength;// pointLights[i].Color * pointLights[i].Strength;

    }

    FragColor = vec4(baseColor.rgb * totalLighting, baseColor.a * fade);
    //FragColor = vec4(totalLighting, 1.0);

    
    //float d = length(pointLights[0].PositionRange.xyz - v_fragPosition);
    //float distNorm = clamp(d / 20.0, 0.0, 1.0);
    //FragColor = vec4(distNorm, distNorm, distNorm, 1.0);
}
