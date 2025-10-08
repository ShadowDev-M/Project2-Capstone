#include "InspectorWindow.h"
#include "InputManager.h"
#include "EditorManager.h"
#include "imgui_stdlib.h"
#include <EMath.h>


InspectorWindow::InspectorWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
	EditorManager::getInstance().RegisterWindow("Inspector", true);
}

void InspectorWindow::ShowInspectorWindow(bool* pOpen)
{
	if (ImGui::Begin("Inspector", pOpen)) {

		// no actors selected
		if (sceneGraph->debugSelectedAssets.empty()) {
			ImGui::Text("No Actor Selected");
		}

		// only 1 actor is selected
		else if (sceneGraph->debugSelectedAssets.size() == 1) {
			auto selectedActor = sceneGraph->debugSelectedAssets.begin();

			DrawActorHeader(selectedActor->second);
			
			ImGui::SameLine();
			ImGui::Text("ID: %i", selectedActor->first);

			ImGui::Separator();

			// slider for increasing stud multiplier (in-scene movement with wasd)
			float sliderMulti = InputManager::getInstance().GetStudMultiplier();
			ImGui::Text("Stud Movement");
			ImGui::SameLine();
			if (ImGui::SliderFloat("##StudSlider", &sliderMulti, 0.0f, 10.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
				InputManager::getInstance().SetStudMultiplier(sliderMulti);
			}
			
			ImGui::Separator();

			/// Components Section
			// TODO: PhysicsComponent, CollisionComponent + CollisionSystem


			// transform
			if (selectedActor->second->GetComponent<TransformComponent>()) {
				DrawTransformComponent(selectedActor->second->GetComponent<TransformComponent>());
				RightClickContext<TransformComponent>("##TransformPopup", selectedActor->second);
				ImGui::Separator();
			}

			// camera
			if (selectedActor->second->GetComponent<CameraComponent>()) {
				DrawCameraComponent(selectedActor->second->GetComponent<CameraComponent>());
				RightClickContext<CameraComponent>("##CameraPopup", selectedActor->second);
				ImGui::Separator();
			}
			
			// light
			if (selectedActor->second->GetComponent<LightComponent>()) {
				DrawLightComponent(selectedActor->second->GetComponent<LightComponent>());
				RightClickContext<LightComponent>("##LightPopup", selectedActor->second);
				ImGui::Separator();
			}

			// mesh
			if (selectedActor->second->GetComponent<MeshComponent>()) {
				DrawMeshComponent(selectedActor->second->GetComponent<MeshComponent>());
				RightClickContext<MeshComponent>("##MeshPopup", selectedActor->second);
				ImGui::Separator();
			}

			// material
			if (selectedActor->second->GetComponent<MaterialComponent>()) {
				DrawMaterialComponent(selectedActor->second->GetComponent<MaterialComponent>());
				RightClickContext<MaterialComponent>("##MaterialPopup", selectedActor->second);
				ImGui::Separator();
			}

			// shader
			if (selectedActor->second->GetComponent<ShaderComponent>()) {
				DrawShaderComponent(selectedActor->second->GetComponent<ShaderComponent>());
				RightClickContext<ShaderComponent>("##ShaderPopup", selectedActor->second);
				ImGui::Separator();
			}

			if (ImGui::Button("Add Component##Button", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Add Component");
			}

			// sets the placement and size of the dialog box
			const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 800, mainViewport->WorkPos.y - 600), ImGuiCond_Appearing);
			ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

			if (ImGui::BeginPopupModal("Add Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Select A Component");
				// TODO search bar
				ImGui::Separator();

				if (ImGui::Selectable("Mesh Component")) {
					if (!selectedActor->second->GetComponent<MeshComponent>()) {
						selectedActor->second->AddComponent<MeshComponent>(nullptr, "");
					}
				}

				if (ImGui::Selectable("Material Component")) {
					if (!selectedActor->second->GetComponent<MaterialComponent>()) {
						selectedActor->second->AddComponent<MaterialComponent>(nullptr, "");
					}
				}

				if (ImGui::Selectable("Shader Component")) {
					if (!selectedActor->second->GetComponent<ShaderComponent>()) {
						selectedActor->second->AddComponent<ShaderComponent>(nullptr, "", "");
					}
				}

				if (ImGui::Selectable("Camera Component")) {
					if (!selectedActor->second->GetComponent<CameraComponent>()) {
						selectedActor->second->AddComponent<CameraComponent>(selectedActor->second, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
					}
				}

				if (ImGui::Selectable("Light Component")) {
					if (!selectedActor->second->GetComponent<LightComponent>()) {
						selectedActor->second->AddComponent<LightComponent>(nullptr, LightType::Point, Vec4(1.0f, 1.0f, 1.0f, 1.0f), Vec4(0.5f, 0.5f, 0.5f, 1.0f), 200.0f);
						sceneGraph->AddLight(selectedActor->second);
						
					}
				}

				ImGui::Separator();

				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

		}

		// TODO: if an asset is selected in the assetbrowser it'll display some information about it in the inspector window


		// more than 1 actor selected
		else {
			// TODO: multi-actor editing

			for (const auto& pair : sceneGraph->debugSelectedAssets) {
				
				

			}

			

		}
	}

	ImGui::End();
}

void InspectorWindow::DrawActorHeader(Ref<Actor> actor_)
{
	if (oldActorName != actor_->getActorName()) {
		oldActorName = actor_->getActorName();
		newActorName = oldActorName;
	}

	ImGui::InputText("##ActorHeader", &newActorName);

	// passes the rename inforamtion over to the editor manager (editor manager then passes it to the hierarchy window) 
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		if (!newActorName.empty() && newActorName != oldActorName) {
			EditorManager::getInstance().RequestActorRename(oldActorName, newActorName);
			oldActorName = newActorName;
		}
	}

}

void InspectorWindow::DrawTransformComponent(Ref<TransformComponent> transform)
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Setting up right click popup menu, context sensitive to the header
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##TransformPopup");
		}

		// Position
		Vec3 pos = transform->GetPosition();
		float position[3] = { pos.x, pos.y, pos.z };

		ImGui::Text("Position");
		ImGui::SameLine();
		if (ImGui::DragFloat3("##Position", position, 0.1f)) {
			transform->SetPos(position[0], position[1], position[2]);
			
		}
		ImGui::ActiveItemLockMousePos();

		//Rotation
		if (!isEditingRotation) {
			// get the quaternions rotation when not edting anything
			if (!ImGui::IsAnyItemActive()) {
				Euler quatEuler = EMath::toEuler(transform->GetQuaternion());
				eulerAngles = Vec3(quatEuler.xAxis, quatEuler.yAxis, quatEuler.zAxis);
			}
		}

		float rotation[3] = { eulerAngles.x, eulerAngles.y, eulerAngles.z };

		ImGui::Text("Rotation");
		ImGui::SameLine();

		bool rotationChanged = ImGui::DragFloat3("##Rotation", rotation, 1.0f);

		isEditingRotation = ImGui::IsItemActive();

		if (rotationChanged) {
			// store the euler angles
			eulerAngles.x = rotation[0];
			eulerAngles.y = rotation[1];
			eulerAngles.z = rotation[2];

			// convert euler to quaternion
			Euler eulerRot(rotation[0], rotation[1], rotation[2]);
			Quaternion newOrientation = QMath::toQuaternion(eulerRot);
			transform->SetOrientation(QMath::normalize(newOrientation));
		}
		ImGui::ActiveItemLockMousePos();

		// Scale
		Vec3 scaleVector = transform->GetScale();
		float scale[3] = { scaleVector.x, scaleVector.y, scaleVector.z };
		Vec3 lastScale = scaleVector;

		ImGui::Text("Scale");
		ImGui::SameLine();
		if (ImGui::Checkbox("##Lock", &scaleLock));
		ImGui::SameLine();
		if (ImGui::DragFloat3("##Scale", scale, 0.1f, 0.1f, 100.0f)) {
			if (scaleLock) {
				float uniformScale = scale[0];

				if (scale[0] != lastScale.x)
				{
					uniformScale = scale[0];
				}
				else if (scale[1] != lastScale.y)
				{
					uniformScale = scale[1];
				}
				else if (scale[2] != lastScale.z)
				{
					uniformScale = scale[2];
				}

				scale[0] = scale[1] = scale[2] = uniformScale;
				transform->SetTransform(Vec3(uniformScale, uniformScale, uniformScale));
				lastScale = Vec3(uniformScale, uniformScale, uniformScale);
			}
			else {
				transform->SetTransform(Vec3(scale[0], scale[1], scale[2]));
				lastScale = Vec3(scale[0], scale[1], scale[2]);
			}
			
		}
		ImGui::ActiveItemLockMousePos();
	}
}

