#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 8
#define MAX_SHADOW_CASTERS 4 // FBO locked
#define PI 3.14159265358979323846

layout (location = 0) in mat3 TBN;
layout (location = 3) in vec2 textureCoords;
layout (location = 4) in vec3 worldPos;
layout (location = 5) in vec3 localPos;
layout (location = 6) in vec3 localNormal;
layout (location = 7) in vec4 vFragPosLightSpace;

uniform vec3 cameraPos;
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform bool lightType[MAX_LIGHTS];
uniform int lightCastsShadow[MAX_LIGHTS];
uniform int numPointShadow[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;

uniform bool hasSpec;
uniform bool hasNorm;
uniform bool hasRough;
uniform bool hasMetal;
uniform bool isPBR;
uniform bool isTiled;
uniform vec3 uvTiling;
uniform vec2 tileScale;
uniform vec2 tileOffset;

uniform sampler2D shadowMap;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D metallicTexture;
uniform samplerCube pointShadowMaps[MAX_SHADOW_CASTERS]; // one cubemap per light

uniform vec3 shadowLightDir; 
uniform float pointShadowFarPlanes[MAX_SHADOW_CASTERS];

layout (location = 0) out vec4 fragColor;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float PointShadowCalculation(vec3 fragPos, vec3 lightWorldPos, int index) {
       
   //https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
    vec3 fragToLight = fragPos - lightWorldPos; 
    float currentDepth = length(fragToLight);  
    float viewDistance = length(cameraPos - fragPos);
    float shadow = 0.0;
    float bias   = 0.15;
    int samples  = 20;
    float diskRadius = (1.0 + (viewDistance / pointShadowFarPlanes[index])) / 25.0;  

    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(pointShadowMaps[index], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= pointShadowFarPlanes[index];   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);  
    return 1 - shadow;
}

float ShadowCalculation() {
    vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w;
    vec2 shadowUV = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z * 0.5 + 0.5;

    if(abs(vFragPosLightSpace.x) > vFragPosLightSpace.w ||
       abs(vFragPosLightSpace.z) > vFragPosLightSpace.w)
        return 1.0;
    if(vFragPosLightSpace.y > vFragPosLightSpace.w)
        return 1.0;
    if(vFragPosLightSpace.y < -vFragPosLightSpace.w)
        return 1.0;

    vec3 N = normalize(TBN[2]);
    vec3 L = normalize(-shadowLightDir);
    float NdotL = dot(N, L);

    if(NdotL < 0.0)
        return 0.0;

float bias = mix(0.005, 0.0005, NdotL * NdotL); 

float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            vec2 sampleUV = clamp(shadowUV + vec2(x, y) * texelSize, 0.001, 0.999);
            float closestDepth = texture(shadowMap, sampleUV).r;
            shadow += (currentDepth > closestDepth + bias) ? 1.0 : 0.0;
        }
    }

    shadow /= 25.0;

    float sunAngleFade = smoothstep(0.0, 0.0, NdotL); // makes shadows fade a bit but seems to also cause issues with no shadows on 90 degree angle surfaces so ill change it from 0.15 to 0 
    return 1.0 - (shadow * sunAngleFade);
}

vec4 CalcNonPBRLighting(vec3 PosDir, vec3 normal, vec4 tex, vec4 diff, vec4 spec, float str, bool type) {
    vec4 result;

    // light Pos/Dir vector l + Attenuation Calc
    float LightIntensity = str;

    vec3 l;
    if (type == false) {    // if the light is a directional light
        l = -PosDir;
    } else {                // else point light
        l = PosDir - worldPos;
        float dist = length(l);
        l = normalize(l);
        LightIntensity /= dist * dist;
    }
    
    // Light Diffuse segment
    float LightDiffuse = max(dot(normal, l), 0.0f);

    // Light Specular segment
    vec3 reflection = reflect(-l, normal);
    vec3 v = normalize(cameraPos - worldPos);
    float LightSpecular = (LightDiffuse > 0.0) ? pow(max(dot(v, reflection), 0.0), 16.0) : 0.0;


    result = ((LightDiffuse * diff) + (LightSpecular * spec)) * tex * LightIntensity;
    return result;
}

