#version 150

in vec2 v_uv1;
in vec3 v_normal;
in vec3 v_fragPosition;

out vec4 FragColor;

uniform sampler2D texture1; // or sampler2D if you have a 2D texture
uniform float fade;         // optional alpha factor

//--------------------------------------
// Suppose we define a max count
//--------------------------------------
const int MAX_LIGHTS = 8;

//--------------------------------------
// One light struct
//--------------------------------------
struct PointLight
{
    vec4 PositionRange; // .xyz => pos, .w => range
    vec3 Color;
    float Strength;
};

//--------------------------------------
// We'll have an array of lights plus a count
//--------------------------------------
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
    for(int i=0; i<u_lightCount; i++)
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

    vec4 texColor = texture(texture1, v_uv1);
    vec3 finalColor = texColor.rgb * totalLight;

    FragColor = vec4(finalColor, texColor.a * fade);
}



/*#version 150

in vec2 v_uv1;
in vec3 v_normal;
in vec3 v_fragPosition;

out vec4 FragColor;

// We'll sample a texture for the cube's base color
uniform sampler2D texture1;
uniform float fade; // can fade out the object if needed

//--------------------------------------------
// Our point light struct
//--------------------------------------------
struct PointLight
{
    vec4 PositionRange; // .xyz => position, .w => range
    vec3 Color;         // the light color
    float Strength;     // brightness scale
};

//--------------------------------------------
// We define an array of possible lights
// We'll pass in only the N that are in range
//--------------------------------------------
uniform int  u_lightCount;              // how many lights are valid
uniform PointLight u_pointLights[8];    // we can support up to 8

//--------------------------------------------
// Helper: attenuation based on distance
//--------------------------------------------
float calcAttenuation(vec3 lightPos, float lightRange, vec3 fragPos)
{
    float dist = length(lightPos - fragPos);
    if(dist > lightRange) 
        return 0.0;
    // linear fade
    float atten = 1.0 - (dist / lightRange);
    return atten * atten;
}

void main()
{
    // We'll do a basic ambient
    vec3 normal = normalize(v_normal);
    vec3 totalLight = vec3(0.2); // ambient

    //--------------------------------------------
    // Add up each valid light
    //--------------------------------------------
    for(int i = 0; i < u_lightCount; i++)
    {
        PointLight L = u_pointLights[i];
        vec3 lightPos = L.PositionRange.xyz;
        float range   = L.PositionRange.w;

        // attenuation
        float att = calcAttenuation(lightPos, range, v_fragPosition);
        if(att <= 0.0) 
            continue;

        // diffuse factor
        vec3 lightDir = normalize(lightPos - v_fragPosition);
        float diff = max(dot(normal, lightDir), 0.0);

        // accumulate
        totalLight += att * diff * L.Color * L.Strength;
    }

    // sample the base color from the texture
    vec4 texColor = texture(texture1, v_uv1);

    // multiply by total lighting
    vec3 finalColor = texColor.rgb * totalLight;

    // fade out alpha if desired
    FragColor = vec4(finalColor, texColor.a * fade);
}
*/