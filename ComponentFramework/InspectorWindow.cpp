#include "pch.h"
#include "InspectorWindow.h"
#include "InputManager.h"
#include "EditorManager.h"

//#include "ScriptComponent.h"


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

			/// Components Section
			// TODO: CollisionComponent + CollisionSystem


			// transform
			if (selectedActor->second->GetComponent<TransformComponent>()) {
				DrawTransformComponent(sceneGraph->debugSelectedAssets);
			}

			// mesh
			if (selectedActor->second->GetComponent<MeshComponent>()) {
				DrawMeshComponent(sceneGraph->debugSelectedAssets);
			}

			// material
			if (selectedActor->second->GetComponent<MaterialComponent>()) {
				DrawMaterialComponent(sceneGraph->debugSelectedAssets);
			}

			// shader
			if (selectedActor->second->GetComponent<ShaderComponent>()) {
				DrawShaderComponent(sceneGraph->debugSelectedAssets);
			}

			// physics (rigidbody)
			if (selectedActor->second->GetComponent<PhysicsComponent>()) {
				DrawPhysicsComponent(sceneGraph->debugSelectedAssets);
			}

			// camera
			if (selectedActor->second->GetComponent<CameraComponent>()) {
				DrawCameraComponent(sceneGraph->debugSelectedAssets);
			}
			
			// script
			if (selectedActor->second->GetComponent<ScriptComponent>()) {
				DrawScriptComponent(sceneGraph->debugSelectedAssets);
			}

			// light
			if (selectedActor->second->GetComponent<LightComponent>()) {
				DrawLightComponent(sceneGraph->debugSelectedAssets);
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
						selectedActor->second->AddComponent<MaterialComponent>(nullptr, "", "");
					}
				}

				if (ImGui::Selectable("Shader Component")) {
					if (!selectedActor->second->GetComponent<ShaderComponent>()) {
						selectedActor->second->AddComponent<ShaderComponent>(nullptr, "", "");
					}
				}

				if (ImGui::Selectable("Physics Component")) {
					if (!selectedActor->second->GetComponent<PhysicsComponent>()) {
						selectedActor->second->AddComponent<PhysicsComponent>();
					}
				}

				if (ImGui::Selectable("Camera Component")) {
					if (!selectedActor->second->GetComponent<CameraComponent>()) {
						selectedActor->second->AddComponent<CameraComponent>(selectedActor->second, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
					}
				}
				if (ImGui::Selectable("Script Component")) {
				//	if (!selectedActor->second->GetComponent<ScriptComponent>()) {
						selectedActor->second->AddComponent<ScriptComponent>(selectedActor->second.get());
					//}
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
			
			// Header
			if (ImGui::CollapsingHeader("Selected Actors", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (const auto& pair : sceneGraph->debugSelectedAssets) {
					ImGui::Text("%s", pair.second->getActorName().c_str());
				}
			}
			ImGui::Separator();

			DrawTransformComponent(sceneGraph->debugSelectedAssets);
			DrawMeshComponent(sceneGraph->debugSelectedAssets);
			DrawMaterialComponent(sceneGraph->debugSelectedAssets);
			DrawShaderComponent(sceneGraph->debugSelectedAssets);
			DrawPhysicsComponent(sceneGraph->debugSelectedAssets);
			DrawScriptComponent(sceneGraph->debugSelectedAssets);
			DrawCameraComponent(sceneGraph->debugSelectedAssets);
			DrawLightComponent(sceneGraph->debugSelectedAssets);

			if (ImGui::Button("Add Component##Button", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Add Component");
			}

			// sets the placement and size of the dialog box
			const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 800, mainViewport->WorkPos.y - 600), ImGuiCond_Appearing);
			ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

			if (ImGui::BeginPopupModal("Add Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Select A Component");
				ImGui::Separator();

				ComponentState<MeshComponent> meshState(sceneGraph->debugSelectedAssets);
				ComponentState<MaterialComponent> materialState(sceneGraph->debugSelectedAssets);
				ComponentState<ShaderComponent> shaderState(sceneGraph->debugSelectedAssets);
				ComponentState<PhysicsComponent> physicsState(sceneGraph->debugSelectedAssets);
				ComponentState<CameraComponent> cameraState(sceneGraph->debugSelectedAssets);
				ComponentState<ScriptComponent> scriptState(sceneGraph->debugSelectedAssets);
				ComponentState<LightComponent> lightState(sceneGraph->debugSelectedAssets);

				if (meshState.noneHaveComponent || meshState.someHaveComponent) {
					if (ImGui::Selectable("Mesh Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<MeshComponent>()) {
								pair.second->AddComponent<MeshComponent>(nullptr, "");
							}
						}
					}
				}

				if (materialState.noneHaveComponent || materialState.someHaveComponent) {
					if (ImGui::Selectable("Material Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<MaterialComponent>()) {
								pair.second->AddComponent<MaterialComponent>(nullptr, "", "");
							}
						}
					}
				}

				if (shaderState.noneHaveComponent || shaderState.someHaveComponent) {
					if (ImGui::Selectable("Shader Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<ShaderComponent>()) {
								pair.second->AddComponent<ShaderComponent>(nullptr, "", "");
							}
						}
					}
				}

				if (physicsState.noneHaveComponent || physicsState.someHaveComponent) {
					if (ImGui::Selectable("Physics Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<PhysicsComponent>()) {
								pair.second->AddComponent<PhysicsComponent>();
							}
						}
					}
				}

				if (cameraState.noneHaveComponent || cameraState.someHaveComponent) {
					if (ImGui::Selectable("Camera Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<CameraComponent>()) {
								pair.second->AddComponent<CameraComponent>(pair.second);
							}
						}
					}
				}
				
				if (scriptState.noneHaveComponent || scriptState.someHaveComponent) {
					if (ImGui::Selectable("Script Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<ScriptComponent>()) {
								pair.second->AddComponent<ScriptComponent>(nullptr);
							}
						}
					}
				}


				if (lightState.noneHaveComponent || lightState.someHaveComponent) {
					if (ImGui::Selectable("Light Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<LightComponent>()) {
								pair.second->AddComponent<LightComponent>(nullptr, LightType::Point, Vec4(1.0f, 1.0f, 1.0f, 1.0f), Vec4(0.5f, 0.5f, 0.5f, 1.0f), 200.0f);
							}
						}
					}
				}

				ImGui::Separator();

				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
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

