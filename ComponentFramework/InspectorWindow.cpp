#include "pch.h"
#include "InspectorWindow.h"
#include "InputManager.h"
#include "EditorManager.h"
#include "AnimatorComponent.h"
#include "PhysicsSystem.h"
#include <algorithm>
//#include "ScriptComponent.h"
#include "CollisionComponent.h"
#include "ColliderDebug.h"


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

			// collision
			if (selectedActor->second->GetComponent<CollisionComponent>()) {
				DrawCollisionComponent(sceneGraph->debugSelectedAssets);
			}

			// camera
			if (selectedActor->second->GetComponent<CameraComponent>()) {
				DrawCameraComponent(sceneGraph->debugSelectedAssets);
			}
			
			// script
			if (selectedActor->second->GetComponent<ScriptComponent>()) {
				DrawScriptComponent(sceneGraph->debugSelectedAssets);
			}

			if (selectedActor->second->GetComponent<AnimatorComponent>()) {
				DrawAnimatorComponent(sceneGraph->debugSelectedAssets);
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
						PhysicsSystem::getInstance().AddActor(selectedActor->second);
					}
				}

				if (ImGui::Selectable("Collision Component")) {
					if (!selectedActor->second->GetComponent<CollisionComponent>()) {
						selectedActor->second->AddComponent<CollisionComponent>();
						CollisionSystem::getInstance().AddActor(selectedActor->second);
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
				if (ImGui::Selectable("Animator Component")) {
					if (selectedActor->second->GetComponent<MeshComponent>())
					selectedActor->second->AddComponent<AnimatorComponent>(selectedActor->second.get());
					else {
						// if the actor can't be found by name or by ID
#ifdef _DEBUG
						Debug::Error("You require a MeshComponent to create an Animator!: " + selectedActor->second->getActorName(), __FILE__, __LINE__);
#endif
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
			DrawCollisionComponent(sceneGraph->debugSelectedAssets);
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
				ComponentState<CollisionComponent> collisionState(sceneGraph->debugSelectedAssets);
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

				if (collisionState.noneHaveComponent || collisionState.someHaveComponent) {
					if (ImGui::Selectable("Collision Component")) {
						for (const auto& pair : sceneGraph->debugSelectedAssets) {
							if (!pair.second->GetComponent<CollisionComponent>()) {
								pair.second->AddComponent<CollisionComponent>();
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

	// actor tag system, similar to unity
	// gets all tag list from scenegraph, then can select current actors tag
	// can also add tags too
	ImGui::Text("Tag");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(labelWidth);

	std::vector<std::string>& allTags = sceneGraph->getAllTags();
	const std::string& currentTag = actor_->getTag();

	// going through each tag in the vector
	int currentTagIndex = 0;
	for (int i = 0; i < (int)allTags.size(); i++) {
		if (allTags[i] == currentTag) {
			currentTagIndex = i; 
			break;
		}
	}

	// building the dropdown menu for the tags
	
	// converting strings to cstrings (imgui dropdown expect cstrings)
	std::vector<const char*> tagCStr;
	tagCStr.reserve(allTags.size());
	for (auto& tag : allTags) {
		tagCStr.push_back(tag.c_str());
	}

	if (ImGui::Combo("##TagDropdown", &currentTagIndex, tagCStr.data(), (int)tagCStr.size())) {
		actor_->setTag(allTags[currentTagIndex]);
	}

	// TODO: add tag
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
		bool hasMixedRot = transformState.HasMixedQuaternion(&TransformComponent::GetOrientation);

		if (!isEditingRotation) {
			// get the quaternions rotation when not edting anything
			if (!ImGui::IsAnyItemActive()) {
				Quaternion firstQuat = transformState.GetFirst()->GetOrientation();
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
				Quaternion firstQuat = transformState.GetFirst()->GetOrientation();
				Quaternion delta = newOrientation * QMath::inverse(firstQuat);

				for (auto& component : transformState.components) {
					Quaternion currentQuat = component->GetOrientation();
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
					component->SetScale(Vec3(updatedUniformScale, updatedUniformScale, updatedUniformScale));
				}

				lastScale = lastScale + Vec3(uniformDelta, uniformDelta, uniformDelta);
				scale[0] = scale[1] = scale[2] = lastScale.x;

			}
			else {
				Vec3 delta(scale[0] - lastScale.x, scale[1] - lastScale.y, scale[2] - lastScale.z);

				for (auto& component : transformState.components) {
					Vec3 currentScale = component->GetScale();
					component->SetScale(Vec3(currentScale.x + delta.x, currentScale.y + delta.y, currentScale.z + delta.z));
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

	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
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

	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
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

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
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
		if (ImGui::DragFloat("##FarDrag", &farClipPlane, 1.0f, 0.0f, 1000.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
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

			if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
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

				std::unordered_map<std::string, sol::object> pubRefMap = script->getPublicReferences();
				int id = 0;

				for (auto& pair : pubRefMap) {

					auto& obj = pair.second;
					ImGui::PushID(id);

					if (pair.second.is<float>()) {

						float objVar = pair.second.as<float>();

						bool isEditingAnim = false;

						{
							bool valChanged = false;

							ImGui::Text("%s:", pair.first.c_str());
							ImGui::SameLine();
							valChanged = ImGui::DragFloat("##FloatVar", &objVar,
								0.1f, -1000, 1000);
							if (valChanged) {
								script->setPublicReference(pair.first, objVar);
							}

							isEditingAnim = ImGui::IsItemActive();
							ImGui::ActiveItemLockMousePos();
						}
						

					}
					id++;
					
					ImGui::PopID(); 

					
				}

			}
			ImGui::Separator();

			ImGui::PopID(); // Pop the ID from the stack
		}
	}
}


void InspectorWindow::DrawAnimatorComponent(const std::unordered_map<uint32_t, Ref<Actor>>&selectedActors_)
{
	ComponentState<AnimatorComponent> animatorState(selectedActors_);
	if (animatorState.noneHaveComponent) return;

	for (auto& animator : selectedActors_.begin()->second->GetAllComponent<AnimatorComponent>()) {
		if (ImGui::CollapsingHeader("AnimatorComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
			RightClickContext<AnimatorComponent>("##AnimatorPopup", sceneGraph->debugSelectedAssets);

			ImGuiChildFlags detailsFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
			if (ImGui::BeginChild("details_panel", ImVec2(0, 150), detailsFlags)) {
				if (ImGui::CollapsingHeader("Animation Clip")) {




					float clipLength = animator->getClipLength();
					float startTime = animator->getStartTime();
					float speedMult = animator->getSpeedMult();
					float currentTime = animator->getCurrentTime();

					bool isEditingAnim = false;

					//Current Time
					{
						ImGui::Text("Current Time:");
						ImGui::SameLine();
						bool currentTimeChanged = false;

						if (clipLength >= 0.0f) {

							char displayTimeRatioBuffer[64];
							std::snprintf(displayTimeRatioBuffer, sizeof(displayTimeRatioBuffer), "%%.3f / %.3f", clipLength);

							currentTimeChanged = ImGui::DragFloat("##CurrentTime", &currentTime,
								0.1f, 0.0f, clipLength, displayTimeRatioBuffer);

							currentTime = std::clamp(currentTime, 0.0f, clipLength);

						}
						else {
							ImGui::Text("NULL");
						}

						if (currentTimeChanged) {
							animator->setCurrentTime(currentTime);
						}
						isEditingAnim = ImGui::IsItemActive();
						ImGui::ActiveItemLockMousePos();
					}

					//Start Time
					{
						ImGui::Text("Start Time:");
						ImGui::SameLine();
						bool startTimeChanged = false;

						if (clipLength >= 0.0f) {

							char displayTimeRatioBuffer[64];
							std::snprintf(displayTimeRatioBuffer, sizeof(displayTimeRatioBuffer), "%%.3f / %.3f", clipLength);

							startTimeChanged = ImGui::DragFloat("##StartTime", &startTime,
								0.1f, 0.0f, clipLength, displayTimeRatioBuffer);

							startTime = std::clamp(startTime, 0.0f, clipLength);

						}
						else {
							ImGui::Text("NULL");
						}

						if (startTimeChanged) {
							animator->setStartTime(startTime);
						}
						isEditingAnim = ImGui::IsItemActive();
						ImGui::ActiveItemLockMousePos();
					}
					//SPEEDMULT
					{
						bool speedMultChanged = false;

						ImGui::Text("Speed Multiplier:");
						ImGui::SameLine();
						speedMultChanged = ImGui::DragFloat("##SpeedMult", &speedMult,
							0.1f, -10, 10);
						speedMult = std::clamp(speedMult, -10.0f, 10.0f);
						if (speedMultChanged) {
							animator->setSpeedMult(speedMult);
						}

						isEditingAnim = ImGui::IsItemActive();
						ImGui::ActiveItemLockMousePos();
					}

					//LOOPING
					{
						ImGui::Text("is Looping?:");
						ImGui::SameLine();

						bool isLooping = animator->getLoopingState();
						if (ImGui::Checkbox("##Looping", &isLooping)) {
							animator->setLooping(isLooping);
						}
					}

					/*ImGui::TextWrapped("Script Path: %s", script->getPath().c_str());*/

					if (animator->hasAnim()) 
						ImGui::Text(animator->getAnimName().c_str());

					ImGui::Button("Drop Animation Here", ImVec2(-1, 0));


					if (ImGui::BeginDragDropTarget()) {
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_ASSET");
						if (payload) {
							// store the payload data
							const char* droppedAssetName = static_cast<const char*>(payload->Data);

							// get a reference to the asset that has been dropped
							//ScriptAbstract is the asset in assetmanager that has the name, the actors have the component
							Ref<Animation> newAnim = AssetManager::getInstance().GetAsset<Animation>(droppedAssetName);

							animator->setAnimation(newAnim);
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::Separator();
			}
			ImGui::EndChild();
		}
		ImGui::Separator();
	}
}

void InspectorWindow::DrawLightComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<LightComponent> lightState(selectedActors_);

	if (lightState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
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

	if (ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen)) {
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
		RightClickContext<PhysicsComponent>("##PhysicsPopup", sceneGraph->debugSelectedAssets);

		ImGui::Text("Physics State");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		// Physics State
		PhysicsState currentState = physicsState.GetFirst()->getState();

		// creating a dropdown, tracking current state name
		const char* stateModes[] = { "Dynamic", "Kinematic", "Static" };
		int stateIndex = static_cast<int>(currentState);

		// dropdown for states
		if (ImGui::Combo("##PhysicsStates", &stateIndex, stateModes, 3)) {
			PhysicsState newState = static_cast<PhysicsState>(stateIndex);

			for (auto& component : physicsState.components) {
				component->setState(newState);
			}
		}


		// static objects are not affected by mass, drag, gravity (dont show those variables)
		if (physicsState.GetFirst()->getState() != PhysicsState::Static) {
			// Mass
			ImGui::Text("Mass");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(labelWidth);

			bool hasMixedMass = physicsState.HasMixedFloat(&PhysicsComponent::getMass);
			float displayMass = physicsState.GetFirst()->getMass();
			float mass = displayMass;

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
			ImGui::Text("Drag");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(labelWidth);

			bool hasMixedDrag = physicsState.HasMixedFloat(&PhysicsComponent::getDrag);
			float displayDrag = physicsState.GetFirst()->getDrag();
			float drag = displayDrag;

			if (hasMixedDrag && !isEditingDrag) {
				ImGui::DragFloat("##Drag", &drag, 0.1f, 0.0f, 2.0f, "---");
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##Drag", &drag, 0.1f, 0.1f, 2.0f)) {
					for (auto& component : physicsState.components) {
						component->setDrag(drag);
					}
				}
			}

			isEditingDrag = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();

			// Angular Drag
			ImGui::Text("Angular Drag (NOT IMPLEMENTED YET)");

			bool hasMixedAngularDrag = physicsState.HasMixedFloat(&PhysicsComponent::getAngularDrag);
			float displayAngularDrag = physicsState.GetFirst()->getAngularDrag();
			float angularDrag = displayAngularDrag;

			if (hasMixedAngularDrag && !isEditingAngularDrag) {
				ImGui::DragFloat("##AngularDrag", &angularDrag, 0.01f, 0.0f, 2.0f, "---");
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##AngularDrag", &angularDrag, 0.01f, 0.1f, 2.0f)) {
					for (auto& component : physicsState.components) {
						component->setAngularDrag(angularDrag);
					}
				}
			}

			isEditingAngularDrag = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();

			// Apply gravity
			ImGui::Text("Use Gravity");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(labelWidth);

			bool useGravity = physicsState.GetFirst()->getUseGravity();

			if (ImGui::Checkbox("##UseGrav", &useGravity)) {
				for (auto& component : physicsState.components) {
					component->setUseGravity(useGravity);
				}
			}
		}

		// Friction
		ImGui::Text("Friction");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		bool hasMixedFriction = physicsState.HasMixedFloat(&PhysicsComponent::getFriction);
		float displayFriction = physicsState.GetFirst()->getFriction();
		float friction = displayFriction;

		if (hasMixedFriction && !isEditingFriction) {
			ImGui::DragFloat("##Friction", &friction, 0.01f, 0.0f, 1.0f, "---");
		}
		else { // no delta (might add later)
			if (ImGui::DragFloat("##Friction", &friction, 0.01f, 0.1f, 1.0f)) {
				for (auto& component : physicsState.components) {
					component->setFriction(friction);
				}
			}
		}

		isEditingFriction = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// Restitution 
		ImGui::Text("Restitution");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		bool hasMixedRestitution = physicsState.HasMixedFloat(&PhysicsComponent::getRestitution);
		float displayRestitution = physicsState.GetFirst()->getRestitution();
		float restitution = displayRestitution;

		if (hasMixedRestitution && !isEditingRestitution) {
			ImGui::DragFloat("##Restitution", &restitution, 0.01f, 0.0f, 1.0f, "---");
		}
		else { // no delta (might add later)
			if (ImGui::DragFloat("##Restitution", &restitution, 0.01f, 0.1f, 1.0f)) {
				for (auto& component : physicsState.components) {
					component->setRestitution(restitution);
				}
			}
		}

		isEditingFriction = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// constraints dropdown
		if (ImGui::TreeNode("Constraints")) {
			PhysicsConstraints constraints = physicsState.GetFirst()->getConstraints();
			bool changed = false;

			ImGui::Text("Freeze Positions");
			ImGui::SameLine();
			bool freezePosX = constraints.freezePosX, freezePosY = constraints.freezePosY, freezePosZ = constraints.freezePosZ;
			if (ImGui::Checkbox("X##Pos", &freezePosX)) {
				constraints.freezePosX = freezePosX;
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Y##Pos", &freezePosY)) {
				constraints.freezePosY = freezePosY;
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Z##Pos", &freezePosZ)) {
				constraints.freezePosZ = freezePosZ;
				changed = true;
			}

			ImGui::Text("Freeze Rotation");
			ImGui::SameLine();
			bool freezeRotX = constraints.freezeRotX, freezeRotY = constraints.freezeRotY, freezeRotZ = constraints.freezeRotZ;
			if (ImGui::Checkbox("X##Rot", &freezeRotX)) {
				constraints.freezeRotX = freezeRotX;
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Y##Rot", &freezeRotY)) {
				constraints.freezeRotY = freezeRotY;
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Z##Rot", &freezeRotZ)) {
				constraints.freezeRotZ = freezeRotZ;
				changed = true;
			}

			if (changed) {
				for (auto& component : physicsState.components) {
					component->setConstraints(constraints);
				}
			}

			ImGui::TreePop();
		}

		// additional info, check things like velocity and acceleration (not editable)		
		if (ImGui::TreeNode("Info")) {			
			
			// just displaying text, 1 decimal place, displaying acceleration as speed (might change)
			ImGui::Text("Speed: %.1f", VMath::mag(physicsState.GetFirst()->getAccel()));
			ImGui::Text("Velocity: (%.1f, %.1f, %.1f)", physicsState.GetFirst()->getVel().x, physicsState.GetFirst()->getVel().y, physicsState.GetFirst()->getVel().z);
			
			ImGui::TreePop();
		}
	}

	ImGui::Separator();
}

void InspectorWindow::DrawCollisionComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<CollisionComponent> collisionState(selectedActors_);

	if (collisionState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
		RightClickContext<CollisionComponent>("##CollisionPopup", sceneGraph->debugSelectedAssets);

		ImGui::Text("Colldier Type");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		// Collision Type 
		ColliderType currentType = collisionState.GetFirst()->getType();

		// creating a dropdown, tracking current type name
		const char* typeModes[] = { "Sphere", "Capsule", "AABB", "OBB" };
		int typeIndex = static_cast<int>(currentType);

		// dropdown for types
		if (ImGui::Combo("##ColliderTypes", &typeIndex, typeModes, 4)) {
			ColliderType newType = static_cast<ColliderType>(typeIndex);

			for (auto& component : collisionState.components) {
				component->setType(newType);
			}
		}

		ImGui::Text("Collider Detection");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		// Collision State
		ColliderState currentState = collisionState.GetFirst()->getState();

		// creating a dropdown, tracking current state name
		const char* stateModes[] = { "Discrete", "Continuous" };
		int stateIndex = static_cast<int>(currentState);

		// dropdown for states
		if (ImGui::Combo("##ColliderStates", &stateIndex, stateModes, 2)) {
			ColliderState newState = static_cast<ColliderState>(stateIndex);

			for (auto& component : collisionState.components) {
				component->setState(newState);
			}
		}

		// is trigger
		ImGui::Text("Is Trigger");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(labelWidth);

		bool isTrigger = collisionState.GetFirst()->getIsTrigger();

		if (ImGui::Checkbox("##isTrigger", &isTrigger)) {
			for (auto& component : collisionState.components) {
				component->setIsTrigger(isTrigger);
			}
		}

		if (collisionState.GetFirst()->getType() == ColliderType::Sphere || collisionState.GetFirst()->getType() == ColliderType::Capsule) {
			// Radius
			ImGui::Text("Radius");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(labelWidth);

			bool hasMixedRadius = collisionState.HasMixedFloat(&CollisionComponent::getRadius);
			float displayRadius = collisionState.GetFirst()->getRadius();
			float radius = displayRadius;

			if (hasMixedRadius && !isEditingRadius) {
				ImGui::DragFloat("##Radius", &radius, 0.1f, 0.0f, 100.0f, "---");
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##Radius", &radius, 0.1f, 0.1f, 100.0f)) {
					for (auto& component : collisionState.components) {
						component->setRadius(radius);
					}
				}
			}

			isEditingRadius = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();
		}

		if (collisionState.GetFirst()->getType() == ColliderType::Sphere || collisionState.GetFirst()->getType() == ColliderType::AABB || collisionState.GetFirst()->getType() == ColliderType::OBB) {
			// Centre
			bool hasMixedCentre = collisionState.HasMixedVec3(&CollisionComponent::getCentre);

			Vec3 displayCentre = collisionState.GetFirst()->getCentre();
			lastCentrePosA = displayCentre;

			float centre[3] = { displayCentre.x, displayCentre.y, displayCentre.z };

			ImGui::Text("Centre");
			ImGui::SameLine();

			if (hasMixedCentre && !isEditingCentre) {
				ImGui::DragFloat3("##Centre", centre, 0.1f, 0.0f, 0.0f, "---");
			}
			else {
				if (ImGui::DragFloat3("##Centre", centre, 0.1f)) {
					Vec3 newPos(centre[0], centre[1], centre[2]);
					Vec3 delta = newPos - lastCentrePosA;

					for (auto& component : collisionState.components) {
						Vec3 currentPos = component->getCentre();
						Vec3 updatedPos = currentPos + delta;
						component->setCentre(Vec3(updatedPos.x, updatedPos.y, updatedPos.z));
					}

					lastCentrePosA = newPos;
				}
			}

			isEditingCentre = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();
		}
		
		if (collisionState.GetFirst()->getType() == ColliderType::Capsule) {
			// CentrePosA
			bool hasMixedCentrePosA = collisionState.HasMixedVec3(&CollisionComponent::getCentrePosA);

			Vec3 displayCentrePosA = collisionState.GetFirst()->getCentrePosA();
			lastCentrePosA = displayCentrePosA;

			float centrePosA[3] = { displayCentrePosA.x, displayCentrePosA.y, displayCentrePosA.z };

			ImGui::Text("CentrePosA");
			ImGui::SameLine();

			if (hasMixedCentrePosA && !isEditingCentrePosA) {
				ImGui::DragFloat3("##CentrePosA", centrePosA, 0.1f, 0.0f, 0.0f, "---");
			}
			else {
				if (ImGui::DragFloat3("##CentrePosA", centrePosA, 0.1f)) {
					Vec3 newPos(centrePosA[0], centrePosA[1], centrePosA[2]);
					Vec3 delta = newPos - lastCentrePosA;

					for (auto& component : collisionState.components) {
						Vec3 currentPos = component->getCentrePosA();
						Vec3 updatedPos = currentPos + delta;
						component->setCentrePosA(Vec3(updatedPos.x, updatedPos.y, updatedPos.z));
					}

					lastCentrePosA = newPos;
				}
			}

			isEditingCentrePosA = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();

			// CentrePosB
			bool hasMixedCentrePosB = collisionState.HasMixedVec3(&CollisionComponent::getCentrePosB);

			Vec3 displayCentrePosB = collisionState.GetFirst()->getCentrePosB();
			lastCentrePosB = displayCentrePosB;

			float centrePosB[3] = { displayCentrePosB.x, displayCentrePosB.y, displayCentrePosB.z };

			ImGui::Text("CentrePosB");
			ImGui::SameLine();

			if (hasMixedCentrePosB && !isEditingCentrePosB) {
				ImGui::DragFloat3("##CentrePosB", centrePosB, 0.1f, 0.0f, 0.0f, "---");
			}
			else {
				if (ImGui::DragFloat3("##CentrePosB", centrePosB, 0.1f)) {
					Vec3 newPos(centrePosB[0], centrePosB[1], centrePosB[2]);
					Vec3 delta = newPos - lastCentrePosB;

					for (auto& component : collisionState.components) {
						Vec3 currentPos = component->getCentrePosB();
						Vec3 updatedPos = currentPos + delta;
						component->setCentrePosB(Vec3(updatedPos.x, updatedPos.y, updatedPos.z));
					}

					lastCentrePosB = newPos;
				}
			}

			isEditingCentrePosB = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();
		}

		if (collisionState.GetFirst()->getType() == ColliderType::AABB || collisionState.GetFirst()->getType() == ColliderType::OBB) {
			// Half Extents
			bool hasMixedHalfExtents = collisionState.HasMixedVec3(&CollisionComponent::getHalfExtents);

			Vec3 displayHalfExtents = collisionState.GetFirst()->getHalfExtents();
			lastHalfExtents = displayHalfExtents;

			float halfExtents[3] = { displayHalfExtents.x, displayHalfExtents.y, displayHalfExtents.z };

			ImGui::Text("HalfExtents");
			ImGui::SameLine();

			if (hasMixedHalfExtents && !isEditingHalfExtents) {
				ImGui::DragFloat3("##HalfExtents", halfExtents, 0.1f, 0.0f, 0.0f, "---");
			}
			else {
				if (ImGui::DragFloat3("##HalfExtents", halfExtents, 0.1f)) {
					Vec3 newPos(halfExtents[0], halfExtents[1], halfExtents[2]);
					Vec3 delta = newPos - lastHalfExtents;

					for (auto& component : collisionState.components) {
						Vec3 currentPos = component->getHalfExtents();
						Vec3 updatedPos = currentPos + delta;
						component->setHalfExtents(Vec3(updatedPos.x, updatedPos.y, updatedPos.z));
					}

					lastHalfExtents = newPos;
				}
			}

			isEditingHalfExtents = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();
		}

		if (collisionState.GetFirst()->getType() == ColliderType::OBB) {
			// Orientation
			bool hasMixedOri = collisionState.HasMixedQuaternion(&CollisionComponent::getOrientation);

			if (!isEditingOrientation) {
				// get the quaternions rotation when not edting anything
				if (!ImGui::IsAnyItemActive()) {
					Quaternion firstQuat = collisionState.GetFirst()->getOrientation();
					Euler quatEuler = EMath::toEuler(firstQuat);
					oriEulerAngles = Vec3(quatEuler.xAxis, quatEuler.yAxis, quatEuler.zAxis);
				}
			}

			float orientation[3] = { oriEulerAngles.x, oriEulerAngles.y, oriEulerAngles.z };

			ImGui::Text("Orientation");
			ImGui::SameLine();

			bool rotationChanged = false;

			if (hasMixedOri && !isEditingOrientation) {
				ImGui::DragFloat3("##Orientation", orientation, 0.1f, 0.0f, 0.0f, "---");
			}
			else {
				rotationChanged = ImGui::DragFloat3("##Orientation", orientation, 1.0f);
			}

			if (rotationChanged) {
				// store the euler angles
				oriEulerAngles.x = orientation[0];
				oriEulerAngles.y = orientation[1];
				oriEulerAngles.z = orientation[2];

				// convert euler to quaternion
				Euler eulerRot(orientation[0], orientation[1], orientation[2]);
				Quaternion newOrientation = QMath::toQuaternion(eulerRot);

				if (collisionState.components.size() == 1) {
					collisionState.GetFirst()->setOrientation(QMath::normalize(newOrientation));
				}
				else {
					Quaternion firstQuat = collisionState.GetFirst()->getOrientation();
					Quaternion delta = newOrientation * QMath::inverse(firstQuat);

					for (auto& component : collisionState.components) {
						Quaternion currentQuat = component->getOrientation();
						Quaternion updatedQuat = delta * currentQuat;
						component->setOrientation(QMath::normalize(updatedQuat));
					}
				}

			}

			isEditingOrientation = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();
		}
	}

	ImGui::Separator();
}
