#include "pch.h"
#include "AssetManagerWindow.h"
#include "EditorManager.h"
#include <cstdlib> // Required for system()
AssetManagerWindow::AssetManagerWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
	EditorManager::getInstance().RegisterWindow("AssetManager", true);
}

void AssetManagerWindow::ShowAssetManagerWindow(bool* pOpen)
{
	if (ImGui::Begin("Asset Manager", pOpen, ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save Assets To XML")) {
					AssetManager::getInstance().SaveAssetDatabaseXML();
				}
				if (ImGui::MenuItem("Load Assets From XML")) {
					AssetManager::getInstance().LoadAssetDatabaseXML();
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options")) {
				if (ImGui::MenuItem("Add New Asset")) {
					showAddAssetDialog = true;
				}
				if (ImGui::MenuItem("Reload All Assets")) {
					AssetManager::getInstance().ReloadAssetsXML();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// render the filter
		assetFilter.Draw("##AssetFilter", -1.0f);

		std::vector<Ref<Component>> allAssets = AssetManager::getInstance().GetAllAssets();
		std::vector<std::string> allAssetNames = AssetManager::getInstance().GetAllAssetNames();

		ImGui::Text("Total Assets: %zu", allAssets.size());

		//imgui_demo.cpp Asset Browser
		if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove)) {

			// calculating the layout for the assets
			const float availWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = std::max(1, static_cast<int>(availWidth / (thumbnailSize + padding)));

			// putting all the assets into columns to get a grid style layout
			ImGui::Columns(columnCount, "##AssetGrid", false);

			for (size_t i = 0; i < allAssets.size(); i++) {
				const std::string& assetName = allAssetNames[i];
				Ref<Component> asset = allAssets[i];

				// if asset name isn't in the filter, add it
				if (!assetFilter.PassFilter(assetName.c_str())) {
					continue;
				}

				// create and draw the asset
				DrawAssetThumbnail(assetName, asset);


				// next asset
				ImGui::NextColumn();
			}
			//reset
			ImGui::Columns(1);

		}
		ImGui::EndChild();

	}
	ImGui::End();

	// handler
	ShowAddAssetDialog();
}

void AssetManagerWindow::ShowAddAssetDialog()
{
	if (showAddAssetDialog) {
		ImGui::OpenPopup("Add New Asset");
		showAddAssetDialog = false;
	}

	// sets the placement and size of the dialog box
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

	// a popup modal makes it so you can't interact with other things in the scene
	if (ImGui::BeginPopupModal("Add New Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		// TODO: more asset types go here
		
		// in the popup, pick the type of asset you want to add (using a simple combo here for selection)
		ImGui::Text("Asset Type:");
		const char* assetTypes[] = { "MeshComponent", "MaterialComponent", "ShaderComponent", "ScriptAbstract", "Animation"};
		ImGui::Combo("##AssetType", &selectedAssetType, assetTypes, IM_ARRAYSIZE(assetTypes));

		ImGui::Separator();

		// saves whatever is inputted into the box to the asset name string
		ImGui::Text("Asset Name:");
		ImGui::InputText("##AssetName", &newAssetName);

		ImGui::Separator();

		// saves whatever is inputted into the box to the asset path string
		ImGui::Text("File Path:");

		if (selectedAssetType == 0) { // Mesh
			ImGui::InputText("##MeshPath", &newAssetPath);
			ImGui::TextWrapped("Example: meshes/Mesh.obj");
		}
		else if (selectedAssetType == 1) {
			ImGui::Text("Diffuse Map:");
			ImGui::InputText("##DiffusePath", &newDiffuseMapPath);
			ImGui::Text("Specular Map (Optional):");
			ImGui::InputText("##SpecularPath", &newSpecularMapPath);
			ImGui::Text("Normal Map (Optional):");
			ImGui::InputText("##NormalPath", &newNormalMapPath);
			ImGui::TextWrapped("Example: textures/Texture.png");
		}
		else if (selectedAssetType == 2) {
			ImGui::Text("Vertex Shader:");
			ImGui::InputText("##VertexPath", &newVertShaderPath);
			ImGui::Text("Fragment Shader:");
			ImGui::InputText("##FragmentPath", &newFragShaderPath);
			
			ImGui::TextWrapped("Example: shaders/Shader.vert & .frag");

			//TODO: rest of the shader types
		}
		else if (selectedAssetType == 3) {
			ImGui::Text("Script:");
			ImGui::InputText("##ScriptPath", &newScriptPath);

		}
		else if (selectedAssetType == 4) {
			ImGui::Text("Animation:");
			ImGui::InputText("##AnimationPath", &newAnimationPath);

		}
		
		ImGui::Separator();

		// checks to see if all the InputText fields are empty
		bool canAdd = !newAssetName.empty();
		if (selectedAssetType == 0) {
			canAdd = canAdd && !newAssetPath.empty();
		}
		else if (selectedAssetType == 1) {
			canAdd = canAdd && !newDiffuseMapPath.empty();
		}
		else if (selectedAssetType == 2) {
			canAdd = canAdd && !newVertShaderPath.empty() && !newFragShaderPath.empty();
		}
		else if (selectedAssetType == 3) {
			canAdd = canAdd && !newScriptPath.empty();
		}
		else if (selectedAssetType == 4) {
			canAdd = canAdd && !newAnimationPath.empty();
		}
		// disables button if empty
		if (!canAdd) {
			ImGui::BeginDisabled();
		}

		// if the asset can be added, clear all input fields and load the asset
		if (ImGui::Button("Add Asset") && canAdd) {
			if (AddNewAssetToDatabase()) {
				ResetInput();
			}
		}

		if (!canAdd) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			ResetInput();
		}

		// shows an error message if all fields are empty
		if (!canAdd) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Please enter the asset's name and filepath");
		}

		ImGui::EndPopup();
	}
}

