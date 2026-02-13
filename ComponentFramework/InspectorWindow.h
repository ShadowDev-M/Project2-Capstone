#pragma once
#include "SceneGraph.h"
#include "ScriptComponent.h"
#include "AnimatorComponent.h"
class InspectorWindow
{
	// delete the move and copy constructers
	InspectorWindow(const InspectorWindow&) = delete;
	InspectorWindow(InspectorWindow&&) = delete;
	InspectorWindow& operator = (const InspectorWindow&) = delete;
	InspectorWindow& operator = (InspectorWindow&&) = delete;
private:

	// templated struct to handle the state of components for multiple selected actors
	template <typename ComponentTemplate>
	struct ComponentState {
		std::vector<Ref<ComponentTemplate>> components;
		bool allHaveComponent = false;
		bool someHaveComponent = false;
		bool noneHaveComponent = false;

		// default constructer that handles how many selected actors have the same type of component
		ComponentState(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_) {
			for (const auto& pair : selectedActors_) {
				auto component = pair.second->GetComponent<ComponentTemplate>();
				if (component) {
					if constexpr (std::is_same_v<ComponentTemplate, ScriptComponent>) {

						//duplicate copies vector if scriptcomponent. If you want to add more components just change the if condition.
						std::vector<Ref<ComponentTemplate>> componentsToPush = pair.second->GetAllComponent<ComponentTemplate>();
						for (auto& obj : componentsToPush) {
							components.push_back(obj);
						}
					}
					else {
						components.push_back(component);
					}
					//if (pair.second->get)
				}
			}

			allHaveComponent = components.size() == selectedActors_.size();
			someHaveComponent = !components.empty() && components.size() != selectedActors_.size();
			noneHaveComponent = components.empty();
		}

		/// <summary>
		/// checks to see if multiple vec3 values (x,y,z) are equal or not
		/// </summary>
		/// <param name="getter">gets a specific components getter function 
		/// (i.e you would get all the GetPos (position vectors) for all transform components)</param>
		bool HasMixedVec3(Vec3(ComponentTemplate::* getter)() const) const {
			if (components.size() <= 1) {
				return false;
			}

			Vec3 firstValue = (components[0].get()->*getter)();
			for (size_t i = 1; i < components.size(); i++) {
				Vec3 currentValue = (components[i].get()->*getter)();

				// floating point check against each vec3s xyz
				if (fabs(firstValue.x - currentValue.x) > VERY_SMALL ||
					fabs(firstValue.y - currentValue.y) > VERY_SMALL ||
					fabs(firstValue.z - currentValue.z) > VERY_SMALL) {
					return true;
				}
			}
			return false;
		}

		// checks to see if multiple vec4 values are equal or not
		bool HasMixedVec4(Vec4(ComponentTemplate::* getter)() const) const {
			if (components.size() <= 1) {
				return false;
			}

			Vec4 firstValue = (components[0].get()->*getter)();
			for (size_t i = 1; i < components.size(); i++) {
				Vec4 currentValue = (components[i].get()->*getter)();

				// floating point check against each vec3s xyz
				if (fabs(firstValue.x - currentValue.x) > VERY_SMALL ||
					fabs(firstValue.y - currentValue.y) > VERY_SMALL ||
					fabs(firstValue.z - currentValue.z) > VERY_SMALL ||
					fabs(firstValue.w - currentValue.w) > VERY_SMALL) {
					return true;
				}
			}
			return false;
		}


		// checks to see if multiple quaternion values (w ijk) are equal or not
		bool HasMixedQuaternion(Quaternion(ComponentTemplate::* getter)() const) const {
			if (components.size() <= 1) {
				return false;
			}

			Quaternion firstValue = (components[0].get()->*getter)();
			for (size_t i = 1; i < components.size(); i++) {
				Quaternion currentValue = (components[i].get()->*getter)();

				bool sameSign = fabs(firstValue.w - currentValue.w) < VERY_SMALL &&
					fabs(firstValue.ijk.x - currentValue.ijk.x) < VERY_SMALL &&
					fabs(firstValue.ijk.y - currentValue.ijk.y) < VERY_SMALL &&
					fabs(firstValue.ijk.z - currentValue.ijk.z) < VERY_SMALL;

				bool oppositeSign = fabs(firstValue.w + currentValue.w) < VERY_SMALL &&
					fabs(firstValue.ijk.x + currentValue.ijk.x) < VERY_SMALL &&
					fabs(firstValue.ijk.y + currentValue.ijk.y) < VERY_SMALL &&
					fabs(firstValue.ijk.z + currentValue.ijk.z) < VERY_SMALL;

				if (!sameSign && !oppositeSign) {
					return true;
				}
			}
			return false;
		}

		// checks to see if multiple float values are equal or not
		bool HasMixedFloat(float(ComponentTemplate::* getter)() const) const {
			if (components.size() <= 1) {
				return false;
			}

			float firstValue = (components[0].get()->*getter)();
			for (size_t i = 1; i < components.size(); i++) {
				float currentValue = (components[i].get()->*getter)();

				// floating point check against each vec3s xyz
				if (fabs(firstValue - currentValue) > VERY_SMALL) {
					return true;
				}
			}
			return false;
		}

