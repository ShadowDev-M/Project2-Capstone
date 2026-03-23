#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec2 textureCoords;
layout (location = 2) in vec3 worldPos;
layout (location = 3) in vec3 localPos;
layout (location = 4) in vec3 localNormal;
layout(location = 5) in vec4 vFragPosLightSpace;  

uniform vec3 cameraPos;
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform uint lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;
uniform bool hasSpec;



uniform bool isTiled;
uniform vec3 uvTiling;
uniform vec2 tileScale;
uniform vec2 tileOffset;

uniform sampler2D shadowMap;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform samplerCube pointShadowMaps[MAX_LIGHTS]; // one cubemap per light

layout (location = 0) out vec4 fragColor;


uniform vec3 shadowLightDir; 
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
    float diskRadius = (1.0 + (viewDistance / 200)) / 25.0;  

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(pointShadowMaps[index], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= 200.0;   // undo mapping [0;1]
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
        return 0.0;

    vec3 N = normalize(vertNormal);
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
    }}

    shadow /= 25.0;

    float sunAngleFade = smoothstep(0.0, 0.0, NdotL); // makes shadows fade a bit but seems to also cause issues with no shadows on 90 degree angle surfaces so ill change it from 0.15 to 0 
    return 1.0 - (shadow * sunAngleFade);
}

void main() {
    vec3 normal = normalize(vertNormal);
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec4 kt;
    vec2 tiledTextureCoords;

    if (isTiled == true) {
        vec3 n = abs(localNormal);
        vec2 tiledTex;

		vec2 correctedScale = -tileScale / 100;

        vec2 finalOffset = tileOffset * correctedScale;

        vec3 scaledPos = localPos * uvTiling;

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
    
    vec4 phongResult = ambient * kt;
    float visibility = 1.0f;

    int pointLightCounter = 0;

    if (numLights > 0) {
        for(uint i = 0u; i < numLights; i++) {
            vec4 ks = (hasSpec == true) ? (isTiled == true) ? 
                        texture(specularTexture, tiledTextureCoords) * specular[i] : 
                        texture(specularTexture, textureCoords) * specular[i] : 
                        specular[i];
        
            vec4 kd = diffuse[i];
            
            vec3 lightWorldPos = lightPos[i]; 
            vec3 lightDir;
            
            if (lightType[i] == 0u) {  

                // this is directional
                lightDir = -lightWorldPos; // when a sky light is passed lightPos[i] becomes lightDir
            } else {  
                // point light
                lightDir = normalize(lightWorldPos - worldPos);
            }
            
            // Diffuse
            float diff = max(dot(normal, lightDir), 0.0f);
            
            // Specular
            vec3 reflection = reflect(-lightDir, normal);
            
            float spec = 0.0f;
            if (diff > 0.0f) {
                spec = pow(max(dot(viewDir, reflection), 0.0f), 16.0f);
            }
            
            vec4 lightContrib;
            if (lightType[i] == 0u) {
                lightContrib = ((diff * kd) + (spec * ks)) * kt * intensity[i];
            } else {
                float dist = length(lightWorldPos - worldPos);
                float attenuation = intensity[i] / (1.0f + 0.09f * dist + 0.032f * dist * dist);
                lightContrib = ((diff * kd) + (spec * ks)) * kt * attenuation;
            }
            
            
            float shadow = 1.0;

            if (lightType[i] == 0u) {
                shadow = ShadowCalculation() / float(MAX_LIGHTS);
            }
            if (lightType[i] == 1u && pointLightCounter < MAX_LIGHTS) {
                shadow = PointShadowCalculation(worldPos, lightWorldPos, pointLightCounter);
                pointLightCounter++;
            }

            phongResult += lightContrib * shadow;


        }


//        //SHADOW
//        vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w * 0.5 + 0.5;
//        float closestDepth = texture(shadowMap, projCoords.xy).r;
//        float currentDepth = projCoords.z;
//        float bias = 0.005;
//        shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    }
    

    //shadow = ShadowCalculation();
    fragColor = vec4(phongResult.rgb, phongResult.a);
}