void AssetManagerWindow::ResetInput()
{
	newAssetName.clear();
	newAssetPath.clear();
	newDiffuseMapPath.clear();
	newSpecularMapPath.clear();
	newNormalMapPath.clear();
	newVertShaderPath.clear();
	newFragShaderPath.clear();
	newScriptPath.clear();
	newAnimationPath.clear();
	selectedAssetType = 0;
	ImGui::CloseCurrentPopup();
}

bool AssetManagerWindow::AddNewAssetToDatabase()
{
	// exception handling so that the scene doesn't blow up if something goes wrong when trying to load an asset 
	try {
		bool success = false;

		switch (selectedAssetType) {
		case 0: // Mesh Component
			success = AssetManager::getInstance().LoadAsset<MeshComponent>(
				newAssetName, nullptr, newAssetPath.c_str());
			break;

		case 1: // Material Component
			if (newSpecularMapPath.empty()) {
				success = AssetManager::getInstance().LoadAsset<MaterialComponent>(
					newAssetName, nullptr, newDiffuseMapPath.c_str());
			} 
			else if (!newSpecularMapPath.empty() && newNormalMapPath.empty()) {
				success = AssetManager::getInstance().LoadAsset<MaterialComponent>(
					newAssetName, nullptr, newDiffuseMapPath.c_str(), newSpecularMapPath.c_str());
			} else {
				success = AssetManager::getInstance().LoadAsset<MaterialComponent>(
					newAssetName, nullptr, newDiffuseMapPath.c_str(), newSpecularMapPath.c_str(), newNormalMapPath.c_str());
			}

			break;

		case 2: // Shader Component
			success = AssetManager::getInstance().LoadAsset<ShaderComponent>(
				newAssetName, nullptr, newVertShaderPath.c_str(), newFragShaderPath.c_str());
			break;

		case 3: // Script Component
			success = AssetManager::getInstance().LoadAsset<ScriptAbstract>(
				newAssetName, nullptr, newScriptPath.c_str());
			break;
		case 4: // Animation
			success = AssetManager::getInstance().LoadAsset<Animation>(
				newAssetName, nullptr, newAnimationPath.c_str());
			break;

		default:
			Debug::Error("Unknown asset type selected", __FILE__, __LINE__);
			return false;
		}

		if (success) {
			Debug::Info("Successfully added asset: " + newAssetName, __FILE__, __LINE__);
		}

		return success;
	}
	catch (const std::exception& e) {
		Debug::Error("Error adding new asset: " + std::string(e.what()), __FILE__, __LINE__);
		return false;
	}
}

