#pragma once
#include <vector>
#include <Matrix.h>
#include <iostream>
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

public:
	Actor(Component* parent_);

	// constructer for setting the parent and the name of an actor (name is const because it doesn't need to be changed when its set)
	Actor(Component* parent_, const std::string& actorName_);

	// getter for the actor name
	const std::string& getActorName() { return actorName; }

	~Actor();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;

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

	void ListComponents() const;
	void RemoveAllComponents();


	//
	Matrix4 GetModelMatrix(Ref<Actor> camera = Ref<Actor>());
	

};

