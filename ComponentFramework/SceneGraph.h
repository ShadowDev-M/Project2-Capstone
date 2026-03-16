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
#include <mutex>
#include <queue>


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

	GLenum drawMode = GL_FILL;

	Ref<ShaderComponent> pickerShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colourPickVert.glsl", "shaders/colourPickFrag.glsl");

	std::thread workerThread;
	std::atomic<bool> shouldStop{ false };  // Thread-safe flag

	bool RENDERMAINSCREEN = 0;

	// helper function for renaming an actor
	void UpdateActorNameMap(uint32_t actorID_, const std::string& oldName_, const std::string& newName_) {
		if (!oldName_.empty()) {
			ActorNameToId.erase(oldName_);
		}
		ActorNameToId[newName_] = actorID_;
	}

	std::vector<Ref<Component>> workerQueue;
	//std::vector<Ref<Component>> finishedQueue;


	std::queue<std::function<void()>> mainThreadTasks;

	std::mutex queueMutex;

	std::mutex taskMutex;
	std::condition_variable taskCV;

	void meshLoadingWorker();

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
	void Render() const;

	// Animation functions
	void processMainThreadTasks();
	void scheduleOnMain(std::function<void()> task);
	bool queryMeshLoadStatus(std::string name);
	void pushMeshToWorker(Ref<MeshComponent> mesh);
	void pushAnimationToWorker(Ref<Animation> animation);
	void stopMeshLoadingWorker();
	bool isAllComponentsLoaded() { return (workerQueue.empty()); }
	void startMeshLoadingWorkerThread();

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

	std::unordered_map<uint32_t, Ref<Actor>> getAllActors() { return Actors; }

	Ref<Actor>GetActorCStr(const char* actorName) const;

	Ref<Actor> GetActor(const std::string& actorName) const;

	Ref<Actor> GetActorById(uint32_t actorId) const {
		auto it = Actors.find(actorId);
		return (it != Actors.end()) ? it->second : nullptr; // conditional operator (if actor found by id return it, otherwise return nullptr)
	}

	std::vector<std::string> GetAllActorNames() const;

	bool RemoveActor(const std::string& actorName);

	// lists all actors name and ID
	void ListAllActors() const;

	void RemoveAllActors();

	//Colour picking for object selection
	Ref<Actor> pickColour(int mouseX, int mouseY);
	
	void Preload(ScriptComponent* script);


	void SetDrawMode(GLenum drawMode_) { drawMode = drawMode_; }

	GLenum GetDrawMode() const { return drawMode; }

	// map to store selected/colorpicked actors
	std::unordered_map<uint32_t, Ref<Actor>> debugSelectedAssets;

	mutable std::string cellFileName = "LevelThree";
};