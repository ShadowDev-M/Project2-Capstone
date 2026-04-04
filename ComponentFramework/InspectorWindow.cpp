#include "pch.h"
#include "InspectorWindow.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#include "PhysicsSystem.h"
#include <algorithm>
#include "CollisionComponent.h"
#include "ColliderDebug.h"
#include "LightingSystem.h"

InspectorWindow::InspectorWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
	EditorManager::getInstance().RegisterWindow("Inspector", true);
}

void InspectorWindow::ShowInspectorWindow(bool* pOpen)
{
	const auto& selectedAsset = EditorManager::getInstance().GetSelectedAsset();

	// dummy prefab editor
	if (!dummyActors.empty()) {
		bool prefabStillSelected = selectedAsset.isSet && selectedAsset.absolutePath == loadedPrefabPath;

		if (!prefabStillSelected) {
			ClosePrefabEditor(true);
			EditorManager::getInstance().ClearSelectedAsset();
		}
	}

	if (ImGui::Begin("Inspector", pOpen)) {

		if (selectedAsset.isSet) {
			DrawAssetInspector(selectedAsset);
			ImGui::End();
			return;
		}

		// no actors selected
		const auto& selected = sceneGraph->debugSelectedAssets;
		if (selected.empty()) {
			ImGui::Text("No Actor Selected");
			ImGui::End();
			return;
		}

		// only 1 actor is selected
		if (selected.size() == 1) {
			DrawActorHeader(selected.begin()->second);
		}
		else {
			if (ImGui::CollapsingHeader("Selected Actors", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (const auto& [id, actor] : selected) {
					ImGui::Text("%s", actor->getActorName().c_str());
				}
			}
			ImGui::Separator();
		}

		/// Components Section
		DrawTransformComponent(selected);
		DrawCameraComponent(selected);
		DrawLightComponent(selected);
		DrawMeshComponent(selected);
		DrawMaterialComponent(selected);
		DrawShaderComponent(selected);
		DrawPhysicsComponent(selected);
		DrawCollisionComponent(selected);
		DrawScriptComponent(selected);
		DrawAnimatorComponent(selected);

		// Add Component
		if (ImGui::Button("Add Component##Button", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Add Component##Popup");
		}

		ShowAddComponentPopup(selected);

	}
	ImGui::End();
}

void InspectorWindow::ShowAddComponentPopup(const std::unordered_map<uint32_t, Ref<Actor>>& selected)
{
	if (ImGui::BeginPopup("Add Component##Popup", ImGuiWindowFlags_AlwaysAutoResize)) {

		// TODO search bar

		ImGui::Text("Component");
		ImGui::Separator();

		// states
		ComponentState<MeshComponent> meshState(selected);
		ComponentState<MaterialComponent> materialState(selected);
		ComponentState<ShaderComponent> shaderState(selected);
		ComponentState<PhysicsComponent> physicsState(selected);
		ComponentState<CollisionComponent> collisionState(selected);
		ComponentState<CameraComponent> cameraState(selected);
		ComponentState<LightComponent> lightState(selected);
		ComponentState<AnimatorComponent> animatorState(selected);

		// lambda for greying out components
		auto showIf = [](const auto& state) {
			return state.noneHaveComponent || state.someHaveComponent;
			};

		if (showIf(meshState)) {
			if (ImGui::Selectable("Mesh Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<MeshComponent>()) {
						RECORD actor->AddComponent<MeshComponent>(nullptr, "");
						if (!actor->GetComponent<ShadowSettings>()) {
							RECORD actor->AddComponent<ShadowSettings>(nullptr, true);
						}
					}
				}
			}
		}

		if (showIf(materialState)) {
			if (ImGui::Selectable("Material Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<MaterialComponent>()) {
						RECORD actor->AddComponent<MaterialComponent>(nullptr, "", "");
						if (!actor->GetComponent<TilingSettings>()) {
							RECORD actor->AddComponent<TilingSettings>(nullptr, false);
						}
					}
				}
			}
		}

		if (showIf(shaderState)) {
			if (ImGui::Selectable("Shader Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<ShaderComponent>()) {
						RECORD actor->AddComponent<ShaderComponent>(nullptr, "", "");
					}
				}
			}
		}

		if (showIf(physicsState)) {
			if (ImGui::Selectable("Physics Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<PhysicsComponent>()) {
						RECORD actor->AddComponent<PhysicsComponent>();
						PhysicsSystem::getInstance().AddActor(actor);
					}
				}
			}
		}

		if (showIf(collisionState)) {
			if (ImGui::Selectable("Collision Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<CollisionComponent>()) {
						RECORD actor->AddComponent<CollisionComponent>();
						CollisionSystem::getInstance().AddActor(actor);
					}
				}
			}
		}

		if (showIf(cameraState)) {
			if (ImGui::Selectable("Camera Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<CameraComponent>()) {
						RECORD actor->AddComponent<CameraComponent>(actor.get(), ProjectionType::Perspective, 60.0f, 0.03f, 1000.0f, 5.0f);
					}
				}
			}
		}

		if (ImGui::Selectable("Script Component")) {
			for (auto& [id, actor] : selected) {
				RECORD actor->AddComponent<ScriptComponent>(actor.get());
			}
		}

		if (showIf(animatorState)) {
			if (ImGui::Selectable("Animator Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<AnimatorComponent>()) {
						if (actor->GetComponent<MeshComponent>()) {
							RECORD actor->AddComponent<AnimatorComponent>(actor.get());
						}
						else {
#ifdef _DEBUG
							Debug::Error("You require a MeshComponent to create an Animator!: " + actor->getActorName(), __FILE__, __LINE__);
#endif
						}
					}
				}
			}
		}

		if (showIf(lightState)) {
			if (ImGui::Selectable("Light Component")) {
				for (auto& [id, actor] : selected) {
					if (!actor->GetComponent<LightComponent>()) {
						RECORD actor->AddComponent<LightComponent>();
						LightingSystem::getInstance().AddActor(actor);
					}
				}
			}
		}

		ImGui::EndPopup();
	}
}

