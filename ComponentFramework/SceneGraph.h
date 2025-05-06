#pragma once

#include <unordered_map>
#include <string>
#include "Actor.h"
#include "Debug.h"

#include "PhysicsComponent.h"
#include "ShaderComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"


// this class is very similar to the assetmanager pre-singleton just inline
// I created this class pretty much as a helper class just to hold all the actors and a bunch of functions that would simplify the process of dealing with multiple actors 

// Thoughts on making the scenegraph a singleton: It would allow me to preload certain actors and just place them in any scene later on, (I.E unity prefabs), only issue with this is if in one scene I edited an object, those same values would carry over to another scene

class SceneGraph
{
	friend class XMLObjectFile;
private:
	std::unordered_map<std::string, Ref<Actor>> Actors;

public:
	SceneGraph() {}
	~SceneGraph() { RemoveAllActors(); }

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

	void LoadAllActorsFromFile(std::string name) {
		
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
				glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
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

