#include "InspectorWindow.h"
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

			/// expand section later on for new features like actor renaming 
			ImGui::Text(selectedActor->first.c_str());

			ImGui::Separator();

			/// Components Section
			// TODO: PhysicsComponent, CollisionComponent + CollisionSystem


			// transform
			if (selectedActor->second->GetComponent<TransformComponent>()) {
				DrawTransformComponent(selectedActor->second->GetComponent<TransformComponent>());
				ImGui::Separator();
			}

			// mesh
			if (selectedActor->second->GetComponent<MeshComponent>()) {
				DrawMeshComponent(selectedActor->second->GetComponent<MeshComponent>());
				ImGui::Separator();
			}

			// material
			if (selectedActor->second->GetComponent<MaterialComponent>()) {
				DrawMaterialComponent(selectedActor->second->GetComponent<MaterialComponent>());
				ImGui::Separator();
			}

			// shader
			if (selectedActor->second->GetComponent<ShaderComponent>()) {
				DrawShaderComponent(selectedActor->second->GetComponent<ShaderComponent>());
				ImGui::Separator();
			}

			// TODO: adding and removing components, drop down selection for certain components

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

		ImGui::Text("Scale   ");
		ImGui::SameLine();
		if (ImGui::DragFloat3("##Scale", scale, 0.1f, 0.1f, 100.0f)) {
			transform->SetTransform(Vec3(scale[0], scale[1], scale[2]));
		}


		// right click popup menu 
		if (ImGui::BeginPopup("##TransformPopup")) {
			if (ImGui::MenuItem("Reset")) {
				transform->SetTransform(Vec3(0, 0, 0), Quaternion(1, Vec3(0, 0, 0)), Vec3(1.0f, 1.0f, 1.0f));
			}



			ImGui::EndPopup();
		}


	}
}

void InspectorWindow::DrawMeshComponent(Ref<MeshComponent> mesh)
{
	if (ImGui::CollapsingHeader("Mesh")) {
		// displaying some basic mesh information
		ImGui::TextWrapped("Mesh Name: %s", mesh->getMeshName());
		ImGui::TextWrapped("Mesh Vertices: %zu", mesh->getVertices());


		ImGui::Button("Drop New Asset Here ##Mesh");
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

void InspectorWindow::DrawShaderComponent(Ref<ShaderComponent> shader)
{
	if (ImGui::CollapsingHeader("Shader")) {
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