void InspectorWindow::DrawActorHeader(Ref<Actor> actor_)
{
	if (oldActorName != actor_->getActorName()) {
		oldActorName = actor_->getActorName();
		newActorName = oldActorName;
	}

	ImGui::AlignTextToFramePadding();
	ImGui::InputText("##ActorHeader", &newActorName);
	ImGui::SetNextItemWidth(-1);

	// passes the rename inforamtion over to the editor manager (editor manager then passes it to the hierarchy window) 
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		if (!newActorName.empty() && newActorName != oldActorName) {
			EditorManager::getInstance().RequestActorRename(oldActorName, newActorName);
			oldActorName = newActorName;
		}
	}
	
	ImGui::SameLine();
	ImGui::Text("ID: %i", actor_->getId());

	// actor tag system, similar to unity
	// gets all tag list from scenegraph, then can select current actors tag
	// can also add tags too
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Tag");
	ImGui::SameLine(labelWidth - 20);

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

	// converting strings to cstrings (imgui dropdown expect cstrings)
	std::vector<const char*> tagCStr;
	tagCStr.reserve(allTags.size());
	for (auto& tag : allTags) {
		tagCStr.push_back(tag.c_str());
	}
	
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60);
	// building the dropdown menu for the tags
	if (ImGui::Combo("##TagDropdown", &currentTagIndex, tagCStr.data(), (int)tagCStr.size())) {
		actor_->setTag(allTags[currentTagIndex]);
	}

	ImGui::SameLine();
	// add tag popup button
	if (ImGui::Button("+##AddTag", ImVec2(20.0f, 0.0f))) {
		ImGui::OpenPopup("##TagManager");
		tagBuffer.clear();
	}

	// add or remove tag 
	if (ImGui::BeginPopup("##TagManager")) {
		ImGui::Text("Scene Tags");
		ImGui::Separator();

		int removeTagIndex = -1;
		for (int i = 0; i < (int)allTags.size(); i++) {
			ImGui::PushID(i);

			// untagged is the default tag, so can't remove that
			bool isDefault = (allTags[i] == "Untagged" || allTags[i] == "MainCamera");
			if (isDefault) ImGui::BeginDisabled();

			if (ImGui::SmallButton("X")) {
				removeTagIndex = i;
			}

			if (isDefault) ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::Text("%s", allTags[i].c_str());
			ImGui::PopID();
		}

		if (removeTagIndex >= 0) {
			// check to remove tag from actor using removed tag
			for (const auto& pair : sceneGraph->getAllActors()) {
				if (pair.second->getTag() == allTags[removeTagIndex]) {
					pair.second->setTag("Untagged");
				}
			}
			sceneGraph->removeTag(allTags[removeTagIndex]);
		}

		ImGui::Separator();

		ImGui::SetNextItemWidth(labelWidth);
		ImGui::InputText("##NewTag", &tagBuffer);

		ImGui::SameLine();

		bool canAdd = !tagBuffer.empty();
		if (!canAdd) ImGui::BeginDisabled();

		if (ImGui::Button("Add")) {
			sceneGraph->addTag(tagBuffer);
			tagBuffer.clear();
		}
		
		if (!canAdd) ImGui::EndDisabled();

		ImGui::EndPopup();
	}
}

