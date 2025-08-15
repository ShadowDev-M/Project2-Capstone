#include "AssetManagerWindow.h"

AssetManagerWindow::AssetManagerWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {}

void AssetManagerWindow::ShowAssetManagerWindow(bool* p_open)
{
	if (ImGui::Begin("Asset Manager", p_open, ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {
				if (ImGui::MenuItem("Refresh")) {
					//
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// render the filter
		assetFilter.Draw("##AssetFilter", -1.0f);


		std::vector<Ref<Component>> allAssets = AssetManager::getInstance().GetAllAssets();
		std::vector<std::string> allAssetNames = AssetManager::getInstance().GetAllAssetNames();

		//imgui_demo.cpp Asset Browser
		if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove)) {

			// calculating the layout for the assets
			const float avail_width = ImGui::GetContentRegionAvail().x;
			int column_count = (avail_width / (thumbnail_size + padding));

			// putting all the assets into columns to get a grid style layout
			ImGui::Columns(column_count, "##AssetGrid", false);

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
}

void AssetManagerWindow::DrawAssetThumbnail(const std::string& assetName, Ref<Component> asset)
{
	const ImVec2 button_size(thumbnail_size, thumbnail_size);

	const char* payload_type = "";

	// giving each asset its own unique id
	ImGui::PushID(assetName.c_str());

	// TODO: possibly give meshes a 3d model thumbnail

	// colors
	ImVec4 mesh_color(0.4f, 0.4f, 0.4f, 1.0f);
	ImVec4 shader_color(0.4f, 0.4f, 0.6f, 1.0f);

	// pointers to assets
	auto mesh = std::dynamic_pointer_cast<MeshComponent>(asset);
	auto material = std::dynamic_pointer_cast<MaterialComponent>(asset);
	auto shader = std::dynamic_pointer_cast<ShaderComponent>(asset);

	/// I want to improve this section and remove the if statements, thinking of converting putting all the assets in the asset manger into an XML file which contains the already exisitng asset information + an easy way identify the type

	if (mesh) {
		// using a color button but disabling all of its functionalilty, just want to differentiate between assets
		ImGui::ColorButton("##MeshAssetBtn", mesh_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, button_size);
		payload_type = "MESH_ASSET";
	}
	else if (material) {
		ImGui::ImageButton("##MaterialAssetBtn", ImTextureID(material->getTextureID()), button_size);
		payload_type = "MATERIAL_ASSET";
	}
	else if (shader) {
		ImGui::ColorButton("##ShaderAssetBtn", shader_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, button_size);
		payload_type = "SHADER_ASSET";
	}


	// imgui_demo.cpp Widgets/Drag and drop
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

		// set payload to carry the index of the asset
		ImGui::SetDragDropPayload(payload_type, assetName.c_str(), assetName.length() + 1); // +1 because of null terminator

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
		}
		if (material) {
			ImGui::Text("Texture ID: %u", material->getTextureID());
		}
		if (shader) {
			ImGui::Text("Shader Program ID: %u", shader->GetProgram());
			ImGui::Text("Shader Vert: %s", shader->GetVertName());
			ImGui::Text("Shader Frag: %s", shader->GetFragName());
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
	ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnail_size);
	ImGui::Text("%s", assetName.c_str());
	ImGui::PopTextWrapPos();


	ImGui::PopID();
}
