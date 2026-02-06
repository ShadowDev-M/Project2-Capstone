#pragma once
class MeshComponent;
struct TilingSettings {
	bool isTiled;
	Vec2 tileScale = Vec2(1, 1);
	Vec2 tileOffset;

	bool getIsTiled() const { return isTiled; }
	Vec2 getTileScale() const { return tileScale; }
	Vec2 getTileOffset() const { return tileOffset; }

	void setIsTiled(bool isTiled_) {
		isTiled = isTiled_;
	}

	void setTileScale(Vec2 tileScale_) {
		tileScale = tileScale_;
	}

	void setTileOffset(Vec2 tileOffset_) {
		tileOffset = tileOffset_;
	}
};

using namespace MATH;

class Actor : public Component {
	
	//Script Service should have the ability to fully control actor
	//However, code should be written in a way to avoid lua scripts having this access.
	friend class ScriptService;


	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;
	Actor& operator= (const Actor&) = delete;
	Actor& operator=(Actor&&) = delete;

	static uint32_t getNextID() {
		static uint32_t nextID = 1; // start at 1; 0 = "no actor"
		return nextID++;
	}
	
	uint32_t id;  // unique per actor
	
	TilingSettings tileSettings;

protected:
	std::vector<Ref<Component>> components;
	Matrix4 modelMatrix;
	// string will be used as a key for the unordered map in the scene
	std::string actorName;
	Vec3 selectionColour = Vec3(0.5f,0.5f,0.5f);
public:

	uint32_t getId() const { return id; }

	//For colour picker 
	inline static Vec3 encodeID(uint32_t id) {
		uint8_t r = (id & 0x000000FF);
		uint8_t g = (id & 0x0000FF00) >> 8;
		uint8_t b = (id & 0x00FF0000) >> 16;
		return Vec3(r, g, b) / 255.0f;
	}

	inline static uint32_t decodeID(uint8_t r, uint8_t g, uint8_t b) {
		return r | (g << 8) | (b << 16);
	}

	Actor(Component* parent_);

	// constructer for setting the parent and the name of an actor (name is const because it doesn't need to be changed when its set)
	Actor(Component* parent_, const std::string& actorName_);



	// getter for the actor name
	const std::string& getActorName() { return actorName; }
	void setActorName(const std::string& actorName_) { actorName = actorName_; }
	void setTileSettings(TilingSettings tileSettings_) { tileSettings = tileSettings_; }

	~Actor();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;

	Vec3 getSelectionColour() { return selectionColour; }
	
	TilingSettings getTileSettings() { return tileSettings; }

	template<typename ComponentTemplate>
	void AddComponent(Ref<ComponentTemplate> component_) {
		if (GetComponent<ComponentTemplate>().get() != nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		}

		if (typeid(ComponentTemplate).name() == "LightComponent" && typeid(ComponentTemplate).name() == "TransformComponent") {
			if (ValidateLight()) {
				InitalizeLight();
			}
		}





		components.push_back(component_);
	}

	template<typename ComponentTemplate, typename ... Args>
	void AddComponent(Args&& ... args_) {
		/// before you add the component ask if you have the component in the list already,
		/// if so - don't add a second one (With script component as the exception)
		if (GetComponent<ComponentTemplate>().get() && static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6) != "ScriptComponent") {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		} 



		/// Finish building the component and add the component to the list 
		components.push_back(std::make_shared<ComponentTemplate>(std::forward<Args>(args_)...));
		

		if (typeid(ComponentTemplate).name() == "LightComponent" && typeid(ComponentTemplate).name() == "TransformComponent") {
			if (ValidateLight()) {
				InitalizeLight();
			}
		}
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

	template<typename ComponentTemplate>
	std::vector<Ref<ComponentTemplate>> GetAllComponent() const {
		std::vector<Ref<ComponentTemplate>> componentListReturn;

		for (auto& component : components) {
			if (dynamic_cast<ComponentTemplate*>(component.get())) {
				/// This is a dynamic cast designed for shared_ptr's
				/// https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
				componentListReturn.push_back(std::dynamic_pointer_cast<ComponentTemplate>(component));
			}
		}
		return componentListReturn;

	}

	bool ValidateLight();

	bool InitalizeLight();

	bool DeinitalizeLight();

	// Added this function previously but realized since the components are shared if I remove a component and several actors share that component it just deletes the component, 
	// However this function can still be used later on when components are created in the scene and need to be deleted
	template<typename ComponentTemplate>
	void DeleteComponent() {
		if (typeid(ComponentTemplate).name() == "LightComponent") {
			DeinitalizeLight();
		}
		/// check if the component exists
		if (GetComponent<ComponentTemplate>().get() == nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to remove a component type that does not exist - ignored\n";
#endif
			return;
		}
		/// Finish building the component and add the component to the list 

		GetComponent<ComponentTemplate>()->OnDestroy();

		if (GetComponent<ComponentTemplate>()) {

			auto it = std::find(components.begin(), components.end(), GetComponent<ComponentTemplate>());
			if (it != components.end()) {
				components.erase(it);
			}

		}
		
	}
	// Removes the component from the actor
	// DOES NOT CALL OnDestroy, USE THIS FUNCTION FOR SHARED COMPONENTS, USE DeleteComponent FOR EVERYTHING ELSE
	template<typename ComponentTemplate>
	void RemoveComponent() {

		if (typeid(ComponentTemplate).name() == "LightComponent") {
			if (ValidateLight()) {
				DeinitalizeLight();
			}
		}

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
				if (static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6) == "MeshComponent") {

					pushToSceneGraphWorker(newComponent);
				}
				if (static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6) == "MaterialComponent") {

					tileSettings = TilingSettings();
				}
				return;
			}
		}
		if (static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6) == "MeshComponent") {

			pushToSceneGraphWorker(newComponent);
		}
		if (static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6) == "MaterialComponent") {

			tileSettings = TilingSettings();
		}

		// if the component that is trying to be replaced doesn't exist, add it instead
		AddComponent(newComponent);
	}

	void pushToSceneGraphWorker(Ref<Component> component);

	void ListComponents() const;
	void RemoveAllComponents();

	//
	Matrix4 GetModelMatrix(Ref<Component> camera = nullptr);

	Vec3 GetPositionFromHierarchy(Ref<Component> camera = nullptr);
	
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

	void setParentActor(Actor* parent_) {
		parent = parent_;
	}

	void unparent() {
		parent = nullptr;
	}

	bool isRootActor() const {
		return parent == nullptr || dynamic_cast<Actor*>(parent) == nullptr;
	}
};

