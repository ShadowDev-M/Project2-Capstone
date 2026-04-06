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
#include "ProjectSettingsManager.h"

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

	// redirector helpers
	void Start();
	void Stop();
	void SaveFile(std::string name) const;
	void Preload(ScriptComponent* script);

	// main actor management functions
	bool AddActor(Ref<Actor> actor);
	bool RenameActor(const std::string& oldName_, const std::string& newName_);
	void LoadActor(const char* name_, Ref<Actor> parent = Ref<Actor>());
	bool RemoveActor(const std::string& actorName);
	void RemoveAllActors();

	// actor helpers
	std::string GenerateUniqueActorName(const std::string& originalName) const;
	Ref<Actor> DeepCopyActor(const std::string& newName_, Ref<Actor> original_, Actor* newParent = nullptr);
	Ref<Actor> InstantiatePrefab(const fs::path& prefabPath, Vec3 position = Vec3(0, 0, 0), Quaternion rotation = Quaternion(1, Vec3(0, 0, 0)), Actor* parent = nullptr);
	const std::unordered_map<uint32_t, Ref<Actor>> getAllActors() const { return Actors; }
	Ref<Actor>GetActorCStr(const char* actorName) const;
	Ref<Actor> GetActor(const std::string& actorName) const;
	Ref<Actor> GetActorById(uint32_t actorId) const {
		auto it = Actors.find(actorId);
		return (it != Actors.end()) ? it->second : nullptr; // conditional operator (if actor found by id return it, otherwise return nullptr)
	}
	std::vector<std::string> GetAllActorNames() const;

	// these are now purely redirectors for the global tags
	std::vector<std::string>& getAllTags() {
		return ProjectSettingsManager::getInstance().Get().tags;
	}
	void addTag(const std::string& tag) {
		auto& tags = ProjectSettingsManager::getInstance().Get().tags;
		if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
			tags.push_back(tag);
			ProjectSettingsManager::getInstance().SaveDefault();
		}
	}
	void removeTag(const std::string& tag) {
		auto& tags = ProjectSettingsManager::getInstance().Get().tags;
		tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
		ProjectSettingsManager::getInstance().SaveDefault();
	}

	// map to store selected/colorpicked actors
	std::unordered_map<uint32_t, Ref<Actor>> debugSelectedAssets;

	// EditorManager/Current scene saving/loading
	mutable std::string sceneFileName = "SampleScene";
};