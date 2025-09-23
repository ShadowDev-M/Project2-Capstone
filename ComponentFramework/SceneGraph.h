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
//#include "Raycast.h"
// this class is very similar to the assetmanager pre-singleton just inline
// I created this class pretty much as a helper class just to hold all the actors and a bunch of functions that would simplify the process of dealing with multiple actors 

// Thoughts on making the scenegraph a singleton: It would allow me to preload certain actors and just place them in any scene later on, (I.E unity prefabs), only issue with this is if in one scene I edited an object, those same values would carry over to another scene

class SceneGraph
{
	friend class XMLObjectFile;
private:
	std::unordered_map<std::string, Ref<Actor>> Actors;
	
	Ref<ShaderComponent> pickerShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colourPickVert.glsl", "shaders/colourPickFrag.glsl");

	GLuint pickingFBO = 0;
	GLuint pickingTexture = 0;
	GLuint pickingDepth = 0;

	int pickingFBOWidth, pickingFBOHeight;

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

		pickerShader->OnCreate();
	}
	~SceneGraph() { RemoveAllActors();
	pickerShader->OnDestroy();
	glDeleteFramebuffers(1, &pickingFBO);
	glDeleteRenderbuffers(1, &pickingDepth);
	glDeleteTextures(1, &pickingTexture);


	}

	void useDebugCamera() {
		usedCamera = debugCamera->GetComponent<CameraComponent>();
	}

	std::unordered_map<std::string, Ref<Actor>> debugSelectedAssets;

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

	bool AddActor(Ref<Actor> actor) {
		const std::string& name = actor->getActorName();
		auto it = Actors.find(name);

		// check to see if there is already an actor with the same name in the map
		if (it != Actors.end()) {
			Debug::Warning("There is already an actor with the name: " + name, __FILE__, __LINE__);
			return false;
		}

		Actors[name] = actor;
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
		auto it = Actors.find(actorName);

		// if actor is found return it, else throw error
		if (it != Actors.end()) {
			return it->second;
		}
		else {
			Debug::Error("Can't fint requested actor: ", __FILE__, __LINE__);
			return nullptr;
		}
	}

	std::vector<std::string> GetAllActorNames() const {
		std::vector<std::string> allActorNames;

		for (auto& pair : Actors) {
			allActorNames.push_back(pair.first);
		}

		return allActorNames;
	}

	bool RemoveActor(const std::string& actorName) {

		auto it = Actors.find(actorName);

		// if actor is found remove it, else throw warning
		if (it != Actors.end()) {
			Ref<Actor> actorToRemove = it->second;
			actorToRemove->DeleteComponent<CameraComponent>();

			if (actorToRemove->GetComponent<CameraActor>()) {
				actorToRemove->GetComponent<CameraActor>()->OnDestroy();
			}


			// if the actor that is being removed is parented or a parent, get all children
			std::vector<std::string> childrenToRemove;
			for (const auto& pair : Actors) {
				if (pair.second->getParentActor() == actorToRemove.get()) {
					childrenToRemove.push_back(pair.first);
				}
			}

			// recursivly remove each child actor
			for (const std::string& childName : childrenToRemove) {
				RemoveActor(childName);
			}

			// also remove the actor from the debug
			debugSelectedAssets.erase(actorName);

			actorToRemove->OnDestroy();
			Actors.erase(it);
			return true;
		}
		else {
			Debug::Warning("Actor: " + actorName + " does not exist!", __FILE__, __LINE__);
			return false;
		}
	}

	// lists all actors in the map
	void ListAllActors() const {
		std::cout << "All actors in the scene: " << std::endl;
		for (auto it = Actors.begin(); it != Actors.end(); it++) {
			std::cout << it->first << std::endl;
		}
		std::cout << "------------------------------------------" << std::endl;
	}


	void RemoveAllActors() {
		std::cout << "Deleting All Actors In The Scene" << std::endl;

		// call the OnDestroy for each actor 
		for (auto& pair : Actors) {
			pair.second->OnDestroy();
		}

		// clear the map
		Actors.clear();
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

	Ref<Actor> pickColour(int mouseX, int mouseY) {

		int w, h;
		SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);


		//use the picking buffer so it is seperated
		glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		

		//get the special shader for picking and set its uniforms
		glUseProgram(pickerShader->GetProgram());
		glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
		glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

		for (auto& actor : Actors) {

			if (!actor.second->GetComponent<MeshComponent>()) {continue;}//no meshcomponent for actor

			//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
			glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor.second->GetModelMatrix(getUsedCamera()));
			
			//encode the id of the actor as rgb
			Vec3 idColor = Actor::encodeID(actor.second->getId());

			//send over the rbg to the shader to use as its rendered colour
			glUniform3fv(pickerShader->GetUniformID("uIDColor"), 1, &idColor.x); 

			glBindTexture(GL_TEXTURE_2D, 0);
			actor.second->GetComponent<MeshComponent>()->Render();
		}

		//rgb pixel data
		unsigned char pixel[3];

		std::cout << "w / h: " << w << ", " << h << "\n"; 
		
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		//get the mouse click's rgb pixel data
		glReadPixels(mouseX, h - mouseY, 1, 1,
			GL_RGB, GL_UNSIGNED_BYTE, pixel);
		
		//reverse selected pixel's rgb and decode into an id
		uint32_t pickedID = Actor::decodeID(pixel[0], pixel[1], pixel[2]);

		//unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);

		//check if the clicked pixel's colour id is the same as any of the actors
		for (auto& actor : Actors) {
			if (actor.second->getId() == pickedID) return actor.second;
		}

		return nullptr; // nothing clicked
	}

	// all const
	void Render() const {



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


				if (!debugSelectedAssets.empty() && debugSelectedAssets.find(actor->getActorName()) != debugSelectedAssets.end()) glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
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

	}

	bool OnCreate() {



		// if an actor was setup wrong throw an error
		for (auto& actor : Actors) {
			if (!actor.second->OnCreate()) {
				Debug::Error("Actor failed to initialize: " + actor.first, __FILE__, __LINE__);
				return false;
			}

		}

		return true;
	}

};

