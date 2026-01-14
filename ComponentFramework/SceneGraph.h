#pragma once

#include <unordered_map>
#include <string>
#include "Actor.h"
#include "Debug.h"
#include "PhysicsComponent.h"
#include "ShaderComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "MMath.h"
#include "AssetManager.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "ScriptComponent.h"

#include "Raycast.h"

class SceneGraph
{
	friend class XMLObjectFile;

	//I don't want anything else to really touch the docking FBOs yet, so may as well give SceneWindow access to the private members of SceneGraph
	friend class SceneWindow;
private:
	// main actor map, replaced old name lookup system with new id lookup system (this makes things more optimized and better in the longrun when we expand the engine)
	// mainly did this because renaming was annoying and this system makes things so easy
	std::unordered_map<uint32_t, Ref<Actor>> Actors;

	// seconday actor map, this acts as an inbetween the original name key to actor lookup and the new id key 
	// this way we can still look up actors by name but it'll still be optimized with the actor ids
	std::unordered_map<std::string, uint32_t> ActorNameToId;
	
	GLenum drawMode = GL_FILL;

	Ref<ShaderComponent> pickerShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colourPickVert.glsl", "shaders/colourPickFrag.glsl");

	bool RENDERMAINSCREEN = 0;
 
	GLuint pickingFBO = 0;
	GLuint pickingTexture = 0;
	GLuint pickingDepth = 0;

	int pickingFBOWidth, pickingFBOHeight;

	GLuint dockingFBO = 0;
	GLuint dockingTexture = 0;
	GLuint dockingDepth = 0;

	int dockingFBOWidth, dockingFBOHeight; // Or match your window size

	Ref<Actor> debugCamera;

	Ref<CameraComponent> usedCamera;

	std::vector<Ref<Actor>> lightActors;

	// delete copy and move constructers
	SceneGraph(const SceneGraph&) = delete;
	SceneGraph(SceneGraph&&) = delete;
	SceneGraph& operator = (const SceneGraph&) = delete;
	SceneGraph& operator = (SceneGraph&&) = delete;

	// helper function for renaming an actor
	void UpdateActorNameMap(uint32_t actorID_, const std::string& oldName_, const std::string& newName_) {
		if (!oldName_.empty()) {
			ActorNameToId.erase(oldName_);
		}
		ActorNameToId[newName_] = actorID_;
	}

public:

	//Hardcode the screen height and width rather than using SDL_GetWindowSize, as at lot of the code is designed for 1280 x 720
	static const int SCENEWIDTH = 1280;
	static const int SCENEHEIGHT = 720;


	// Meyers Singleton (from JPs class)
	static SceneGraph& getInstance() {
		static SceneGraph instance;
		return instance;
	}

	SceneGraph();

	~SceneGraph();

	void useDebugCamera();

	// changing this to use actor ids as well
	// pretty much all of the code stays the same (just instead of getting actor name get id) but some code actually got cut down cause of this which is nice
	std::unordered_map<uint32_t, Ref<Actor>> debugSelectedAssets;

	mutable std::string cellFileName = "LevelThree";

	void setUsedCamera(Ref<CameraComponent> newCam);

	Ref<CameraComponent> getUsedCamera() const;

	void checkValidCamera();

	void ValidateAllLights();

	bool AddLight(Ref<Actor> actor);

	std::vector<Vec3> GetLightsPos() const;

	bool GetLightExist(Ref<Actor> actor);

	std::vector<Ref<Actor>> GetLightActors() { return lightActors; }

	void RemoveLight(Ref<Actor> actor) {
		if (GetLightExist(actor)) lightActors.erase(std::find(lightActors.begin(), lightActors.end(), actor));
	}

	// now uses ID
	bool AddActor(Ref<Actor> actor);

	void Start();

	void Stop();

	// 
	bool RenameActor(const std::string& oldName_, const std::string& newName_);

	void SaveFile(std::string name) const;

	void LoadAllActorsFromFile(std::string name) {

	}

	/// <summary>
	/// Loads actor from file into scenegraph
	/// </summary>
	void LoadActor(const char* name_, Ref<Actor> parent = Ref<Actor>());


	Ref<Actor> MeshRaycast(Vec3 start, Vec3 end);

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

	/// <summary>
	/// The update called to handle physics components
	/// </summary>
	/// <param name="deltaTime"></param>
	void Update(const float deltaTime);


	//values for picking framebuffer

	//create colour picker fbo
	void createFBOPicking(int w, int h);

	

	void createDockingFBO(int w, int h);

	//Colour picking for object selection
	Ref<Actor> pickColour(int mouseX, int mouseY);

	// all const
	void Render() const; 

	bool OnCreate();

	void SetDrawMode(GLenum drawMode_) { drawMode = drawMode_; }

	GLenum GetDrawMode() const { return drawMode; }
};

