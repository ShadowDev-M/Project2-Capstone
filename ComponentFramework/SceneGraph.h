#pragma once

#include "Actor.h"
#include "PhysicsComponent.h"
#include "ShaderComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "AssetManager.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "ScriptComponent.h"
#include "AnimatorComponent.h"
#include "MemoryRecorder.h"

#include "ScriptAbstract.h"

class SceneGraph
{
	friend class XMLObjectFile;

	// delete copy and move constructers
	SceneGraph(const SceneGraph&) = delete;
	SceneGraph(SceneGraph&&) = delete;
	SceneGraph& operator = (const SceneGraph&) = delete;
	SceneGraph& operator = (SceneGraph&&) = delete;
private:
	// main actor map, ids and actors
	std::unordered_map<uint32_t, Ref<Actor>> Actors;

	// seconday actor map, acts as an inbetween the original name key to actor lookup and the new id key 
	std::unordered_map<std::string, uint32_t> ActorNameToId;
	
	// vector that holds all tags (giving some default ones here)
	// TODO: save tags throughout engine lifetime (closing/opening)
	std::vector<std::string> allTags = { "Untagged", "MainCamera", "Player", "Ground" };

	// helper function for renaming an actor
	void UpdateActorNameMap(uint32_t actorID_, const std::string& oldName_, const std::string& newName_) {
		if (!oldName_.empty()) {
			ActorNameToId.erase(oldName_);
		}
		ActorNameToId[newName_] = actorID_;
	}

	// reference to the maincamera
	Ref<Actor> m_mainCamera;

public:
	// Meyers Singleton (from JPs class)
	static SceneGraph& getInstance() {
		static SceneGraph instance;
		return instance;
	}

	SceneGraph();
	~SceneGraph();
	
	bool OnCreate();
	void OnDestroy();
	void Update(const float deltaTime);

	// Camera functions
	Ref<Actor> GetMainCamera() const;
	void SetMainCamera(Ref<Actor> actor_);
	Ref<Actor> GetCameraByName(const std::string& name_) const;

	void Start();
	void Stop();
	void SaveFile(std::string name) const;

	bool AddActor(Ref<Actor> actor);
	bool RenameActor(const std::string& oldName_, const std::string& newName_);
	void LoadActor(const char* name_, Ref<Actor> parent = Ref<Actor>());

	std::vector<std::string>& getAllTags() { return allTags; }

	void addTag(const std::string& tag) {
		if (std::find(allTags.begin(), allTags.end(), tag) == allTags.end()) {
			allTags.push_back(tag);
		}
	}

	void removeTag(const std::string& tag) {
		allTags.erase(std::remove(allTags.begin(), allTags.end(), tag), allTags.end());
	}

	const std::unordered_map<uint32_t, Ref<Actor>> getAllActors() const { return Actors; }

	Ref<Actor>GetActorCStr(const char* actorName) const;

	Ref<Actor> GetActor(const std::string& actorName) const;

	Ref<Actor> GetActorById(uint32_t actorId) const {
		auto it = Actors.find(actorId);
		return (it != Actors.end()) ? it->second : nullptr; // conditional operator (if actor found by id return it, otherwise return nullptr)
	}

	std::vector<std::string> GetAllActorNames() const;

	bool RemoveActor(const std::string& actorName);
	void RemoveAllActors();
	
	void Preload(ScriptComponent* script);

	// map to store selected/colorpicked actors
	std::unordered_map<uint32_t, Ref<Actor>> debugSelectedAssets;

	mutable std::string cellFileName = "LevelThree";
};