void InspectorWindow::DrawTransformComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	// component state for all selected actors transform 
	ComponentState<TransformComponent> transformState(selectedActors_);
	
	if (transformState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Setting up right click popup menu, context sensitive to the header
		RightClickContext<TransformComponent>("##TransformPopup", sceneGraph->debugSelectedAssets);

		// Position
		bool hasMixedPos = transformState.HasMixedVec3(&TransformComponent::GetPosition);
		
		Vec3 displayPos = transformState.GetFirst()->GetPosition();
		lastPos = displayPos;

		float position[3] = { displayPos.x, displayPos.y, displayPos.z };

		ImGui::Text("Position ");
		ImGui::SameLine();

		if (hasMixedPos && !isEditingPosition) {
			ImGui::DragFloat3("##Position", position, 0.1f, 0.0f, 0.0f, "---");
		}
		else {
			if (ImGui::DragFloat3("##Position", position, 0.1f)) {
				Vec3 newPos(position[0], position[1], position[2]);
				Vec3 delta = newPos - lastPos;

				for (auto& component : transformState.components) {
					Vec3 currentPos = component->GetPosition();
					Vec3 updatedPos = currentPos + delta;
					component->SetPos(updatedPos.x, updatedPos.y, updatedPos.z);
				}

				lastPos = newPos;
			}
		}
			
		isEditingPosition = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		//Rotation
		bool hasMixedRot = transformState.HasMixedQuaternion(&TransformComponent::GetQuaternion);

		if (!isEditingRotation) {
			// get the quaternions rotation when not edting anything
			if (!ImGui::IsAnyItemActive()) {
				Quaternion firstQuat = transformState.GetFirst()->GetQuaternion();
				Euler quatEuler = EMath::toEuler(firstQuat);
				eulerAngles = Vec3(quatEuler.xAxis, quatEuler.yAxis, quatEuler.zAxis);
			}
		}

		float rotation[3] = { eulerAngles.x, eulerAngles.y, eulerAngles.z };

		ImGui::Text("Rotation ");
		ImGui::SameLine();

		bool rotationChanged = false;

		if (hasMixedRot && !isEditingRotation) {
			ImGui::DragFloat3("##Rotation", rotation, 0.1f, 0.0f, 0.0f, "---");
		}
		else {
			rotationChanged = ImGui::DragFloat3("##Rotation", rotation, 1.0f);
		}

		if (rotationChanged) {
			// store the euler angles
			eulerAngles.x = rotation[0];
			eulerAngles.y = rotation[1];
			eulerAngles.z = rotation[2];

			// convert euler to quaternion
			Euler eulerRot(rotation[0], rotation[1], rotation[2]);
			Quaternion newOrientation = QMath::toQuaternion(eulerRot);

			if (transformState.components.size() == 1) {
				transformState.GetFirst()->SetOrientation(QMath::normalize(newOrientation));
			}
			else {
				Quaternion firstQuat = transformState.GetFirst()->GetQuaternion();
				Quaternion delta = newOrientation * QMath::inverse(firstQuat);

				for (auto& component : transformState.components) {
					Quaternion currentQuat = component->GetQuaternion();
					Quaternion updatedQuat = delta * currentQuat;
					component->SetOrientation(QMath::normalize(updatedQuat));
				}
			}
			
		}

		isEditingRotation = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();
		
		// Scale
		bool hasMixedScale = transformState.HasMixedVec3(&TransformComponent::GetScale);

		Vec3 displayScale = transformState.GetFirst()->GetScale();
		lastScale = displayScale;

		float scale[3] = { displayScale.x, displayScale.y, displayScale.z };

		ImGui::Text("Scale");
		ImGui::SameLine();
		/*if*/ (ImGui::Checkbox("##Lock", &scaleLock));
		ImGui::SameLine();

		bool scaleChanged = false;

		if (hasMixedScale && !isEditingScale) {
			ImGui::DragFloat3("##Scale", scale, 0.1f, 0.1f, 100.0f, "---");
		}
		else {
			scaleChanged = ImGui::DragFloat3("##Scale", scale, 0.1f, 0.1f, 100.0f);
		}

		if (scaleChanged) {
			if (scaleLock) {
				float uniformDelta = scale[0] - lastScale.x;

				if (fabs(scale[0] - lastScale.x) > VERY_SMALL) uniformDelta = scale[0] - lastScale.x;
				else if (fabs(scale[1] - lastScale.y) > VERY_SMALL) uniformDelta = scale[1] - lastScale.y;
				else if (fabs(scale[2] - lastScale.z) > VERY_SMALL) uniformDelta = scale[2] - lastScale.z;

				for (auto& component : transformState.components) {
					Vec3 currentScale = component->GetScale();
					float updatedUniformScale = currentScale.x + uniformDelta; 
					component->SetTransform(Vec3(updatedUniformScale, updatedUniformScale, updatedUniformScale));
				}

				lastScale = lastScale + Vec3(uniformDelta, uniformDelta, uniformDelta);
				scale[0] = scale[1] = scale[2] = lastScale.x;

			}
			else {
				Vec3 delta(scale[0] - lastScale.x, scale[1] - lastScale.y, scale[2] - lastScale.z);

				for (auto& component : transformState.components) {
					Vec3 currentScale = component->GetScale();
					component->SetTransform(Vec3(currentScale.x + delta.x, currentScale.y + delta.y, currentScale.z + delta.z));
				}

				lastScale = Vec3(scale[0], scale[1], scale[2]);
			}
		}

		isEditingScale = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();
	}
	ImGui::Separator();
}