void InspectorWindow::DrawTransformComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	// component state for all selected actors transform 
	ComponentState<TransformComponent> transformState(selectedActors_);
	
	if (transformState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Setting up right click popup menu, context sensitive to the header
		RightClickContext<TransformComponent>("##TransformPopup", selectedActors_);

		// Position
		bool hasMixedPos = transformState.HasMixedVec3(&TransformComponent::GetPosition);
		
		Vec3 displayPos = transformState.GetFirst()->GetPosition();
		lastPos = displayPos;

		float position[3] = { displayPos.x, displayPos.y, displayPos.z };

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Position");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Rotation");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Scale");
		ImGui::SameLine();
		ImGui::Checkbox("##Lock", &scaleLock);
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

		bool scaleChanged = false;

		if (hasMixedScale && !isEditingScale) {
			ImGui::DragFloat3("##Scale", scale, 0.1f, 0.1f, 100.0f, "---", ImGuiSliderFlags_AlwaysClamp);
		}
		else {
			scaleChanged = ImGui::DragFloat3("##Scale", scale, 0.1f, 0.001f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp);
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
		RightClickContext<MeshComponent>("##MeshPopup", selectedActors_);


		// one actor selected
		if (meshState.allHaveComponent && meshState.Count() == 1) {
			Ref<MeshComponent> mesh = meshState.GetFirst();
		
			// displaying some basic mesh information
			std::string assetName = AssetManager::getInstance().GetAssetName(mesh);
			if (!assetName.empty()) ImGui::TextWrapped("Name: %s", assetName.c_str());
			ImGui::TextWrapped("Vertices: %zu", mesh->getVertices());
			ImGui::TextWrapped("Skeleton: %s", mesh->hasSkeleton() ? "Yes" : "No");
			if (mesh->hasSkeleton()) ImGui::TextWrapped("Bones: %zu", mesh->getBoneCount());
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
			if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
				if (std::string(data->componentType) == "MeshComponent") {
					auto mesh = AssetManager::getInstance().GetAsset<MeshComponent>(data->assetName);
					if (mesh) {
						for (auto& [id, actor] : selectedActors_) {
							actor->ReplaceComponent<MeshComponent>(mesh);
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Cast Shadow");
		ImGui::SameLine(labelWidth + 20);
		
		bool castShadow = false;
		for (const auto& [id, actor] : selectedActors_) {
			if (actor->GetComponent<MeshComponent>()) {
				if (!actor->GetComponent<ShadowSettings>()) {
					actor->AddComponent<ShadowSettings>(actor.get(), true);
				}
				if (!castShadow) {
					castShadow = actor->GetComponent<ShadowSettings>()->getCastShadow();
				}
			}
		}
		if (ImGui::Checkbox("##castShadow", &castShadow)) {
			for (const auto& [id, actor] : selectedActors_) {
				Ref<ShadowSettings> shadowSettings = actor->GetComponent<ShadowSettings>();
				if (shadowSettings) shadowSettings->setCastShadow(castShadow);
			}
		}
	}

	ImGui::Separator();
}

void InspectorWindow::DrawMaterialComponent(const std::unordered_map<uint32_t, Ref<Actor>>& selectedActors_)
{
	ComponentState<MaterialComponent> materialState(selectedActors_);

	if (materialState.noneHaveComponent) return;

	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
		RightClickContext<MaterialComponent>("##MaterialPopup", selectedActors_);

		if (materialState.allHaveComponent && materialState.Count() == 1) {
			Ref<MaterialComponent> material = materialState.GetFirst();

			std::string assetName = AssetManager::getInstance().GetAssetName(material);
			if (!assetName.empty()) ImGui::TextWrapped("Name: %s", assetName.c_str());
			if (material->getSpecularID() != 0) ImGui::TextWrapped("Has Specular");
			if (material->getNormalID() != 0) ImGui::TextWrapped("Has Normal");
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
			if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
				if (std::string(data->componentType) == "MaterialComponent") {
					auto newMat = AssetManager::getInstance().GetAsset<MaterialComponent>(data->assetName);
					if (newMat) {
						for (auto& [id, actor] : selectedActors_) {
							actor->ReplaceComponent<MaterialComponent>(newMat);
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

	}

	if (materialState.Count() == 1) {
		ImGui::Separator();

		for (const auto& [id, actor] : selectedActors_) {
			if (actor->GetComponent<MaterialComponent>() && !actor->GetComponent<TilingSettings>()) {
				actor->AddComponent<TilingSettings>(actor.get(), false);
			}
		}

		Ref<TilingSettings> tileSettings;
		for (const auto& [id, actor] : selectedActors_) {
			tileSettings = actor->GetComponent<TilingSettings>();
			if (tileSettings) break;
		}

		if (tileSettings) {
			bool isTiled = tileSettings->getIsTiled();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Tiling");
			ImGui::SameLine(labelWidth + 20);
			if (ImGui::Checkbox("##isTiled", &isTiled)) {
				tileSettings->setIsTiled(isTiled);
			}

			if (isTiled) {
				Vec2 scale = tileSettings->getTileScale();
				float tileScale[2] = { scale.x, scale.y };
				Vec2 offset = tileSettings->getTileOffset();
				float tileOffset[2] = { offset.x, offset.y };

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Tile Scale");
				ImGui::SameLine(labelWidth + 20);
				ImGui::SetNextItemWidth(-1);
				if (ImGui::DragFloat2("##tileScale", tileScale, 0.01f)) {
					tileSettings->setTileScale(Vec2(tileScale[0], tileScale[1]));
				}
				ImGui::ActiveItemLockMousePos();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Tile Offset");
				ImGui::SameLine(labelWidth + 20);
				ImGui::SetNextItemWidth(-1);
				if (ImGui::DragFloat2("##tileOffset", tileOffset, 0.01f)) {
					tileSettings->setTileOffset(Vec2(tileOffset[0], tileOffset[1]));
				}
				ImGui::ActiveItemLockMousePos();
			}
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

		ProjectionType type = camera->getType();
		float fov = camera->getFOV();
		float nearClipPlane = camera->getNearClipPlane(), farClipPlane = camera->getFarClipPlane();
		float orthoSize = camera->getOrthoSize();

		int fovInt = static_cast<int>(fov);

		bool hasMixedFOV = cameraState.HasMixedFloat(&CameraComponent::getFOV);
		bool hasMixedNear = cameraState.HasMixedFloat(&CameraComponent::getNearClipPlane);
		bool hasMixedFar = cameraState.HasMixedFloat(&CameraComponent::getFarClipPlane);
		bool hasMixedOrthoSzie = cameraState.HasMixedFloat(&CameraComponent::getOrthoSize);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Projection");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

		// Projection type
		ProjectionType currentType = cameraState.GetFirst()->getType();

		// creating a dropdown
		const char* typeModes[] = { "Perspective", "Orthographic" };
		int typeIndex = static_cast<int>(currentType);

		// dropdown for types
		if (ImGui::Combo("##ProjectionTypes", &typeIndex, typeModes, 2)) {
			ProjectionType newType = static_cast<ProjectionType>(typeIndex);

			for (auto& component : cameraState.components) {
				component->setType(newType);
			}
		}
		
		if (cameraState.GetFirst()->getType() == ProjectionType::Perspective) {
			ImGui::AlignTextToFramePadding();
			ImGui::Text("FOV");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			if (ImGui::SliderInt("##fovslider", &fovInt, 1, 120, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
				for (auto& component : cameraState.components) {
					component->setFOV((float)fovInt);
				}
			}
		}

		if (cameraState.GetFirst()->getType() == ProjectionType::Orthographic) {
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Size");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			if (ImGui::SliderFloat("##orthoslider", &orthoSize, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
				for (auto& component : cameraState.components) {
					component->setOrthoSize(orthoSize);
				}
			}
		}

		ImGui::Text("Clipping Planes");
		
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Near");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);
		if (ImGui::DragFloat("##NearDrag", &nearClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
			for (auto& component : cameraState.components) {
				component->setNearClipPlane(nearClipPlane);
			}
		}

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Far");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);
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
				RightClickContext<ScriptComponent>("##ScriptPopup", selectedActors_, id-1);

				ImGui::TextWrapped("Script: %s", AssetManager::getInstance().GetAssetName(script->getBaseAsset()).c_str());

				ImGui::Button("Drop New Script Here", ImVec2(-1, 0));
				if (ImGui::BeginDragDropTarget()) {
					if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
						auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
						if (std::string(data->componentType) == "ScriptAbstract") {
							auto newScript = AssetManager::getInstance().GetAsset<ScriptAbstract>(data->assetName);
							if (newScript) script->setFilenameFromAbstract(newScript);
						}
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
			RightClickContext<AnimatorComponent>("##AnimatorPopup", selectedActors_);

			ImGuiChildFlags detailsFlags = ImGuiChildFlags_Borders;
			if (ImGui::BeginChild("details_panel", ImVec2(0, 150), detailsFlags)) {
				if (ImGui::CollapsingHeader("Animation Clip")) {




					float clipLength = animator->getClipLength();
					float startTime = animator->getStartTime();
					float speedMult = animator->getSpeedMult();
					float currentTime = animator->getCurrentTime();

					bool isEditingAnim = false;

					//Current Time
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Current Time");
						ImGui::SameLine(labelWidth + 30);
						ImGui::SetNextItemWidth(-1);
						bool currentTimeChanged = false;

						if (clipLength >= 0.0f) {

							char displayTimeRatioBuffer[64];
							std::snprintf(displayTimeRatioBuffer, sizeof(displayTimeRatioBuffer), "%%.3f / %.3f", clipLength);

							currentTimeChanged = ImGui::DragFloat("##CurrentTime", &currentTime,
								0.1f, 0.0f, clipLength, displayTimeRatioBuffer, ImGuiSliderFlags_AlwaysClamp);

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
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Start Time");
						ImGui::SameLine(labelWidth + 30);
						ImGui::SetNextItemWidth(-1);
						bool startTimeChanged = false;

						if (clipLength >= 0.0f) {

							char displayTimeRatioBuffer[64];
							std::snprintf(displayTimeRatioBuffer, sizeof(displayTimeRatioBuffer), "%%.3f / %.3f", clipLength);

							startTimeChanged = ImGui::DragFloat("##StartTime", &startTime,
								0.1f, 0.0f, clipLength, displayTimeRatioBuffer, ImGuiSliderFlags_AlwaysClamp);

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

						ImGui::AlignTextToFramePadding();
						ImGui::Text("Speed Multi");
						ImGui::SameLine(labelWidth + 30);
						ImGui::SetNextItemWidth(-1);
						speedMultChanged = ImGui::DragFloat("##SpeedMult", &speedMult,
							0.1f, -10, 10, nullptr, ImGuiSliderFlags_AlwaysClamp);
						speedMult = std::clamp(speedMult, -10.0f, 10.0f);
						if (speedMultChanged) {
							animator->setSpeedMult(speedMult);
						}

						isEditingAnim = ImGui::IsItemActive();
						ImGui::ActiveItemLockMousePos();
					}

					//LOOPING
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Loop");
						ImGui::SameLine(labelWidth + 30);
						ImGui::SetNextItemWidth(-1);

						bool isLooping = animator->getLoopingState();
						if (ImGui::Checkbox("##Looping", &isLooping)) {
							animator->setLooping(isLooping);
						}
					}

					/*ImGui::TextWrapped("Script Path: %s", script->getPath().c_str());*/


					ImGui::AlignTextToFramePadding();
					ImGui::Text("Name");
					ImGui::SameLine(labelWidth + 30);
					ImGui::SetNextItemWidth(-1);
					if (animator->hasAnim()) {
						std::string displayName = AssetManager::getInstance().GetAssetName(animator->getAnimationClip().getAnim());
						if (displayName.empty()) displayName = animator->getAnimName(); // fallback
						ImGui::Text("%s", displayName.c_str());
					}
					else {
						ImGui::TextDisabled("(none)");
					}

					ImGui::Button("Drop Animation Here", ImVec2(-1, 0));


					if (ImGui::BeginDragDropTarget()) {
						if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
							auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
							if (std::string(data->componentType) == "Animation") {
								auto newAnim = AssetManager::getInstance().GetAsset<Animation>(data->assetName);
								if (newAnim) animator->setAnimation(newAnim);
							}
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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Specular");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);
		
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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Diffuse");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Intensity");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Type");
		ImGui::SameLine(labelWidth);
		ImGui::SetNextItemWidth(-1);

		// Light Type 
		LightType currentType = lightState.GetFirst()->getType();

		// creating a dropdown, tracking current type name
		const char* typeModes[] = { "Sky", "Point" };
		int typeIndex = static_cast<int>(currentType);

		// dropdown for types
		if (ImGui::Combo("##LightTypes", &typeIndex, typeModes, 2)) {
			LightType newType = static_cast<LightType>(typeIndex);

			for (auto& component : lightState.components) {
				component->setType(newType);
			}
		}

		// Shadow settings for light component		
		if (ImGui::TreeNode("Shadows")) {

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Shadow Type");
			ImGui::SameLine(labelWidth + 60);
			ImGui::SetNextItemWidth(-1);

			// Shadow Type 
			ShadowType currentShadowType = light->getShadowType();

			// creating a dropdown, tracking current type name
			const char* shadowTypeModes[] = { "None", "Hard" };
			int shadowTypeIndex = static_cast<int>(currentShadowType);

			// dropdown for types
			if (ImGui::Combo("##ShadowTypes", &shadowTypeIndex, shadowTypeModes, 2)) {
				ShadowType newShadowType = static_cast<ShadowType>(shadowTypeIndex);

				for (auto& component : lightState.components) {
					component->setShadowType(newShadowType);
				}
			}

			if (light->getShadowType() != ShadowType::None) {
				float shadowNear = light->getShadowNear();
				float shadowFar = light->getShadowFar();
				int shadowResolution = light->getShadowResolution();
				float shadowOrthoSize = light->getShadowOrthoSize();
				
				// Resolution
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Resolution");
				ImGui::SameLine(labelWidth + 60);
				ImGui::SetNextItemWidth(-1);

				const int   resolutionOptions[] = { 256, 512, 1024 };
				const char* resolutionLabels[] = { "256", "512", "1024" };
				int resolutionId = 2;
				for (int i = 0; i < 3; i++) if (resolutionOptions[i] == shadowResolution) resolutionId = i;
				if (ImGui::Combo("##ShadowResolution", &resolutionId, resolutionLabels, 3)) {
					int newShadowResolution = resolutionOptions[resolutionId];

					for (auto& component : lightState.components) {
						component->setShadowResolution(newShadowResolution);
					}
				}

				// Near
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Near Plane");
				ImGui::SameLine(labelWidth + 60);
				ImGui::SetNextItemWidth(-1);

				if (ImGui::DragFloat("##ShadowNear", &shadowNear, 0.1f, 0.0001f, 10.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
					for (auto& component : lightState.components) {
						component->setShadowNear(shadowNear);
					}
				}
				ImGui::ActiveItemLockMousePos();

				// Far
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Far Plane");
				ImGui::SameLine(labelWidth + 60);
				ImGui::SetNextItemWidth(-1);

				if (ImGui::DragFloat("##ShadowFar", &shadowFar, 0.1f, 1.0f, 1000.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
					for (auto& component : lightState.components) {
						component->setShadowFar(shadowFar);
					}
				}
				ImGui::ActiveItemLockMousePos();

				// Size
				if (light->getType() == LightType::Sky) {
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Shadow Size");
					ImGui::SameLine(labelWidth + 60);
					ImGui::SetNextItemWidth(-1);

					if (ImGui::DragFloat("##ShadowSize", &shadowOrthoSize, 1.0f, 1.0f, 500.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
						for (auto& component : lightState.components) {
							component->setShadowOrthoSize(shadowOrthoSize);
						}
					}
					ImGui::ActiveItemLockMousePos();
				}
			}
			
			ImGui::TreePop();
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
			std::string assetName = AssetManager::getInstance().GetAssetName(shader);
			if (!assetName.empty()) ImGui::TextWrapped("Name: %s", assetName.c_str());
			if (shader->GetVertName() && strlen(shader->GetVertName()) > 0) ImGui::TextWrapped("Vert: %s", fs::path(shader->GetVertName()).filename().string().c_str());
			if (shader->GetFragName() && strlen(shader->GetFragName()) > 0) ImGui::TextWrapped("Frag: %s", fs::path(shader->GetFragName()).filename().string().c_str());
			if (shader->GetGeomName() && strlen(shader->GetGeomName()) > 0) ImGui::TextWrapped("Geom: %s", fs::path(shader->GetGeomName()).filename().string().c_str());
		}
		else if (shaderState.allHaveComponent) {
			ImGui::TextWrapped("All Selected Actors Have a ShaderComponent");
		}
		else if (shaderState.someHaveComponent) {
			ImGui::TextWrapped("Some Selected Actors Have a ShaderComponent");
		}


		ImGui::Button("Drop New Shader Here", ImVec2(-1, 0));
		if (ImGui::BeginDragDropTarget()) {
			if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
				if (std::string(data->componentType) == "ShaderComponent") {
					auto newShader = AssetManager::getInstance().GetAsset<ShaderComponent>(data->assetName);
					if (newShader) {
						for (auto& [id, actor] : selectedActors_) {
							actor->ReplaceComponent<ShaderComponent>(newShader);
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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("State");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

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
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Mass");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool hasMixedMass = physicsState.HasMixedFloat(&PhysicsComponent::getMass);
			float displayMass = physicsState.GetFirst()->getMass();
			float mass = displayMass;

			if (hasMixedMass && !isEditingMass) {
				ImGui::DragFloat("##Mass", &mass, 0.1f, 0.0f, 100.0f, "---", ImGuiSliderFlags_AlwaysClamp);
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##Mass", &mass, 0.1f, 0.1f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
					for (auto& component : physicsState.components) {
						component->setMass(mass);
					}
				}
			}

			isEditingMass = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();

			// Drag
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Drag");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool hasMixedDrag = physicsState.HasMixedFloat(&PhysicsComponent::getDrag);
			float displayDrag = physicsState.GetFirst()->getDrag();
			float drag = displayDrag;

			if (hasMixedDrag && !isEditingDrag) {
				ImGui::DragFloat("##Drag", &drag, 0.1f, 0.0f, 2.0f, "---", ImGuiSliderFlags_AlwaysClamp);
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##Drag", &drag, 0.1f, 0.1f, 2.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
					for (auto& component : physicsState.components) {
						component->setDrag(drag);
					}
				}
			}

			isEditingDrag = ImGui::IsItemActive();
			ImGui::ActiveItemLockMousePos();

			// Angular Drag
			/*
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
			*/

			// Apply gravity
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Use Gravity");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool useGravity = physicsState.GetFirst()->getUseGravity();

			if (ImGui::Checkbox("##UseGrav", &useGravity)) {
				for (auto& component : physicsState.components) {
					component->setUseGravity(useGravity);
				}
			}
		}

		// Friction
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Friction");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

		bool hasMixedFriction = physicsState.HasMixedFloat(&PhysicsComponent::getFriction);
		float displayFriction = physicsState.GetFirst()->getFriction();
		float friction = displayFriction;

		if (hasMixedFriction && !isEditingFriction) {
			ImGui::DragFloat("##Friction", &friction, 0.01f, 0.0f, 1.0f, "---", ImGuiSliderFlags_AlwaysClamp);
		}
		else { // no delta (might add later)
			if (ImGui::DragFloat("##Friction", &friction, 0.01f, 0.1f, 1.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
				for (auto& component : physicsState.components) {
					component->setFriction(friction);
				}
			}
		}

		isEditingFriction = ImGui::IsItemActive();
		ImGui::ActiveItemLockMousePos();

		// Restitution 
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Restitution");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

		bool hasMixedRestitution = physicsState.HasMixedFloat(&PhysicsComponent::getRestitution);
		float displayRestitution = physicsState.GetFirst()->getRestitution();
		float restitution = displayRestitution;

		if (hasMixedRestitution && !isEditingRestitution) {
			ImGui::DragFloat("##Restitution", &restitution, 0.01f, 0.0f, 1.0f, "---", ImGuiSliderFlags_AlwaysClamp);
		}
		else { // no delta (might add later)
			if (ImGui::DragFloat("##Restitution", &restitution, 0.01f, 0.0f, 1.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Position");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool freezePosX = constraints.freezePosX, freezePosY = constraints.freezePosY, freezePosZ = constraints.freezePosZ;
			if (ImGui::Checkbox("X##Pos", &freezePosX)) {
				constraints.freezePosX = freezePosX;
				changed = true;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Checkbox("Y##Pos", &freezePosY)) {
				constraints.freezePosY = freezePosY;
				changed = true;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Checkbox("Z##Pos", &freezePosZ)) {
				constraints.freezePosZ = freezePosZ;
				changed = true;
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Rotation");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool freezeRotX = constraints.freezeRotX, freezeRotY = constraints.freezeRotY, freezeRotZ = constraints.freezeRotZ;
			if (ImGui::Checkbox("X##Rot", &freezeRotX)) {
				constraints.freezeRotX = freezeRotX;
				changed = true;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Checkbox("Y##Rot", &freezeRotY)) {
				constraints.freezeRotY = freezeRotY;
				changed = true;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
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
		RightClickContext<CollisionComponent>("##CollisionPopup", selectedActors_);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Shape");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

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

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Detection");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

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
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Is Trigger");
		ImGui::SameLine(labelWidth + 20);
		ImGui::SetNextItemWidth(-1);

		bool isTrigger = collisionState.GetFirst()->getIsTrigger();

		if (ImGui::Checkbox("##isTrigger", &isTrigger)) {
			for (auto& component : collisionState.components) {
				component->setIsTrigger(isTrigger);
			}
		}

		if (collisionState.GetFirst()->getType() == ColliderType::Sphere || collisionState.GetFirst()->getType() == ColliderType::Capsule) {
			// Radius
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Radius");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

			bool hasMixedRadius = collisionState.HasMixedFloat(&CollisionComponent::getRadius);
			float displayRadius = collisionState.GetFirst()->getRadius();
			float radius = displayRadius;

			if (hasMixedRadius && !isEditingRadius) {
				ImGui::DragFloat("##Radius", &radius, 0.1f, 0.0f, 100.0f, "---", ImGuiSliderFlags_AlwaysClamp);
			}
			else { // no delta (might add later)
				if (ImGui::DragFloat("##Radius", &radius, 0.1f, 0.1f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Centre");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("CentrePosA");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("CentrePosB");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("HalfExtents");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

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

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Orientation");
			ImGui::SameLine(labelWidth + 20);
			ImGui::SetNextItemWidth(-1);

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

void InspectorWindow::DrawAssetInspector(const EditorManager::SelectedAsset& asset)
{
	// dispatcher for asset types

	ImGui::TextWrapped("Asset: %s", asset.assetName.c_str());
	ImGui::TextWrapped("Path: %s", SearchPath::getInstance().MakeRelative(asset.absolutePath).string().c_str());
	ImGui::Separator();

	std::string ext = asset.absolutePath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".mat") DrawMatManifestEditor(asset);
	else if (ext == ".shader") DrawShaderManifestEditor(asset);
	else if (ext == ".prefab") DrawPrefabEditor(asset);
	else if (asset.componentType == "MeshComponent") {
		auto mesh = AssetManager::getInstance().GetAsset<MeshComponent>(asset.assetName);
		if (mesh) {
			ImGui::TextWrapped("Vertices: %zu", mesh->getVertices());
			ImGui::TextWrapped("Skeleton: %s", mesh->hasSkeleton() ? "Yes" : "No");
			if (mesh->hasSkeleton()) ImGui::TextWrapped("Bones: %zu", mesh->getBoneCount());
		}
	}
	else if (asset.componentType == "Animation") {
		auto anim = AssetManager::getInstance().GetAsset<Animation>(asset.assetName);
		if (anim && anim->queryLoadStatus()) {
			ImGui::TextWrapped("Duration: %.2f ticks", anim->getDuration());
			ImGui::TextWrapped("Ticks/s: %.2f", anim->getTicksPerSecond());
		}
		else {
			ImGui::TextDisabled("(loading...)");
		}
	}
	else if (asset.componentType == "ScriptAbstract") {
		if (ImGui::Button("Open in Editor")) {
			std::string cmd = "start \"\" \"" + asset.absolutePath.string() + "\"";
			system(cmd.c_str());
		}
	}
}

void InspectorWindow::DrawMatManifestEditor(const EditorManager::SelectedAsset& asset)
{
	static std::string diff, spec, norm;
	static fs::path loaded;
	static bool refresh = false;

	// read the mat 
	if (loaded != asset.absolutePath) {
		diff.clear(); spec.clear(); norm.clear(); refresh = false;
		XMLDocument doc;
		if (doc.LoadFile(asset.absolutePath.string().c_str()) == XML_SUCCESS) {
			auto* root = doc.FirstChildElement("Material");
			if (root) {
				// lambda to help read for values
				auto rd = [&](const char* tag, std::string& out) {
					auto* el = root->FirstChildElement(tag);
					if (el && el->GetText()) out = el->GetText();
					};
				rd("Diffuse", diff); rd("Specular", spec); rd("Normal", norm);
			}
		}

		loaded = asset.absolutePath;
	}

	// lambda to use for diffuse, specular, and normal,
	// uses inputtext (can't actually inputtext) as a drop off point for assets
	auto slot = [&](const char* label, std::string& rel, bool required) {
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%-10s", label);
		ImGui::SameLine(labelWidth + 20.0f);

		std::string display = rel.empty() ? "(none)" : fs::path(rel).filename().string();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (required ? 0.0f : 26.0f));
		ImGui::InputText((std::string("##") + label).c_str(), &display, ImGuiInputTextFlags_ReadOnly);

		if (ImGui::BeginDragDropTarget()) {
			if (auto* p = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* d = static_cast<const EditorManager::ProjectDragPayload*>(p->Data);
				std::string dropExt = fs::path(d->absolutePath).extension().string();
				std::transform(dropExt.begin(), dropExt.end(), dropExt.begin(), ::tolower);
				if (dropExt == ".png" || dropExt == ".jpg" || dropExt == ".jpeg") {
					rel = SearchPath::getInstance().MakeRelative(fs::path(d->absolutePath)).string();
					std::replace(rel.begin(), rel.end(), '\\', '/');
					refresh = true;
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (!required) {
			ImGui::SameLine();
			if (ImGui::SmallButton((std::string("x##") + label).c_str())) {
				rel.clear();
				refresh = true;
			}
		}
		};

	slot("Diffuse", diff, true);
	slot("Specular", spec, false);
	slot("Normal", norm, false);

	if (!diff.empty()) {
		fs::path abs = SearchPath::getInstance().Resolve(diff);
		if (!abs.empty() && AssetManager::getInstance().HasAsset<MaterialComponent>(abs.stem().string())) {
			auto mat = AssetManager::getInstance().GetAsset<MaterialComponent>(abs.stem().string());
			if (mat && mat->getDiffuseID() != 0) {
				ImGui::Image((ImTextureID)(intptr_t)mat->getDiffuseID(), ImVec2(48, 48), ImVec2(0, 0), ImVec2(1, 1));
			}
		}
	}

	ImGui::Separator();

	ImGui::BeginDisabled(!refresh);
	if (ImGui::Button("Apply")) {
		XMLObjectFile::WriteMatManifest(asset.absolutePath, diff, spec, norm);
		AssetManager::getInstance().RefreshSingle(asset.absolutePath);
		refresh = false;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Revert")) { loaded.clear(); refresh = false; }

	if (ImGui::Button("Open File")) {
		std::string cmd = "start \"\" \"" + asset.absolutePath.string() + "\"";
		system(cmd.c_str());
	}
}

void InspectorWindow::DrawShaderManifestEditor(const EditorManager::SelectedAsset& asset)
{
	static std::string vert, frag, tc, te, geom;
	static fs::path loaded;
	static bool refresh = false;
	static bool compiled = false;

	// read the shader
	if (loaded != asset.absolutePath) {
		vert.clear(); frag.clear(); tc.clear(); te.clear(); geom.clear();
		refresh = false;
		XMLDocument doc;
		if (doc.LoadFile(asset.absolutePath.string().c_str()) == XML_SUCCESS) {
			auto* root = doc.FirstChildElement("Shader");
			if (root) {
				// lambda to help read for values
				auto rd = [&](const char* tag, std::string& out) {
					auto* el = root->FirstChildElement(tag);
					if (el && el->GetText()) out = el->GetText();
					};
				rd("Vertex", vert); rd("Fragment", frag);
				rd("TessControl", tc); rd("TessEval", te); rd("Geometry", geom);
			}
		}

		compiled = AssetManager::getInstance().HasAsset<ShaderComponent>(asset.assetName);
		loaded = asset.absolutePath;
	}

	// lambda for the different shader values
	auto slot = [&](const char* label, std::string& rel, bool required) {
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%-12s", label);
		ImGui::SameLine(labelWidth + 20.0f);

		std::string display = rel.empty() ? "(none)" : fs::path(rel).filename().string();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (required ? 0.0f : 26.0f));
		ImGui::InputText((std::string("##") + label).c_str(), &display, ImGuiInputTextFlags_ReadOnly);

		if (ImGui::BeginDragDropTarget()) {
			if (auto* p = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* d = static_cast<const EditorManager::ProjectDragPayload*>(p->Data);
				std::string ext = fs::path(d->absolutePath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (ext == ".glsl") {
					rel = SearchPath::getInstance().MakeRelative(fs::path(d->absolutePath)).string();
					std::replace(rel.begin(), rel.end(), '\\', '/');
					refresh = true;
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (!required) {
			ImGui::SameLine();
			if (ImGui::SmallButton((std::string("x##") + label).c_str())) {
				rel.clear(); 
				refresh = true;
			}
		}
		};

	slot("Vertex", vert, true);
	slot("Fragment", frag, true);
	slot("Tess Ctrl", tc, false);
	slot("Tess Eval", te, false);
	slot("Geometry", geom, false);

	ImGui::Separator();

	bool canCompile = !vert.empty() && !frag.empty();
	ImVec4 statusColor = compiled ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f) : ImVec4(0.9f, 0.4f, 0.4f, 1.0f);
	ImGui::TextColored(statusColor, "Status: %s", compiled ? "Compiled" : "Not compiled");
	if (!canCompile) ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "Vertex and Fragment required.");

	ImGui::BeginDisabled(!refresh || !canCompile);
	if (ImGui::Button("Apply")) {
		XMLObjectFile::WriteShaderManifest(asset.absolutePath, vert, frag, tc, te, geom);
		AssetManager::getInstance().RefreshSingle(asset.absolutePath);
		compiled = AssetManager::getInstance().HasAsset<ShaderComponent>(asset.assetName);
		refresh = false;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Revert")) { loaded.clear(); refresh = false; }

	if (ImGui::Button("Open File")) {
		std::string cmd = "start \"\" \"" + asset.absolutePath.string() + "\"";
		system(cmd.c_str());
	}
}

void InspectorWindow::DrawPrefabEditor(const EditorManager::SelectedAsset& asset)
{
	// creates dummy actors from the prefab

	if (loadedPrefabPath != asset.absolutePath) {
		if (!dummyActors.empty()) {
			ClosePrefabEditor(true);
		}

		std::vector<Ref<Actor>> built;
		Ref<Actor> root = XMLObjectFile::ReadPrefab(asset.absolutePath, built);
		if (!root) {
			ImGui::Text("Failed to load prefab.");
			return;
		}

		if (auto tc = root->GetComponent<TransformComponent>()) {
			tc->SetTransform(Vec3(0, 0, 0),
				Quaternion(1, Vec3(0, 0, 0)), tc->GetScale());
		}

		for (auto& a : built) {
			std::string uniqueName = SceneGraph::getInstance().GenerateUniqueActorName(a->getActorName());
			a->setActorName(uniqueName);
			SceneGraph::getInstance().AddActor(a);
			dummyActors.push_back(a);
		}

		loadedPrefabPath = asset.absolutePath;
		refresh = false;
	}

	if (dummyActors.empty()) {
		ImGui::Text("Failed to load prefab.");
		return;
	}

	Ref<Actor> root = dummyActors[0];

	ImGui::Text("Prefab: %s", asset.assetName.c_str());
	ImGui::Separator();

	std::unordered_map<uint32_t, Ref<Actor>> fakeSelected;
	fakeSelected[root->getId()] = root;

	DrawTransformComponent(fakeSelected);
	DrawMeshComponent(fakeSelected);
	DrawMaterialComponent(fakeSelected);
	DrawShaderComponent(fakeSelected);
	DrawLightComponent(fakeSelected);
	DrawCameraComponent(fakeSelected);
	DrawPhysicsComponent(fakeSelected);
	DrawCollisionComponent(fakeSelected);
	DrawAnimatorComponent(fakeSelected);
	DrawScriptComponent(fakeSelected);

	if (ImGui::IsAnyItemActive()) refresh = true;

	ImGui::Separator();

	if (refresh && !ImGui::IsAnyItemActive()) {
		XMLObjectFile::WritePrefab(root->getActorName(), asset.absolutePath);
		AssetManager::getInstance().RefreshSingle(asset.absolutePath);
		refresh = false;
	}

	ImGui::Separator();
	if (ImGui::Button("Instantiate in Scene")) {
		SceneGraph::getInstance().InstantiatePrefab(asset.absolutePath);
	}

	if (ImGui::Button("Done (save and close)")) {
		ClosePrefabEditor(true);
		EditorManager::getInstance().ClearSelectedAsset();
	}
}

void InspectorWindow::ClosePrefabEditor(bool save)
{
	if (dummyActors.empty()) return;

	if (save && !loadedPrefabPath.empty()) {
		XMLObjectFile::WritePrefab(dummyActors[0]->getActorName(), loadedPrefabPath);
		AssetManager::getInstance().RefreshSingle(loadedPrefabPath);
	}

	for (int i = (int)dummyActors.size() - 1; i >= 0; i--) {
		SceneGraph::getInstance().RemoveActor(dummyActors[i]->getActorName());
	}
	dummyActors.clear();
	loadedPrefabPath.clear();
	refresh = false;
}
