in vec4 v_color;
in vec2 v_uv1;
in vec3 v_normal;
in vec3 v_fragPosition;

out vec4 FragColor;

uniform sampler2DArray u_texture;
uniform bool useTexture;  // Toggle texture

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
    vec4 baseColor = v_color;
    vec3 totalLighting = vec3(0.3); // Base ambient lighting

    for(int i = 0; i < lightsInRange; i++){
        float attenuation = calculateAttenuation(pointLights[i].PositionRange.xyz, pointLights[i].PositionRange.w, v_fragPosition);
        vec3 lightDir = normalize(pointLights[i].PositionRange.xyz - v_fragPosition);
        float diff = max(dot(v_normal, lightDir), 0.0);
        totalLighting += attenuation * diff * pointLights[i].Color * pointLights[i].Strength;
    }

    FragColor = vec4(baseColor.rgb * totalLighting, baseColor.a * fade);

//  if (useTexture) {baseColor *= texture(u_texture, vTexCoord);}

    //FragColor = v_color;
    //FragColor = vec4(v_normal,1.0);
    //FragColor = vec4(v_fragPosition,1.0);
}


/*
in vec3 v_normal;
in vec4 v_color;
in vec2 v_uv1;
in vec3 v_fragPosition;

out vec4 FragColor;

uniform sampler2D texture1; // or sampler2D if you have a 2D texture
uniform float fade;         // optional alpha factor

const int MAX_LIGHTS = 8;

struct PointLight
{
    vec4 PositionRange; // .xyz => pos, .w => range
    vec3 Color;
    float Strength;
};

uniform int u_lightCount;
uniform PointLight u_pointLights[MAX_LIGHTS]; //MAX_LIGHTS //u_lightCount

float calcAttenuation(vec3 lightPos, float range, vec3 fragPos)
{
    float dist = length(lightPos - fragPos);
    if(dist > range) return 0.0;
    float atten = 1.0 - (dist / range);
    return atten * atten;
}

void main()
{
    // start with base ambient
    vec3 norm = normalize(v_normal);
    vec3 totalLight = vec3(0.2);
    // sum each valid light
    for(int i=0; i<2; i++) //u_lightCount; i++)
    {
        PointLight L = u_pointLights[i];

        vec3 lightPos = L.PositionRange.xyz;
        float range   = L.PositionRange.w;

        float att = calcAttenuation(lightPos, range, v_fragPosition);
        if(att <= 0.0) continue;

        vec3 lightDir = normalize(lightPos - v_fragPosition);
        float diff = max(dot(norm, lightDir), 0.0);

        totalLight += att * diff * L.Color * L.Strength;
    }

    PointLight L = u_pointLights[0];

    vec3 lightPos = L.PositionRange.xyz;
    float range   = L.PositionRange.w;

    float att = calcAttenuation(lightPos, range, v_fragPosition);
    if(att <= 0.0) continue;

    vec3 lightDir = normalize(lightPos - v_fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);

    totalLight += att * diff * L.Color * L.Strength;

    vec4 texColor = texture(texture1, v_uv1);
    vec3 finalColor = texColor.rgb * totalLight;

    FragColor = vec4(finalColor, texColor.a * fade);

    FragColor = vec4(v_color,1.0f);
}
*/