		// applys a components setter function to all selected actors
		template<typename ... Args>
		void ApplyToAll(void(ComponentTemplate::* setter)(Args...), Args... args_) {
			for (auto& component : components) {
				(component.get()->*setter)(args_...);
			}
		}

		// get the first selected actors component type
		Ref<ComponentTemplate> GetFirst() const {
			return (components.empty()) ? nullptr : components[0];
		}

		// returns how many components there are
		size_t Count() const {
			return components.size();
		}
	};

	// pointer to scenegraph
	SceneGraph* sceneGraph;

	// thumbnail size, assetmanager also has this, possibly find a way to make it so they share the same thing incase I edit
	const float thumbnailSize = 64;

	bool scaleLock = false;

	// rename variables
	std::string oldActorName = "";
	std::string newActorName = "";

	// draw transform component variables
	bool isEditingPosition = false;
	Vec3 lastPos;

	bool isEditingRotation = false;
	Vec3 eulerAngles;

	bool isEditingScale = false;
	Vec3 lastScale;

	// light states
	bool isEditingSpec = false;
	bool isEditingDiff = false;
	bool isEditingIntensity = false;

	// physics states
	bool isEditingMass = false;
	bool isEditingDrag = false;
	bool isEditingAngularDrag = false;
	bool isEditingFriction = false;
	bool isEditingRestitution = false;

	// collider states
	bool isEditingRadius = false;
	bool isEditingCentre = false;
	bool isEditingCentrePosA = false;
	bool isEditingCentrePosB = false;
	bool isEditingHalfExtents = false;
	bool isEditingOrientation = false;
	Vec3 lastCentre;
	Vec3 lastCentrePosA;
	Vec3 lastCentrePosB;
	Vec3 lastHalfExtents;
	Vec3 oriEulerAngles;

	// header for renaming, isactive
	void DrawActorHeader(Ref<Actor> actor_);

	// component functions
	void DrawTransformComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawMeshComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawMaterialComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawCameraComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawScriptComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawLightComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawShaderComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawPhysicsComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawCollisionComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);
	void DrawAnimatorComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);

	// right click popup menu
	template <typename ComponentTemplate>
	void RightClickContext(const char* popupName_, const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_);

public:
	explicit InspectorWindow(SceneGraph* sceneGraph_);
	~InspectorWindow() {}

	void ShowInspectorWindow(bool* pOpen);
};

template<typename ComponentTemplate>
inline void InspectorWindow::RightClickContext(const char* popupName_, const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	if (ImGui::BeginPopupContextItem(popupName_)) {
		ComponentState<ComponentTemplate> componentState(selectedActors_);

		if (ImGui::MenuItem("Reset")) {
			if constexpr (std::is_same_v<ComponentTemplate, TransformComponent>) {
				// gets a refernece to the selected actors transform components setter function and applys all given arguments
				componentState.ApplyToAll(&TransformComponent::SetTransform, Vec3(0, 0, 0), Quaternion(1, Vec3(0, 0, 0)), Vec3(1.0f, 1.0f, 1.0f));
			}
			ImGui::Separator();
		}

		if (ImGui::MenuItem("Remove Component")) {
			for (const auto& pair : selectedActors_) {
				if constexpr (std::is_same_v<ComponentTemplate, MeshComponent>) {
					if (pair.second->GetComponent<MeshComponent>()) {
						pair.second->RemoveComponent<MeshComponent>();

					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, MaterialComponent>) {
					if (pair.second->GetComponent<MaterialComponent>()) {
						pair.second->RemoveComponent<MaterialComponent>();
					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, ShaderComponent>) {
					if (pair.second->GetComponent<ShaderComponent>()) {
						pair.second->RemoveComponent<ShaderComponent>();
					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, PhysicsComponent>) {
					if (pair.second->GetComponent<PhysicsComponent>()) {
						pair.second->RemoveComponent<PhysicsComponent>();
					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
					if (pair.second->GetComponent<CameraComponent>()) {
						pair.second->DeleteComponent<CameraComponent>();
					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, ScriptComponent>) {
					if (pair.second->GetComponent<ScriptComponent>()) {
						pair.second->DeleteComponent<ScriptComponent>();
					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, LightComponent>) {
					if (pair.second->GetComponent<LightComponent>()) {
						sceneGraph->RemoveLight(pair.second);
						pair.second->DeleteComponent<LightComponent>();

					}
				}
				if constexpr (std::is_same_v<ComponentTemplate, AnimatorComponent>) {
					if (pair.second->GetComponent<AnimatorComponent>()) {
						pair.second->DeleteComponent<AnimatorComponent>();
					}
				}
			}
		}

		if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {

			// making sure that this only works when 1 actor is selected
			if (componentState.allHaveComponent && selectedActors_.size() == 1)
			{
				if (ImGui::MenuItem("Use Camera")) {
					std::cout << "USING CAMERA BUTTON" << std::endl;
					sceneGraph->setUsedCamera(componentState.GetFirst());
				}
			}

		}



		ImGui::EndPopup();
	}
}