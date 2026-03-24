#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

uniform bool isTiled;
uniform vec3 uvTiling;
uniform vec2 tileScale;
uniform vec2 tileOffset;
uniform vec3 cameraPos;

layout (location = 0) in mat3 TBN;
layout (location = 3) in vec2 textureCoords;
layout (location = 4) in vec3 worldPos;
layout (location = 5) in vec3 localPos;
layout (location = 6) in vec3 localNormal;
layout (location = 7) in vec4 vFragPosLightSpace;


uniform sampler2D shadowMap;
uniform sampler2D myTexture; 

void main() {
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec4 ks = vec4(1.0, 0.0, 0.0, 0.0);
	vec4 kd = vec4(0.75, 0.6, 0.6, 0.0);
	vec4 kt;
	vec4 ka = 0.1 * kd;

	vec3 vertNormal = TBN[2];

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
            
		vec2 tiledTextureCoords = tiledTex + finalOffset;
		if (tiledTextureCoords.x == 0) {
			tiledTextureCoords.x = textureCoords.x;
		} 
		if (tiledTextureCoords.y == 0) {
			tiledTextureCoords.y = textureCoords.y;
		}

        kt = texture(myTexture, tiledTextureCoords);
	} else {
		kt = texture(myTexture, textureCoords); 
	}

	vec3 lightWorldDir = normalize(viewDir);

	float diff = max(dot(vertNormal, lightWorldDir), 0.0);
	vec3 reflection = normalize(reflect(-lightWorldDir, vertNormal));
	float spec = max(dot(viewDir, reflection), 0.0);
	spec = pow(spec, 14.0);

	// Compute screen-space edge width using normal vector
	float edge = fwidth(vertNormal.x) + fwidth(vertNormal.y) + fwidth(vertNormal.z);
	float normalLength = length(vertNormal);
	float outlineFactor = smoothstep(0.3, 1.2 + edge * 1, normalLength);

	// White outline if normal is near edge
	vec4 outlineColor = vec4(1.0, 1.0, 3.0, 0.0); // white

	vec4 litColor = (ka + (diff * kd) + (spec * ks)) * kt;

	fragColor = mix(outlineColor, litColor, outlineFactor);
}