#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec2 textureCoords;
layout (location = 2) in vec3 worldPos;
layout (location = 3) in vec3 localPos;
layout (location = 4) in vec3 localNormal;

uniform vec3 cameraPos;

uniform vec3 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform uint lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;
uniform int hasSpec;

uniform bool isTiled;
uniform vec3 uvTiling;
uniform vec2 tileScale;
uniform vec2 tileOffset;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

layout (location = 0) out vec4 fragColor;



void main() {
    vec3 normal = normalize(vertNormal);
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec4 kt;

    if (isTiled == true){
        vec3 n = abs(localNormal);
        vec2 tiledTex;

        vec2 finalOffset = tileOffset * tileScale;

        vec3 scaledPos = localPos * uvTiling;

        if (n.y > n.x && n.y > n.z)
            tiledTex = scaledPos.xz * tileScale; // top  
        else if (n.x > n.z)
            tiledTex = scaledPos.zy * tileScale; // side
        else
            tiledTex = scaledPos.xy * tileScale; // front
            
        kt = texture(diffuseTexture, tiledTex + finalOffset);
       
    } else {
        kt = texture(diffuseTexture, textureCoords);
    }
    
    vec4 phongResult = ambient * kt;
    
    if (numLights > 0) {
        for(uint i = 0u; i < numLights; i++) {
            vec4 ks = (hasSpec == 1) ? texture(specularTexture, textureCoords) * specular[i] : specular[i];
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
            
            phongResult += lightContrib;
            //phongResult += vec4(lightDir * 0.5 + 0.5, 1.0);;
        }
    }
    fragColor = phongResult;
}