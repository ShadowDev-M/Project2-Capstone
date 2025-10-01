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
#include "CameraActor.h"
#include "AssetManager.h"
#include "CameraComponent.h"
#include "LightComponent.h"

#include "Raycast.h"

class SceneGraph
{
	friend class XMLObjectFile;

	//I don't want anything else to really touch the docking FBOs yet, so may as well give DockingWindow access to the private members of SceneGraph
	friend class DockingWindow;
private:
	// main actor map, replaced old name lookup system with new id lookup system (this makes things more optimized and better in the longrun when we expand the engine)
	// mainly did this because renaming was annoying and this system makes things so easy
	std::unordered_map<uint32_t, Ref<Actor>> Actors;

	// seconday actor map, this acts as an inbetween the original name key to actor lookup and the new id key 
	// this way we can still look up actors by name but it'll still be optimized with the actor ids
	std::unordered_map<std::string, uint32_t> ActorNameToId;
	
	Ref<ShaderComponent> pickerShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colourPickVert.glsl", "shaders/colourPickFrag.glsl");

	bool RENDERMAINSCREEN = 0;
 
	GLuint pickingFBO = 0;
	GLuint pickingTexture = 0;
	GLuint pickingDepth = 0;

	int pickingFBOWidth, pickingFBOHeight;

	GLuint dockingFBO = 0;
	GLuint dockingTexture = 0;
	GLuint dockingDepth = 0;

	int dockingFBOWidth, dockingFBOHeight;

	GLuint selectionFBO = 0;
	GLuint selectionColorTex = 0;
	GLuint selectionDepthRBO = 0;
	int fboWidth = 0, fboHeight = 0;  // Or match your window size

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
	// Meyers Singleton (from JPs class)
	static SceneGraph& getInstance() {
		static SceneGraph instance;
		return instance;
	}

	SceneGraph() {
	
		int w, h;

		SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
		createFBOPicking(w,h);

		//fbo for the imgui docking
		createDockingFBO(w, h);

		pickerShader->OnCreate();
	}
	~SceneGraph() { RemoveAllActors();
	pickerShader->OnDestroy();
	glDeleteFramebuffers(1, &pickingFBO);
	glDeleteRenderbuffers(1, &pickingDepth);
	glDeleteTextures(1, &pickingTexture);

	glDeleteFramebuffers(1, &dockingFBO);
	glDeleteRenderbuffers(1, &dockingDepth);
	glDeleteTextures(1, &dockingTexture);

	}

	void useDebugCamera() {
		usedCamera = debugCamera->GetComponent<CameraComponent>();
	}

	// changing this to use actor ids as well
	// pretty much all of the code stays the same (just instead of getting actor name get id) but some code actually got cut down cause of this which is nice
	std::unordered_map<uint32_t, Ref<Actor>> debugSelectedAssets;

	mutable std::string cellFileName = "LevelThree";

	void setUsedCamera(Ref<CameraComponent> newCam);

	Ref<CameraComponent> getUsedCamera() const {
		if (!usedCamera || !usedCamera->GetUserActor()) {


			return debugCamera->GetComponent<CameraComponent>();


			std::cout << "ERROR: NO CAMERA EXISTS IN SCENEGRAPH" << std::endl;
		}


		return usedCamera;
	}

