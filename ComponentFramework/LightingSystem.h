#pragma once
#include "Actor.h"

// the lighting in the engine was spread across multiple different functions through the actor and scenegraph
// so I'm creating a system for it now, that way everything that deals with lighting is all in one place

class LightingSystem
{
	// deleting copy and move constructers, setting up singleton
	LightingSystem() = default;
	LightingSystem(const LightingSystem&) = delete;
	LightingSystem(LightingSystem&&) = delete;
	LightingSystem& operator=(const LightingSystem&) = delete;
	LightingSystem& operator=(LightingSystem&&) = delete;

	std::vector<Ref<Actor>> lightActors;
	Vec3 ambientColor = Vec4(0.1f, 0.1f, 0.15f, 0.0f); // TODO: saving ambient light (global), can edit in editor

public:
	// Meyers Singleton (from JPs class)
	static LightingSystem& getInstance() {
		static LightingSystem instance;
		return instance;
	}

	// light management functions 
	void AddActor(Ref<Actor> actor_);
	void RemoveActor(Ref<Actor> actor_);
	void ClearActors() { lightActors.clear(); }
	const std::vector<Ref<Actor>>& GetLightActors() const { return lightActors; }
	
	// getters and setters for ambient
	Vec4 GetAmbient() const { return ambientColor; }
	void SetAmbient(const Vec4& ambient_) { ambientColor = ambient_; }

	// Uploads all light data to a given shader program
	// make sure to only call it once per shader that needs lighting
	void UploadUniforms(GLuint shaderProgram) const;
};