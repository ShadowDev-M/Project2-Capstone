#include "pch.h"
#include "LightingSystem.h"
#include "LightComponent.h"

void LightingSystem::AddActor(Ref<Actor> actor_) {
	if (actor_->GetComponent<LightComponent>().get() == nullptr) {
		Debug::Error("The Actor must have a LightComponent - ignored ", __FILE__, __LINE__);
		return;
	}

	// additional check to make sure not adding duplicate actors
	if (std::find(lightActors.begin(), lightActors.end(), actor_) != lightActors.end()) {
		Debug::Warning("Actor already added to CollisionSystem", __FILE__, __LINE__);
		return;
	}

	lightActors.push_back(actor_);
}

void LightingSystem::RemoveActor(Ref<Actor> actor_) {
	// finding actor from vector and removing it
	auto it = std::find(lightActors.begin(), lightActors.end(), actor_);
	if (it != lightActors.end()) {
		lightActors.erase(it);
	}
}

void LightingSystem::UploadUniforms(GLuint shaderProgram, const std::vector<int>& shadowCasters) const
{
	if (!shaderProgram) return;

	glUseProgram(shaderProgram);

	// passing number of lights and ambient first
	GLint numLoc = glGetUniformLocation(shaderProgram, "numLights");
	GLint ambLoc = glGetUniformLocation(shaderProgram, "ambient");
	if (numLoc >= 0) glUniform1ui(numLoc, (GLuint)lightActors.size());
	if (ambLoc >= 0) glUniform4fv(ambLoc, 1, &ambientColor.x);

	if (lightActors.empty()) return;

	std::vector<Vec3> lightPos;
	std::vector<Vec4> lightSpec;
	std::vector<Vec4> lightDiff;
	std::vector<float> lightIntensity;
	std::vector<GLuint> lightTypes;
	std::vector<GLint> castsShadow;
	std::vector<GLint> numPointLightShadow;

	// just a performance thing, pre-allocates memory 
	lightPos.reserve(lightActors.size());
	lightSpec.reserve(lightActors.size());
	lightDiff.reserve(lightActors.size());
	lightIntensity.reserve(lightActors.size());
	lightTypes.reserve(lightActors.size());
	castsShadow.reserve(lightActors.size());
	numPointLightShadow.reserve(lightActors.size());

	for (size_t i = 0; i < lightActors.size(); i++) {
		const auto& light = lightActors[i];
		Ref<TransformComponent> TC = light->GetComponent<TransformComponent>();
		Ref<LightComponent> LC = light->GetComponent<LightComponent>();

		if (LC->getType() == LightType::Point) {
			lightPos.push_back(Vec3(light->GetModelMatrix().getColumn(Matrix4::Colunm::three))); // "Colunm" scott-typo lol
			lightTypes.push_back(1u);
		}
		else {
			lightPos.push_back(TC->GetForward());
			lightTypes.push_back(0u);
		}

		lightSpec.push_back(LC->getSpec());
		lightDiff.push_back(LC->getDiff());
		lightIntensity.push_back(LC->getIntensity());

		// number of shadow casters
		int num = (i < shadowCasters.size()) ? shadowCasters[i] : -1;
		castsShadow.push_back(num != -1 ? 1 : 0);

		// >= 0 is pointlight/cube map -2 is sky, -1 is none
		numPointLightShadow.push_back(num >= 0 ? num : -1);
	}

	if (lightPos.empty()) return;
	GLsizei count = (GLsizei)lightPos.size();

	GLint posLoc = glGetUniformLocation(shaderProgram, "lightPos[0]");
	GLint diffLoc = glGetUniformLocation(shaderProgram, "diffuse[0]");
	GLint specLoc = glGetUniformLocation(shaderProgram, "specular[0]");
	GLint intLoc = glGetUniformLocation(shaderProgram, "intensity[0]");
	GLint typLoc = glGetUniformLocation(shaderProgram, "lightType[0]");
	GLint numShadowLoc = glGetUniformLocation(shaderProgram, "lightCastsShadow[0]");
	GLint numPointLoc = glGetUniformLocation(shaderProgram, "numPointShadow[0]");

	if (posLoc >= 0) glUniform3fv(posLoc, count, reinterpret_cast<const GLfloat*>(lightPos.data()));
	if (diffLoc >= 0) glUniform4fv(diffLoc, count, reinterpret_cast<const GLfloat*>(lightDiff.data()));
	if (specLoc >= 0) glUniform4fv(specLoc, count, reinterpret_cast<const GLfloat*>(lightSpec.data()));
	if (intLoc >= 0) glUniform1fv(intLoc, count, lightIntensity.data());
	if (typLoc >= 0) glUniform1uiv(typLoc, count, lightTypes.data());
	if (numShadowLoc >= 0) glUniform1iv(numShadowLoc, count, castsShadow.data());
	if (numPointLoc >= 0) glUniform1iv(numPointLoc, count, numPointLightShadow.data());
}