	void checkValidCamera() {

		if (!usedCamera || !usedCamera->GetUserActor()) {
			std::cout << "usedCamera is invalid" << std::endl;



			std::cout << "try debugCam" << std::endl;

			if (debugCamera && debugCamera->GetComponent<CameraComponent>()) {
				std::cout << "DebugCam is found" << std::endl;
			}
			else {

				std::cout << "DebugCam is invalid" << std::endl;
				debugCamera = std::make_shared<Actor>(nullptr, "cameraDebugOne");
				debugCamera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));
				debugCamera->OnCreate();

				//doesn't need to be added to the sceneGraph
				//sceneGraph.AddActor(debugCamera);

				debugCamera->AddComponent<CameraComponent>(debugCamera, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
				debugCamera->GetComponent<CameraComponent>()->OnCreate();
				debugCamera->GetComponent<CameraComponent>()->fixCameraToTransform();

				setUsedCamera(debugCamera->GetComponent<CameraComponent>());
			}
			usedCamera = debugCamera->GetComponent<CameraComponent>();


		}
	}

	void ValidateAllLights() {
		for (std::vector<Ref<Actor>>::iterator it; it != lightActors.end(); ++it) {
			if (!(*it)->ValidateLight()) lightActors.erase(it);
		}
	}

	bool AddLight(Ref<Actor> actor) {
		if (!actor->ValidateLight()) return false;
		
		lightActors.push_back(actor); 
		return true;
	}

	bool GetLightExist(Ref<Actor> actor) {
		auto it = std::find(lightActors.begin(), lightActors.end(), actor);

		if (it != lightActors.end()) {
			// Element found
			return true;
		}
		
		return false;
	}

	bool RemoveLight(Ref<Actor> actor) {
		lightActors.erase(std::remove(lightActors.begin(), lightActors.end(), actor), lightActors.end());

		return false;
	}

	// now uses ID
	bool AddActor(Ref<Actor> actor) {
		if (!actor) {
			Debug::Error("Attempted to add null actor", __FILE__, __LINE__);
			return false;
		}

		const std::string& name = actor->getActorName();
		uint32_t id = actor->getId();

		// check if an actor with this ID already exists
		auto it = Actors.find(id);
		if (it != Actors.end()) {
			Debug::Warning("An actor with ID " + std::to_string(id) + " already exists", __FILE__, __LINE__);
			return false;
		}

		// check to see if there is already an actor with the same name in the map
		auto nameIt = ActorNameToId.find(name);
		if (nameIt != ActorNameToId.end() && nameIt->second != id) {
			Debug::Warning("An actor named: " + name + " already exists", __FILE__, __LINE__);
			return false;
		}

		// add the actor using ID as key
		Actors[id] = actor;
		ActorNameToId[name] = id;

		return true;
	}

	// 
	bool RenameActor(const std::string& oldName_, const std::string& newName_) {
		if (oldName_ == newName_) return true;
		if (newName_.empty()) {
			Debug::Warning("Cannot rename actor to empty name", __FILE__, __LINE__);
			return false;
		}

		// check if new name is already taken
		auto nameIt = ActorNameToId.find(newName_);
		if (nameIt != ActorNameToId.end()) {
			Debug::Warning("An actor named: " + newName_ + " already exists", __FILE__, __LINE__);
			return false;
		}

		// find the actor by old name
		auto oldNameIt = ActorNameToId.find(oldName_);
		if (oldNameIt == ActorNameToId.end()) {
			Debug::Error("Cannot find actor named: " + oldName_, __FILE__, __LINE__);
			return false;
		}

		uint32_t actorId = oldNameIt->second;
		Ref<Actor> actor = Actors[actorId];

		// update the actor's internal name
		actor->setActorName(newName_);

		// update the name lookup map
		UpdateActorNameMap(actorId, oldName_, newName_);

		return true;
	}

	void SaveFile(std::string name) const;

	void LoadAllActorsFromFile(std::string name) {

	}

	/// <summary>
	/// Loads actor from file into scenegraph
	/// </summary>
	void LoadActor(const char* name_, Ref<Actor> parent = Ref<Actor>());


	Ref<Actor> MeshRaycast(Vec3 start, Vec3 end) {
		float minDistance = 0;
		Ref<Actor> closestSelected = nullptr;

		for (auto& actor : Actors) {
			Ref<Actor> targetActor = actor.second;

			Ref<TransformComponent> actorTransform = targetActor->GetComponent<TransformComponent>();

			Vec3 dir = VMath::normalize(end - start);

			//skip to next if no transform
			if (actorTransform == nullptr) { continue; }

			//if actor's origin isn't within a 30 degrees cone to the camera we can skip to make less expensive by assuming it's probably not intersecting (May have to be increased)
			if (!Raycast::isInRayCone(actorTransform->GetPosition(), start, dir, 0.8660254f)) { continue; }

			//	std::cout << targetActor->getActorName() << std::endl;

			Vec3 intersectSpot;
			if (targetActor->GetIntersectTriangles(start, dir, &intersectSpot)) {

				float actorDistance = VMath::distance(start, intersectSpot);
				std::cout << targetActor->getActorName() << " : " << actorDistance << std::endl;
				if (actorDistance < minDistance || minDistance == 0) {
					minDistance = actorDistance;
					closestSelected = targetActor;
				}
			}
		}

		if (closestSelected) {
			return closestSelected;
		}



		return nullptr;
	}

	Ref<Actor> GetActor(const std::string& actorName) const {
        // try to find the actor by name
		auto nameIt = ActorNameToId.find(actorName);
        if (nameIt == ActorNameToId.end()) {
            Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
            return nullptr;
        }

		// try to find actor by ID
        uint32_t actorId = nameIt->second;
        auto actorIt = Actors.find(actorId);

        if (actorIt != Actors.end()) {
            return actorIt->second;
        }

		// if the actor can't be found by name or by ID
        Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
        return nullptr;
    }

	Ref<Actor> GetActorById(uint32_t actorId) const {
		auto it = Actors.find(actorId);
		return (it != Actors.end()) ? it->second : nullptr; // conditional operator (if actor found by id return it, otherwise return nullptr)
	}

	std::vector<std::string> GetAllActorNames() const {
		std::vector<std::string> allActorNames;
		allActorNames.reserve(Actors.size());

		for (const auto& pair : Actors) {
			allActorNames.push_back(pair.second->getActorName());
		}

		return allActorNames;
	}

	bool RemoveActor(const std::string& actorName) {
		auto nameIt = ActorNameToId.find(actorName);
		if (nameIt == ActorNameToId.end()) {
			Debug::Warning("Actor: " + actorName + " does not exist!", __FILE__, __LINE__);
			return false;
		}

		uint32_t actorId = nameIt->second;
		auto actorIt = Actors.find(actorId);

		if (actorIt == Actors.end()) {
			Debug::Error("Actor: " + actorName + " ID does not exist!", __FILE__, __LINE__);
			return false;
		}

		Ref<Actor> actorToRemove = actorIt->second;
		actorToRemove->DeleteComponent<CameraComponent>();

		if (actorToRemove->GetComponent<CameraActor>()) {
			actorToRemove->GetComponent<CameraActor>()->OnDestroy();
		}

		// if the actor that is being removed is parented or a parent, get all children
		std::vector<std::string> childrenToRemove;
		for (const auto& pair : Actors) {
			if (pair.second->getParentActor() == actorToRemove.get()) {
				childrenToRemove.push_back(pair.second->getActorName());
			}
		}

		// recursivly remove each child actor
		for (const std::string& childName : childrenToRemove) {
			RemoveActor(childName);
		}

		// also remove the actor from the debug
		debugSelectedAssets.erase(actorId);

		actorToRemove->OnDestroy();

		// remove from both maps
		Actors.erase(actorId);
		ActorNameToId.erase(actorName);

		return true;
	}

	// lists all actors name and ID
	void ListAllActors() const {
		std::cout << "All actors in the scene: " << std::endl;
		for (const auto& pair : Actors) {
			std::cout << pair.second->getActorName() << " (ID: " << pair.first << ")" << std::endl;
		}
	}

	void RemoveAllActors() {
		std::cout << "Deleting All Actors In The Scene" << std::endl;

		// call the OnDestroy for each actor 
		for (auto& pair : Actors) {
			pair.second->OnDestroy();
		}

		// clear the maps
		Actors.clear();
		ActorNameToId.clear();
		debugSelectedAssets.clear();
	}

	/// <summary>
	/// The update called to handle physics components
	/// </summary>
	/// <param name="deltaTime"></param>
	void Update(const float deltaTime) {
		checkValidCamera();
		//	std::cout << usedCamera << std::endl;





		for (auto& pair : Actors) {




			// get the second value from the pair (actor)
			Ref<Actor> actor = pair.second;
			// get the physics component from the actor
			Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();
			// if the actor has a physicis component, call the update
			if (PC) {
				PC->UpdateP(deltaTime, actor);
			}
		}
		/*if (!(usedCamera == nullptr)) {
			setUsedCamera();
		}*/
	}


	//values for picking framebuffer


	//create colour picker fbo
	void createFBOPicking(int w, int h) {

		//create depth buffer
		glGenRenderbuffers(1, &pickingDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, pickingDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		//create picking buffer
		glGenFramebuffers(1, &pickingFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

		//create texture buffer
		glGenTextures(1, &pickingTexture);
		glBindTexture(GL_TEXTURE_2D, pickingTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

		// magic sauce :>
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
								  GL_DEPTH_ATTACHMENT, // 2. attachment point
								  GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
								  pickingDepth);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Framebuffer is not complete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	
	}

	

	void createDockingFBO(int w, int h) {

		//create depth buffer
		glGenRenderbuffers(1, &dockingDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, dockingDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		//create picking buffer
		glGenFramebuffers(1, &dockingFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, dockingFBO);

		//create texture buffer
		glGenTextures(1, &dockingTexture);
		glBindTexture(GL_TEXTURE_2D, dockingTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dockingTexture, 0);

		// magic sauce :>
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
			dockingDepth);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Framebuffer is not complete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	//Colour picking for object selection
	Ref<Actor> pickColour(int mouseX, int mouseY);

	// all const
	void Render() const {
		int w, h;
		SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

		if (!RENDERMAINSCREEN) {


			//use the picking buffer so it is seperated
			glBindFramebuffer(GL_FRAMEBUFFER, dockingFBO);
			glViewport(0, 0, w, h);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		}

		// go through all actors
		for (const auto& pair : Actors) {

			Ref<Actor> actor = pair.second;

			// getting the shader, mesh, and mat for each indivual actor, using mainly for the if statement to check if the actor has each of these components
			Ref<ShaderComponent> shader = actor->GetComponent<ShaderComponent>();
			Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
			Ref<MaterialComponent> material = actor->GetComponent<MaterialComponent>();

			// if the actor has a shader, mesh, and mat component then render it
			if (shader && mesh && material) {



				//glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix() * MMath::translate(Vec3(GetActor("camera")->GetComponent<TransformComponent>()->GetPosition())));

				Matrix4 modelMatrix = actor->GetModelMatrix(getUsedCamera());

				//glDisable(GL_DEPTH_TEST);
				//Matrix4 outlineModel = modelMatrix * MMath::scale(1.05, 1.05, 1.05); // Slightly larger

				//glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, outlineModel);

				//mesh->Render(GL_TRIANGLES);



				glEnable(GL_DEPTH_TEST);
				//glUniformMatrix4fv("shaders/texturePhongVert.glsl", 1, GL_FALSE, modelMatrix);


				if (!debugSelectedAssets.empty() && debugSelectedAssets.find(actor->getId()) != debugSelectedAssets.end()) glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
				else {
					if (pair.second->GetComponent<ShaderComponent>()) {
						glUseProgram(shader->GetProgram());
					}
					else continue;

					//				glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());

				}

				///glUseProgram(pickerShader->GetProgram());

				//Vec3 idColor = Actor::encodeID(actor->id);
				



				glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);

				glBindTexture(GL_TEXTURE_2D, material->getTextureID());
				mesh->Render(GL_TRIANGLES);
				glBindTexture(GL_TEXTURE_2D, 0);



			}

		}
		
		if (!RENDERMAINSCREEN ) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, w, h);
		}

	}

	// Ghost erorr...

	bool OnCreate() {
		// if an actor was setup wrong throw an error
		for (auto& actor : Actors) {
			if (!actor.second->OnCreate()) {
				Debug::Error("Actor failed to initialize: " + actor.second->getActorName(), __FILE__, __LINE__);
				return false;
			}

		}

		return true;
	}

};

