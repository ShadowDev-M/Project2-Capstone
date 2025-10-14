#include "InspectorWindow.h"
#include "InputManager.h"
#include "imgui_stdlib.h"
#include <EMath.h>

InspectorWindow::InspectorWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {}

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

			if (ImGui::Button("Add Component##Button")) {
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
						selectedActor->second->AddComponent<LightComponent>(nullptr, LightType::Point, Vec4(1.0f, 1.0f, 1.0f, 1.0f), Vec4(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
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

			if (ImGui::CollapsingHeader("Selected Actors", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (const auto& pair : sceneGraph->debugSelectedAssets) {
					ImGui::Text("%s", pair.first.c_str());
					
				}
			}
		}
	}

	ImGui::End();
}

void InspectorWindow::DrawActorHeader(Ref<Actor> actor_)
{
	actorName = actor_->getActorName();

	ImGui::InputText("##ActorHeader", &actorName);

	if (ImGui::IsItemDeactivatedAfterEdit()) {

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

		//Rotation
		//TODO: fix gimbal lock, advanced rotation
		// possibly use orientation

		Euler quatEuler = EMath::toEuler(transform->GetQuaternion());
		float rotation[3] = { quatEuler.xAxis, quatEuler.yAxis, quatEuler.zAxis };

		ImGui::Text("Rotation");
		ImGui::SameLine();

		if (ImGui::DragFloat3("##Rotation", rotation, 1.0f)) {
			Quaternion rotX = QMath::angleAxisRotation(rotation[0], Vec3(1.0f, 0.0f, 0.0f));
			Quaternion rotY = QMath::angleAxisRotation(rotation[1], Vec3(0.0f, 1.0f, 0.0f));
			Quaternion rotZ = QMath::angleAxisRotation(rotation[2], Vec3(0.0f, 0.0f, 1.0f));

			// euler
			Quaternion finalRotation = rotZ * rotY * rotX;
			transform->SetOrientation(QMath::normalize(finalRotation));
		}


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


		ImGui::Button("Drop New Mesh Here");
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_ASSET")) {
				// store payload data		
				const char* droppedAsset = (const char*)payload->Data;

				// get a reference to the asset that has been dropped
				Ref<MeshComponent> newMesh = AssetManager::getInstance().GetAsset<MeshComponent>(droppedAsset);


				// an extra check to make sure the new mesh is valid and that there is an actor selected
				//if (newMesh && !sceneGraph.debugSelectedAssets.empty())

				// get the actor
				for (const auto& pair : sceneGraph->debugSelectedAssets) {
					Ref<Actor> actor = pair.second;

					// replace the actors mesh
					actor->ReplaceComponent<MeshComponent>(newMesh);
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
			ImGui::ImageButton("Drop New Asset Here ##Material", ImTextureID(material->getTextureID()), ImVec2(thumbnailSize, thumbnailSize));
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_ASSET")) {
				// store payload data
				const char* droppedAsset = (const char*)payload->Data;

				// get a reference to the asset that has been dropped
				Ref<MaterialComponent> newMaterial = AssetManager::getInstance().GetAsset<MaterialComponent>(droppedAsset);

				// get the actor
				for (const auto& pair : sceneGraph->debugSelectedAssets) {
					Ref<Actor> actor = pair.second;

					// replace the actors material
					actor->ReplaceComponent<MaterialComponent>(newMaterial);
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
		ImGui::TextWrapped("Camera ID: %u", camera.get());

		// display material thumbnail
		/*if (material->getTextureID() != 0) {
			ImGui::ImageButton("Drop New Asset Here ##Material", ImTextureID(material->getTextureID()), ImVec2(thumbnailSize, thumbnailSize));
		}*/

		


	}
}

void InspectorWindow::DrawLightComponent(Ref<LightComponent> light)
{
	if (ImGui::CollapsingHeader("Light")) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("##LightPopup");
		}
		ImGui::TextWrapped("Light ID: %u", light.get());
		Vec4 spec = light->getSpec();
		float specular[3] = { spec.x, spec.y, spec.z };

		ImGui::Text("Specular");
		ImGui::SameLine();
		if (ImGui::DragFloat3("##Specular", specular, 0.01f)) {
			for (int i = 0; i < 3; ++i) {
				if (specular[i] < 0) {
					specular[i] = 0;
				}
				else if (specular[i] > 1) {
					specular[i] = 1;
				}
			}
			light->setSpec(Vec4(specular[0], specular[1], specular[2], spec.w));
		}

		Vec4 diff = light->getDiff();
		float diffuse[3] = { diff.x, diff.y, diff.z };

		ImGui::Text("Diffuse");
		ImGui::SameLine();
		if (ImGui::DragFloat3("##Diffuse", diffuse, 0.01f)) {
			for (int i = 0; i < 3; ++i) {
				if (diffuse[i] < 0) {
					diffuse[i] = 0;
				}
				else if (diffuse[i] > 1) {
					diffuse[i] = 1;
				}
			}
			light->setDiff(Vec4(diffuse[0], diffuse[1], diffuse[2], diff.w));
		}

		float intensity = light->getIntensity();

		ImGui::Text("Intensity");
		ImGui::SameLine();
		if (ImGui::DragFloat("##Intensity", &intensity, 0.1f)) {
			
			if (intensity < 0) {
				intensity = 0;
			}
			light->setIntensity(intensity);
		}

		if (ImGui::Button("Light Type##Button")) {
			ImGui::OpenPopup("Light Type");
		}

		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 800, mainViewport->WorkPos.y - 600), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

		if (ImGui::BeginPopupModal("Light Type", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Select a Light Type");
			// TODO search bar
			ImGui::Separator();

			if (ImGui::Selectable("Sky Light")) {
				light->setType(LightType::Sky);
			}
			if (ImGui::Selectable("Point Light")) {
				light->setType(LightType::Point);
			}

			ImGui::Separator();

			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
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


		ImGui::Button("Drop New Asset Here ##Shader");
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_ASSET")) {
				// store the payload data
				const char* droppedAsset = (const char*)payload->Data;

				// get a reference to the asset that has been dropped
				Ref<ShaderComponent> newShader = AssetManager::getInstance().GetAsset<ShaderComponent>(droppedAsset);

				// get the actor
				for (const auto& pair : sceneGraph->debugSelectedAssets) {
					Ref<Actor> actor = pair.second;

					// replace the actors shader
					actor->ReplaceComponent<ShaderComponent>(newShader);
				}
			}
			ImGui::EndDragDropTarget();
		}



	}
}