void AssetManagerWindow::DrawAssetThumbnail(const std::string& assetName, Ref<Component> asset)
{
	const ImVec2 buttonSize((float)thumbnailSize, (float)thumbnailSize);

	const char* payloadType = "";

	// giving each asset its own unique id
	ImGui::PushID(assetName.c_str());

	// colors
	ImVec4 meshColor(0.4f, 0.4f, 0.4f, 1.0f);
	ImVec4 shaderColor(0.4f, 0.4f, 0.6f, 1.0f);

	// pointers to assets
	auto mesh = std::dynamic_pointer_cast<MeshComponent>(asset);
	auto material = std::dynamic_pointer_cast<MaterialComponent>(asset);
	auto shader = std::dynamic_pointer_cast<ShaderComponent>(asset);
	auto script = std::dynamic_pointer_cast<ScriptAbstract>(asset);
	auto animation = std::dynamic_pointer_cast<Animation>(asset);


	// disable background for buttons
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	// TODO: better thumbnails for meshes and shaders
	if (mesh) {
		ImGui::ImageButton("##MeshDiffuseBtn", ImTextureID(EditorManager::getInstance().getEditorIcons().meshIcon->getDiffuseID()), buttonSize);
		payloadType = "MESH_ASSET";
	}
	else if (material) {
		ImGui::ImageButton("##MaterialDiffuseBtn", ImTextureID(material->getDiffuseID()), buttonSize);
		payloadType = "MATERIAL_ASSET";
	}
	else if (shader) {
		ImGui::ImageButton("##ShaderDiffuseBtn", ImTextureID(EditorManager::getInstance().getEditorIcons().shaderIcon->getDiffuseID()), buttonSize);
		payloadType = "SHADER_ASSET";
	}
	else if (script) {
		ImGui::ImageButton("##ScriptDiffuseBtn", ImTextureID(EditorManager::getInstance().getEditorIcons().shaderIcon->getDiffuseID()), buttonSize);
		payloadType = "SCRIPT_ASSET";
	}
	else if (animation) {
		ImGui::ImageButton("##AnimationDiffuseBtn", ImTextureID(EditorManager::getInstance().getEditorIcons().meshIcon->getDiffuseID()), buttonSize);
		payloadType = "ANIMATION_ASSET";
	}
	//TODO: if there are more components that need to go into the assetmanager later add them here

	// pop
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();


	// right click popup menu
	if (ImGui::BeginPopupContextItem("##AssetRightClick")) {
		if (ImGui::MenuItem("Delete Asset")) {
			if (mesh) {
				AssetManager::getInstance().RemoveAsset<MeshComponent>(assetName);
			}
			else if (material) {
				AssetManager::getInstance().RemoveAsset<MaterialComponent>(assetName);
			}
			else if (shader) {
				AssetManager::getInstance().RemoveAsset<ShaderComponent>(assetName);
			}
			else if (script) {
				AssetManager::getInstance().RemoveAsset<ScriptAbstract>(assetName);

			}
			else if (animation) {
				AssetManager::getInstance().RemoveAsset<Animation>(assetName);

			}
			// TODO if removing an asset that an actor is using, replace with a temporary asset so program doesn't crash
		}
		if (ImGui::MenuItem("Open File")) {
			if (script) {
//				std::string filePath = "scripts/" + std::string(script->getName());  // "scripts/testScript.lua"				// The '-n' flag opens a new window/instance of VS Code

				std::string command = "start \"\" \"scripts/" + std::string(script->getName()) + "\"";
				system(command.c_str());				
			}
			else if (material) {
				if (material->getDiffuseName() != "") {
					std::string command = "start \"\" \"" + std::string((material->getDiffuseName())) + "\"";
					system(command.c_str());
				}

				if (material->getSpecularName() != "") {
					std::string command = "start \"\" \"" + std::string((material->getSpecularName())) + "\"";
					system(command.c_str());

				}

			}
			else if (shader) {

				
				if (shader->GetFragName() != "") {
					std::string command = "start \"\" \"" + std::string((shader->GetFragName())) + "\"";
					system(command.c_str());
				}

				if (shader->GetVertName() != "") {
					std::string command = "start \"\" \"" + std::string((shader->GetVertName())) + "\"";
					system(command.c_str());

				}
				

			}
			else if (mesh) {


				if (mesh->getMeshName() != "") {
					std::string command = "start \"\" \"" + std::string((mesh->getMeshName())) + "\"";
					system(command.c_str());
				}

				


			}
			else if (animation) {
				if (animation->getName() != "") {
					std::string command = "start \"\" \"" + std::string((animation->getName())) + "\"";
					system(command.c_str());
				}
			}


		}
		ImGui::EndPopup();

		
	}
	ImGui::OpenPopupOnItemClick("##AssetRightClick", ImGuiPopupFlags_MouseButtonRight);

	// imgui_demo.cpp Widgets/Drag and drop
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

		// set payload to carry the index of the asset
		ImGui::SetDragDropPayload(payloadType, assetName.c_str(), assetName.length() + 1); // +1 because of null terminator

		// display preview
		ImGui::Text("Dragging: %s", assetName.c_str());

		ImGui::EndDragDropSource();
	}

	// tooltip to display some information about the asset that is selected
	if (ImGui::IsItemHovered() && !ImGui::IsMouseDown(0)) {
		ImGui::BeginTooltip();
		ImGui::Text("Name: %s", assetName.c_str());

		if (mesh) {
			ImGui::Text("Vertices: %zu", mesh->getVertices());
			ImGui::Text("File Path: %s", mesh->getMeshName());
		}
		if (material) {
			if (material->getSpecularName()) {
				ImGui::Text("Diffuse Texture ID: %u", material->getDiffuseID());
				ImGui::Text("Specular Texture ID: %u", material->getSpecularID());
				ImGui::Text("Diffuse File Path: %s", material->getDiffuseName());
				ImGui::Text("Specular File Path: %s", material->getSpecularName());
			} else {
				ImGui::Text("Diffuse Texture ID: %u", material->getDiffuseID());
				ImGui::Text("File Path: %s", material->getDiffuseName());
			}
		}
		if (shader) {
			ImGui::Text("Shader Program ID: %u", shader->GetProgram());
			ImGui::Text("Shader Vert: %s", shader->GetVertName());
			ImGui::Text("Shader Frag: %s", shader->GetFragName());
		}
		if (script) {
			ImGui::Text("Script Name: %s", script->getName());

		}
		if (animation) {
			ImGui::Text("Animation Name: %s", animation->getName());

		}

		ImGui::EndTooltip();
	}

	// adds an outline to a hovered asset (visual feedback)
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetItemRectMin();
	ImVec2 size = ImGui::GetItemRectSize();
	if (ImGui::IsItemHovered()) {
		draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
	}

	// asset name under asset
	ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);
	ImGui::Text("%s", assetName.c_str());
	ImGui::PopTextWrapPos();


	ImGui::PopID();
}