void InspectorWindow::DrawMeshComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<MeshComponent> meshState(selectedActors_);

	if (meshState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Mesh"), ImGuiTreeNodeFlags_DefaultOpen) {
		RightClickContext<MeshComponent>("##MeshPopup", sceneGraph->debugSelectedAssets);


		// one actor selected
		if (meshState.allHaveComponent && meshState.Count() == 1) {
			Ref<MeshComponent> mesh = meshState.GetFirst();
		
			// displaying some basic mesh information
			ImGui::TextWrapped("Mesh Name: %s", mesh->getMeshName());
			ImGui::TextWrapped("Mesh Vertices: %zu", mesh->getVertices());
		}
		else if (meshState.allHaveComponent) {
			//for (auto& component : meshState.components) {
				ImGui::TextWrapped("All Selected Actors Have a MeshComponent");
			//}
		}
		else if (meshState.someHaveComponent) {
			ImGui::TextWrapped("Some Selected Actors Have a MeshComponent");
		}

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
					for (auto& component : meshState.components) {
						for (const auto& pair : selectedActors_) {
							// replace the actors material
							if (pair.second->GetComponent<MeshComponent>() == component) {
								pair.second->ReplaceComponent<MeshComponent>(newMesh);
							}
						}
					}
				}
			}

			ImGui::EndDragDropTarget();
		}


		// alternatively, a drop dowm menu that has a list of all meshes

	}

	ImGui::Separator();
}