vec4 CalcPBRLighting(vec3 PosDir, vec3 normal, vec4 tex, float roughness, float metallic, vec4 diff, float str, bool type){
vec4 result;

    // light Pos/Dir vector l + Attenuation Calc
    vec3 lightIntensity = (diff * str).xyz;
    //vec3 specIntensity = (spec * str).xyz;

    vec3 l;
    if (type == false) {    // if the light is a directional light
        l = normalize(-PosDir);
    } else {                // else point light
        l = PosDir - worldPos;
        float dist = length(l);
        l = normalize(l);
        lightIntensity /= dist * dist;
        //specIntensity /= dist * dist;
    }

    vec3 n = normal;
    vec3 v = normalize(cameraPos - worldPos);
    vec3 h = normalize(v + l);

    float NDotH = max(dot(n, h), 0.0);
    float VDotH = max(dot(v, h), 0.0);
    float NDotL = max(dot(n, l), 0.0);
    float NDotV = max(dot(n, v), 0.0);

    // SpecularBRDF = D*G*F / 4*dot(normal, light) * dot(normal, view)

    // building the D term (GGX Distribution)
    float alpha2 = roughness * roughness * roughness * roughness;
    float denom = (NDotH * NDotH * (alpha2 - 1.0) + 1.0);
    float D = alpha2 / (PI * denom * denom);

    // building the G term (Schlick GGX)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float GL = NDotL / max(NDotL * (1.0 - k) + k, 0.001);
    float GV = NDotV / max(NDotV * (1.0 - k) + k, 0.001);
    float G  = GL * GV;

    // building the F term (Schlick Approximation)
    vec3 albedo = pow(tex.rgb, vec3(2.2));
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - VDotH, 0.0, 1.0), 5);

    // Assemble SpecularBRDF
    vec3 SpecularBRDF = D * F * G / (4 * NDotL * NDotV + 0.001);

    // Get kd and Assemble DiffuseBRDF
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 DiffuseBRDF = kd * albedo / PI;

    result = vec4((DiffuseBRDF + SpecularBRDF) * lightIntensity * NDotL, tex.a);

    return result;
}

void main() {
    vec4 kt;
    vec2 tiledTextureCoords;

    // Tiling
    if (isTiled) {
        vec3 n = abs(localNormal);
		vec2 correctedScale = -tileScale / 100;
        vec2 finalOffset = tileOffset * correctedScale;
        vec3 scaledPos = localPos * uvTiling;

        vec2 tiledTex;
        if (n.y > n.x && n.y > n.z)
            tiledTex = scaledPos.xz * correctedScale; // top  
        else if (n.x > n.z)
            tiledTex = scaledPos.zy * correctedScale; // side
        else
            tiledTex = scaledPos.xy * correctedScale; // front
            
		tiledTextureCoords = tiledTex + finalOffset;
		if (tiledTextureCoords.x == 0) {
			tiledTextureCoords.x = textureCoords.x;
		} 
		if (tiledTextureCoords.y == 0) {
			tiledTextureCoords.y = textureCoords.y;
		}

        kt = texture(diffuseTexture, tiledTextureCoords);
	} else {
		kt = texture(diffuseTexture, textureCoords); 
	}
    
     // normal mapping
    vec3 normal; 
    if (hasNorm) {
        vec2 normUV = isTiled ? tiledTextureCoords : textureCoords;
        normal = normalize(TBN * (2.0 * texture(normalTexture, normUV).xyz - 1.0));
    }
    else {
        normal = normalize(TBN[2]);
    }

    // ambient
    vec4 result = ambient * kt;

    // light calculation
    if (numLights > 0u) {
        for(uint i = 0u; i < numLights; i++) {
            if (i >= uint(MAX_LIGHTS)) break;

            vec4 kd = diffuse[i];

            vec4 light;
            if (isPBR == true) {
                vec2 pbrUV = isTiled ? tiledTextureCoords : textureCoords;
                float roughness = hasRough ? texture(roughnessTexture, pbrUV).r : 0.5;
                float metallic = hasMetal ? texture(metallicTexture, pbrUV).r : 0.0;
                light = CalcPBRLighting(lightPos[i], normal, kt, roughness, metallic, kd, intensity[i], lightType[i]);
            }
            else {
                vec2 specUV = isTiled ? tiledTextureCoords : textureCoords;
                vec4 ks = hasSpec ? texture(specularTexture, specUV) * specular[i] : specular[i];
                light = CalcNonPBRLighting(lightPos[i], normal, kt, kd, ks, intensity[i], lightType[i]);
            }

            float shadow = 1.0;

            if (lightCastsShadow[i] == 1) {
                if (lightType[i] == false) {
                    shadow = ShadowCalculation();
                }
                else if (lightType[i] == true) {
                    int num = numPointShadow[i];
                    if (num >= 0 && num < MAX_SHADOW_CASTERS) {
                        shadow = PointShadowCalculation(worldPos, lightPos[i], num);
                    }
                }
            }
            
            result += light * shadow;
        }
    }

    float exposure = 1.5f;
    vec3 toneMapped = vec3(1.0f) - exp(-result.xyz * exposure);
    fragColor = vec4(pow(toneMapped, vec3(1.0/2.2)), result.a);
}