void InspectorWindow::DrawMeshComponent(Ref<MeshComponent> mesh)
{
	if (ImGui::CollapsingHeader("Mesh")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##MeshPopup");
		}

		// displaying some basic mesh information
		ImGui::TextWrapped("Mesh Name: %s", mesh->getMeshName());
		ImGui::TextWrapped("Mesh Vertices: %zu", mesh->getVertices());


		ImGui::Button("Drop New Mesh Here", ImVec2(-1, 0));
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_ASSET");

			if (payload) {
				// store payload data
				const char* droppedAssetName = static_cast<const char*>(payload->Data);
				
				// get a reference to the asset that has been dropped
				Ref<MeshComponent> newMesh = AssetManager::getInstance().GetAsset<MeshComponent>(droppedAssetName);

				// get the actor
				if (newMesh) {
					for (const auto& pair : sceneGraph->debugSelectedAssets) {
						// replace the actors material
						pair.second->ReplaceComponent<MeshComponent>(newMesh);
					}
				}
			}

			ImGui::EndDragDropTarget();
		}


		// alternatively, a drop dowm menu that has a list of all meshes

	}
}

void InspectorWindow::DrawMaterialComponent(Ref<MaterialComponent> material)
{
	if (ImGui::CollapsingHeader("Material")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##MaterialPopup");
		}
		ImGui::TextWrapped("Texture ID: %u", material->getTextureID());

		// display material thumbnail
		if (material->getTextureID() != 0) {
			ImGui::ImageButton("##MaterialThumbnail", ImTextureID(material->getTextureID()),
				ImVec2(thumbnailSize, thumbnailSize));
		}
		else {
			ImGui::Button("Material ##Button", ImVec2(thumbnailSize, thumbnailSize));
		}

		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_ASSET");
			if (payload) {
				// store payload data
				const char* droppedAssetName = static_cast<const char*>(payload->Data);
				
				// get a reference to the asset that has been dropped
				Ref<MaterialComponent> newMaterial = AssetManager::getInstance().GetAsset<MaterialComponent>(droppedAssetName);
				
				// get the actor
				if (newMaterial) {
					for (const auto& pair : sceneGraph->debugSelectedAssets) {
						// replace the actors material
						pair.second->ReplaceComponent<MaterialComponent>(newMaterial);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

	}
}

void InspectorWindow::DrawCameraComponent(Ref<CameraComponent> camera)
{
	if (ImGui::CollapsingHeader("Camera")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##CameraPopup");
		}
		
		float fov = camera->getFOV(), aspectRatio = camera->getAspectRatio(), 
			nearClipPlane = camera->getNearClipPlane(), farClipPlane = camera->getFarClipPlane();

		int fovInt = static_cast<int>(fov);

		ImGui::Text("Fov ");
		ImGui::SameLine();
		if (ImGui::SliderInt("##fovslider", &fovInt, 0, 120, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			camera->setFOV(fovInt);
		}

		ImGui::Text("Clipping Planes");
		
		ImGui::Text("Near");
		ImGui::SameLine();
		if (ImGui::DragFloat("##NearDrag", &nearClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			camera->setNearClipPlane(nearClipPlane);
		}

		ImGui::Text("Far ");
		ImGui::SameLine();
		if (ImGui::DragFloat("##FarDrag", &farClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			camera->setFarClipPlane(farClipPlane);
		}

	}
	
}

void InspectorWindow::DrawLightComponent(Ref<LightComponent> light)
{
	if (ImGui::CollapsingHeader("Light")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##LightPopup");
		}
		ImGui::TextWrapped("Light ID: %u", light.get());

		// display material thumbnail
		/*if (material->getTextureID() != 0) {
			ImGui::ImageButton("Drop New Asset Here ##Material", ImTextureID(material->getTextureID()), ImVec2(thumbnailSize, thumbnailSize));
		}*/




	}
}

void InspectorWindow::DrawShaderComponent(Ref<ShaderComponent> shader)
{
	if (ImGui::CollapsingHeader("Shader")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##ShaderPopup");
		}
		// displaying some basic shader information
		ImGui::TextWrapped("Shader Program ID: %u", shader->GetProgram());
		ImGui::TextWrapped("Shader Vert: %s", shader->GetVertName());
		ImGui::TextWrapped("Shader Frag: %s", shader->GetFragName());


		ImGui::Button("Drop New Shader Here");
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_ASSET");
			if (payload) {
				// store the payload data
				const char* droppedAssetName = static_cast<const char*>(payload->Data);
				
				// get a reference to the asset that has been dropped
				Ref<ShaderComponent> newShader = AssetManager::getInstance().GetAsset<ShaderComponent>(droppedAssetName);
				
				// get the actor
				if (newShader) {
					for (const auto& pair : sceneGraph->debugSelectedAssets) {
						// replace the actors shader
						pair.second->ReplaceComponent<ShaderComponent>(newShader);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

	}
}