void InspectorWindow::DrawMaterialComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<MaterialComponent> materialState(selectedActors_);

	if (materialState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Material"), ImGuiTreeNodeFlags_DefaultOpen) {
		RightClickContext<MaterialComponent>("##MaterialPopup", sceneGraph->debugSelectedAssets);

		if (materialState.allHaveComponent && materialState.Count() == 1) {
			Ref<MaterialComponent> material = materialState.GetFirst();

			ImGui::TextWrapped("Diffuse ID: %u", material->getDiffuseID());
			if (material->getSpecularID() != 0) ImGui::TextWrapped("Specular ID: %u", material->getSpecularID());
		}
		else if (materialState.allHaveComponent) {
			ImGui::TextWrapped("All Selected Actors Have a MaterialComponent");
		}
		else if (materialState.someHaveComponent) {
			ImGui::TextWrapped("Some Selected Actors Have a MaterialComponent");
		}

		// display material thumbnail
		Ref<MaterialComponent> material = materialState.GetFirst();
		if (material && material->getDiffuseID() != 0) {
			if (materialState.Count() == 1) {
				// disable background for buttons
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
				
				ImGui::ImageButton("##MaterialThumbnail", ImTextureID(material->getDiffuseID()),
					ImVec2(thumbnailSize, thumbnailSize));
				
				// pop
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
			else {
				ImGui::Button("Multiple ##Materials", ImVec2(thumbnailSize, thumbnailSize));
			}
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
					for (auto& component : materialState.components) {
						for (const auto& pair : selectedActors_) {
							// replace the actors material
							if (pair.second->GetComponent<MaterialComponent>() == component) {
								pair.second->ReplaceComponent<MaterialComponent>(newMaterial);
							}
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

	}
	ImGui::Separator();
}

void InspectorWindow::DrawCameraComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<CameraComponent> cameraState(selectedActors_);

	if (cameraState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Camera")) {
		RightClickContext<CameraComponent>("##CameraPopup", selectedActors_);
		
		Ref<CameraComponent> camera = cameraState.GetFirst();
		float fov = camera->getFOV(), aspectRatio = camera->getAspectRatio(), 
			nearClipPlane = camera->getNearClipPlane(), farClipPlane = camera->getFarClipPlane();

		int fovInt = static_cast<int>(fov);

		bool hasMixedFOV = cameraState.HasMixedFloat(&CameraComponent::getFOV);
		bool hasMixedNear = cameraState.HasMixedFloat(&CameraComponent::getNearClipPlane);
		bool hasMixedFar = cameraState.HasMixedFloat(&CameraComponent::getFarClipPlane);

		ImGui::Text("Fov ");
		ImGui::SameLine();

		/*if (hasMixedFOV) {
			ImGui::SliderInt("##fovslider", &fovInt, 0, 120, "---", ImGuiSliderFlags_AlwaysClamp);
		}*/
		//else {
			if (ImGui::SliderInt("##fovslider", &fovInt, 0, 120, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
				for (auto& component : cameraState.components) {
					component->setFOV((float)fovInt);
				}
			}
		//}

		ImGui::Text("Clipping Planes");
		
		ImGui::Text("Near");
		ImGui::SameLine();
		if (ImGui::DragFloat("##NearDrag", &nearClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			for (auto& component : cameraState.components) {
				component->setNearClipPlane(nearClipPlane);
			}
		}

		ImGui::Text("Far ");
		ImGui::SameLine();
		if (ImGui::DragFloat("##FarDrag", &farClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			for (auto& component : cameraState.components) {
				component->setFarClipPlane(farClipPlane);
			}
		}

	}
	ImGui::Separator();
}

void InspectorWindow::DrawScriptComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<ScriptComponent> scriptState(selectedActors_);

	if (scriptState.noneHaveComponent) return;
	
	int id = 0;

	if (selectedActors_.size() == 1) {

		for (auto& script : selectedActors_.begin()->second->GetAllComponent<ScriptComponent>()) {

			

			ImGui::PushID(id);
			id++;

			if (ImGui::CollapsingHeader("Script"), ImGuiTreeNodeFlags_DefaultOpen) {
				RightClickContext<ScriptComponent>("##ScriptPopup", sceneGraph->debugSelectedAssets);

				ImGui::TextWrapped("Script Name: %s", script->getName().c_str());

				ImGui::TextWrapped("Script Path: %s", script->getPath().c_str());
			

				ImGui::Button("Drop New Script Here", ImVec2(-1, 0));
				if (ImGui::BeginDragDropTarget()) {
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT_ASSET");
					if (payload) {
						// store the payload data
						const char* droppedAssetName = static_cast<const char*>(payload->Data);

						// get a reference to the asset that has been dropped
						//ScriptAbstract is the asset in assetmanager that has the name, the actors have the component
						Ref<ScriptAbstract> newScript = AssetManager::getInstance().GetAsset<ScriptAbstract>(droppedAssetName);

						
						script->setFilenameFromAbstract(newScript);
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::Separator();

			ImGui::PopID(); // Pop the ID from the stack
		}
	}
}

void InspectorWindow::DrawLightComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<LightComponent> lightState(selectedActors_);

	if (lightState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Light")) {
		RightClickContext<LightComponent>("##LightPopup", selectedActors_);

		Ref<LightComponent> light = lightState.GetFirst();

		bool hasMixedSpec = lightState.HasMixedVec4(&LightComponent::getSpec);
		bool hasMixedDiff = lightState.HasMixedVec4(&LightComponent::getDiff);
		bool hasMixedIntensity = lightState.HasMixedFloat(&LightComponent::getIntensity);

		// Specular
		Vec4 spec = light->getSpec();
		float specular[3] = { spec.x, spec.y, spec.z };

		ImGui::Text("Specular");
		ImGui::SameLine();
		
		if (hasMixedSpec && !isEditingSpec) {
			ImGui::DragFloat3("##Specular", specular, 0.01f, 0.0f, 0.0f, "---");
		}
		else
		{
			if (ImGui::DragFloat3("##Specular", specular, 0.01f)) {
				for (int i = 0; i < 3; ++i) {
					if (specular[i] < 0) {
						specular[i] = 0;
					}
					else if (specular[i] > 1) {
						specular[i] = 1;
					}
				}

				for (auto& component : lightState.components) {
					Vec4 currentSpec = component->getSpec();
					component->setSpec(Vec4(specular[0], specular[1], specular[2], currentSpec.w));
				}
			}
		}

		isEditingSpec = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// Diffuse
		Vec4 diff = light->getDiff();
		float diffuse[3] = { diff.x, diff.y, diff.z };

		ImGui::Text("Diffuse");
		ImGui::SameLine();

		if (hasMixedDiff && !isEditingDiff) {
			ImGui::DragFloat3("##Diffuse", diffuse, 0.01f, 0.0f, 0.0f, "---");
		}
		else {
			if (ImGui::DragFloat3("##Diffuse", diffuse, 0.01f)) {
				for (int i = 0; i < 3; ++i) {
					if (diffuse[i] < 0) {
						diffuse[i] = 0;
					}
					else if (diffuse[i] > 1) {
						diffuse[i] = 1;
					}
				}
				for (auto& component : lightState.components) {
					Vec4 currentDiff = component->getDiff();
					component->setDiff(Vec4(diffuse[0], diffuse[1], diffuse[2], currentDiff.w));
				}
			}
		}

		isEditingDiff = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// Intensity
		float intensity = light->getIntensity();

		ImGui::Text("Intensity");
		ImGui::SameLine();

		if (hasMixedIntensity && !isEditingIntensity) {
			ImGui::DragFloat("##Intensity", &intensity, 0.1f, 0.0f, 0.0f, "---");
		}
		else {
			if (ImGui::DragFloat("##Intensity", &intensity, 0.1f)) {

				if (intensity < 0) {
					intensity = 0;
				}

				for (auto& component : lightState.components) {
					component->setIntensity(intensity);
				}
			}
		}

		isEditingIntensity = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		if (ImGui::Button("Light Type##Button", ImVec2(-1, 0))) {
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
				for (auto& component : lightState.components) {
					component->setType(LightType::Sky);
				}
			}
			if (ImGui::Selectable("Point Light")) {
				for (auto& component : lightState.components) {
					component->setType(LightType::Point);
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	ImGui::Separator();
}

void InspectorWindow::DrawShaderComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<ShaderComponent> shaderState(selectedActors_);

	if (shaderState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Shader"), ImGuiTreeNodeFlags_DefaultOpen) {
		RightClickContext<ShaderComponent>("##ShaderPopup", selectedActors_);

		// one actor selected
		if (shaderState.allHaveComponent && shaderState.Count() == 1) {
			Ref<ShaderComponent> shader = shaderState.GetFirst();

			// displaying some basic shader information
			ImGui::TextWrapped("Shader Program ID: %u", shader->GetProgram());
			ImGui::TextWrapped("Shader Vert: %s", shader->GetVertName());
			ImGui::TextWrapped("Shader Frag: %s", shader->GetFragName());
		}
		else if (shaderState.allHaveComponent) {
			ImGui::TextWrapped("All Selected Actors Have a ShaderComponent");
		}
		else if (shaderState.someHaveComponent) {
			ImGui::TextWrapped("Some Selected Actors Have a ShaderComponent");
		}


		ImGui::Button("Drop New Shader Here", ImVec2(-1, 0));
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_ASSET");
			if (payload) {
				// store the payload data
				const char* droppedAssetName = static_cast<const char*>(payload->Data);
				
				// get a reference to the asset that has been dropped
				Ref<ShaderComponent> newShader = AssetManager::getInstance().GetAsset<ShaderComponent>(droppedAssetName);
				
				// get the actor
				if (newShader) {
					for (auto& component : shaderState.components) {
						for (const auto& pair : selectedActors_) {
							// replace the actors shader
							if (pair.second->GetComponent<ShaderComponent>() == component) {
								pair.second->ReplaceComponent<ShaderComponent>(newShader);
							}
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

	}
	ImGui::Separator();
}

void InspectorWindow::DrawPhysicsComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<PhysicsComponent> physicsState(selectedActors_);

	if (physicsState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
		RightClickContext<PhysicsComponent>("##PhysicsPopup", selectedActors_);
		
		// Mass
		bool hasMixedMass = physicsState.HasMixedFloat(&PhysicsComponent::getMass);

		float displayMass = physicsState.GetFirst()->getMass();

		float mass = displayMass;

		ImGui::Text("Mass ");
		ImGui::SameLine();

		if (hasMixedMass && !isEditingMass) {
			ImGui::DragFloat("##Mass", &mass, 0.1f, 0.0f, 100.0f, "---");
		}
		else { // no delta (might add later)
			if (ImGui::DragFloat("##Mass", &mass, 0.1f, 0.1f, 100.0f)) {
				for (auto& component : physicsState.components) {
					component->setMass(mass);
				}
			}
		}

		isEditingMass = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// Drag

		// Apply gravity
		ImGui::Text("Use Gravity ");
		ImGui::SameLine();
		ImGui::Checkbox("##Checkbox", &useGravity); 
		if (useGravity) {
			for (auto& component : physicsState.components) {
				component->ApplyForce(Vec3(0.0f, -9.8f * component->getMass(), 0.0f));
			}
		}
		else {
			for (auto& component : physicsState.components) {
				component->setVel(Vec3(0.0f, 0.0f, 0.0f));
				component->setAccel(Vec3(0.0f, 0.0f, 0.0f));
			}
		}
		
		if (ImGui::TreeNode("Info")) {
			// Velocity
			Vec3 displayVel = physicsState.GetFirst()->getVel();
			float vel[3] = { displayVel.x, displayVel.y, displayVel.z };

			ImGui::Text("Velocity ");
			ImGui::SameLine();
			ImGui::DragFloat3("##Vel", vel, 0.1f);
			

			// Acceleration
			Vec3 displayAccel = physicsState.GetFirst()->getAccel();
			float accel[3] = { displayAccel.x, displayAccel.y, displayAccel.z };

			ImGui::Text("Acceleration ");
			ImGui::SameLine();
			ImGui::DragFloat3("##Accel", accel, 0.1f);
			
			ImGui::TreePop();
		}
	}
}
