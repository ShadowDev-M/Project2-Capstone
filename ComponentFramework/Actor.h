#pragma once
#include <vector>
#include <Matrix.h>
#include <iostream>
#include "Raycast.h"
#include "Component.h"
// transformcomponent
using namespace MATH;

class Actor : public Component {
	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;
	Actor& operator= (const Actor&) = delete;
	Actor& operator=(Actor&&) = delete;

protected:
	std::vector<Ref<Component>> components;
	Matrix4 modelMatrix;
	// string will be used as a key for the unordered map in the scene
	std::string actorName;
	Vec3 selectionColour = Vec3(0.5f,0.5f,0.5f);
public:
	Actor(Component* parent_);

	// constructer for setting the parent and the name of an actor (name is const because it doesn't need to be changed when its set)
	Actor(Component* parent_, const std::string& actorName_);



	// getter for the actor name
	const std::string& getActorName() { return actorName; }
	void setActorName(const std::string& actorName_) { actorName = actorName_; }


	~Actor();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;

	Vec3 getSelectionColour() { return selectionColour; }

	template<typename ComponentTemplate>
	void AddComponent(Ref<ComponentTemplate> component_) {
		if (GetComponent<ComponentTemplate>().get() != nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		}
		components.push_back(component_);
	}

	template<typename ComponentTemplate, typename ... Args>
	void AddComponent(Args&& ... args_) {
		/// before you add the component ask if you have the component in the list already,
		/// if so - don't add a second one. 
		if (GetComponent<ComponentTemplate>().get() != nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		} 
		/// Finish building the component and add the component to the list 
		components.push_back(std::make_shared<ComponentTemplate>(std::forward<Args>(args_)...));
	}


	template<typename ComponentTemplate>
	Ref<ComponentTemplate> GetComponent() const {
		for (auto& component : components) {
			if (dynamic_cast<ComponentTemplate*>(component.get())) {
				/// This is a dynamic cast designed for shared_ptr's
				/// https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
				return std::dynamic_pointer_cast<ComponentTemplate>(component);
			}
		}
		return Ref<ComponentTemplate>(nullptr);
	}



	// Added this function previously but realized since the components are shared if I remove a component and several actors share that component it just deletes the component, 
	// However this function can still be used later on when components are created in the scene and need to be deleted
	template<typename ComponentTemplate>
	void DeleteComponent() {
		/// check if the component exists
		if (GetComponent<ComponentTemplate>().get() == nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to remove a component type that does not exist - ignored\n";
#endif
			return;
		}
		/// Finish building the component and add the component to the list 
		GetComponent<ComponentTemplate>()->OnDestroy();

		auto it = std::find(components.begin(), components.end(), GetComponent<ComponentTemplate>());
		if (it != components.end()) {
			components.erase(it);
		}
	}

	// Removes the component from the actor
	// DOES NOT CALL OnDestroy, USE THIS FUNCTION FOR SHARED COMPONENTS, USE DeleteComponent FOR EVERYTHING ELSE
	template<typename ComponentTemplate>
	void RemoveComponent() {
		/// check if the component exists
		if (GetComponent<ComponentTemplate>().get() == nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to remove a component type that does not exist - ignored\n";
#endif
			return;
		}

		auto it = std::find(components.begin(), components.end(), GetComponent<ComponentTemplate>());
		if (it != components.end()) {
			components.erase(it);
		}
	}

	// function that replaces an actors shared component
	template<typename ComponentTemplate>
	void ReplaceComponent(Ref<ComponentTemplate> newComponent) {
		for (auto& component : components) {
			if (std::dynamic_pointer_cast<ComponentTemplate>(component)) {
				component = newComponent;

				return;
			}
		}

		// if the component that is trying to be replaced doesn't exist, add it instead
		AddComponent(newComponent);
	}


	void ListComponents() const;
	void RemoveAllComponents();

	//
	Matrix4 GetModelMatrix(Ref<Actor> camera = nullptr);
	
	/// <summary>
	/// Determines whether a ray intersects with the mesh of the actor
	/// </summary>
	/// <param name="intersectSpot">Pointer to Vec3 value to be set to the triangle position the ray intersects with, if it does. </param>
	bool GetIntersectTriangles(Vec3 start, Vec3 dir, Vec3* intersectSpot = nullptr);


	// function to get the parent of an actor
	Actor* getParentActor() const {
		// if actor is not a root actor return parent
		if (!isRootActor()) {
			return dynamic_cast<Actor*>(parent);
		}
		else {
			return nullptr;
		}
	}

	bool isRootActor() const {
		return parent == nullptr || dynamic_cast<Actor*>(parent) == nullptr;
	}
};

