#pragma once
#include "imgui.h"
#include "SceneGraph.h"

class InspectorWindow
{
	// delete the move and copy constructers
	InspectorWindow(const InspectorWindow&) = delete;
	InspectorWindow(InspectorWindow&&) = delete;
	InspectorWindow& operator = (const InspectorWindow&) = delete;
	InspectorWindow& operator = (InspectorWindow&&) = delete;
private:
	// pointer to scenegraph
	SceneGraph* sceneGraph;

	// thumbnail size, assetmanager also has this, possibly find a way to make it so they share the same thing incase I edit
	const int thumbnailSize = 64;

	bool scaleLock = false;

	std::string actorName = "";

	// header for renaming, isactive
	void DrawActorHeader(Ref<Actor> actor_);

	// component functions
	void DrawTransformComponent(Ref<TransformComponent> transform);
	void DrawMeshComponent(Ref<MeshComponent> mesh);
	void DrawMaterialComponent(Ref<MaterialComponent> material);

	void DrawCameraComponent(Ref<CameraComponent> camera);

	void DrawShaderComponent(Ref<ShaderComponent> shader);

	// right click popup menu
	template <typename ComponentTemplate>
	void RightClickContext(const char* popupName_, Ref<Actor> sceneActor_);

public:
	explicit InspectorWindow(SceneGraph* sceneGraph_);
	~InspectorWindow() {}

	void ShowInspectorWindow(bool* pOpen);
};

template<typename ComponentTemplate>
inline void InspectorWindow::RightClickContext(const char * popupName_, Ref<Actor> sceneActor_)
{
	if (ImGui::BeginPopupContextItem(popupName_)) {
		if (ImGui::MenuItem("Reset")) {
			if constexpr (std::is_same_v<ComponentTemplate, TransformComponent>) {
				sceneActor_->GetComponent<TransformComponent>()->SetTransform(Vec3(0, 0, 0), Quaternion(1, Vec3(0, 0, 0)), Vec3(1.0f, 1.0f, 1.0f));
			}
			ImGui::Separator();
		}

		if (ImGui::MenuItem("Remove Component")) {
			if constexpr (std::is_same_v<ComponentTemplate, MeshComponent>) {
				sceneActor_->RemoveComponent<MeshComponent>();
			}
			if constexpr (std::is_same_v<ComponentTemplate, MaterialComponent>) {
				sceneActor_->RemoveComponent<MaterialComponent>();
			}
			if constexpr (std::is_same_v<ComponentTemplate, ShaderComponent>) {
				sceneActor_->RemoveComponent<ShaderComponent>();
			}
			if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
				sceneActor_->DeleteComponent<CameraComponent>();
			}

		}
	
		if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
			if (ImGui::MenuItem("Use Camera")) {
				std::cout << "USING CAMERA BUTTON" << std::endl;
				sceneGraph->setUsedCamera(sceneActor_->GetComponent<CameraComponent>());
			}
		}



		ImGui::EndPopup();
	}
}