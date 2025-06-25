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
//#include "Raycast.h"
// this class is very similar to the assetmanager pre-singleton just inline
// I created this class pretty much as a helper class just to hold all the actors and a bunch of functions that would simplify the process of dealing with multiple actors 

// Thoughts on making the scenegraph a singleton: It would allow me to preload certain actors and just place them in any scene later on, (I.E unity prefabs), only issue with this is if in one scene I edited an object, those same values would carry over to another scene

class SceneGraph
{
	friend class XMLObjectFile;
private:
	std::unordered_map<std::string, Ref<Actor>> Actors;

	GLuint selectionFBO = 0;
	GLuint selectionColorTex = 0;
	GLuint selectionDepthRBO = 0;
	int fboWidth = 0, fboHeight = 0;  // Or match your window size
	
public:
	SceneGraph() {}
	~SceneGraph() { RemoveAllActors(); }

	std::unordered_map<std::string,Ref<Actor>> debugSelectedAssets;


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

	void SaveFile(std::string name);

	void LoadAllActorsFromFile(std::string name) {
		
	}

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

	bool RemoveActor(const std::string& actorName) {
		auto it = Actors.find(actorName);
		
		// if actor is found remove it, else throw warning
		if (it != Actors.end()) {
			Actors.erase(it);
			return true;
		}
		else {
			Debug::Warning("Actor: " + actorName + "| does not exist!", __FILE__, __LINE__);
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
				
				Matrix4 modelMatrix = actor->GetModelMatrix(GetActor("camera"));

				//glDisable(GL_DEPTH_TEST);
				//Matrix4 outlineModel = modelMatrix * MMath::scale(1.05, 1.05, 1.05); // Slightly larger

				//glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, outlineModel);

				//mesh->Render(GL_TRIANGLES);



				glEnable(GL_DEPTH_TEST);
				//glUniformMatrix4fv("shaders/texturePhongVert.glsl", 1, GL_FALSE, modelMatrix);
				
				
				if (!debugSelectedAssets.empty() && debugSelectedAssets.find(actor->getActorName()) != debugSelectedAssets.end()) glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
				else glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());

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

