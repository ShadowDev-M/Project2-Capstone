#include "pch.h"
#include "ProjectWindow.h"
#include "EditorManager.h"

ProjectWindow::ProjectWindow() { 
	EditorManager::getInstance().RegisterWindow("Project", true); 
	selectedDir = SearchPath::getInstance().GetRoot();
}

void ProjectWindow::ShowProjectWindowWindow(bool* pOpen)
{
	if (needsRefresh) { RebuildEntries(); needsRefresh = false; }

	if (ImGui::Begin("Project", pOpen)) {		
		bool atRoot = (selectedDir == SearchPath::getInstance().GetRoot());
		ImGui::BeginDisabled(atRoot);
		if (ImGui::Button("<##back")) {
			selectedDir = selectedDir.parent_path();
			NeedsRefresh();
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip(atRoot ? "Already at Assets root" : "Go up to: %s", selectedDir.parent_path().filename().string().c_str());
		}

		ImGui::SameLine();
		filter.Draw("##filter", ImGui::GetContentRegionAvail().x - 70);
		ImGui::SameLine();
		if (ImGui::Button("Refresh")) {
			AssetManager::getInstance().Refresh();
			NeedsRefresh();
		}
		ImGui::Separator();

		// building out the rest of the window
		ImGui::Columns(2, "ProjectCols", true);
		ImGui::SetColumnWidth(0, PANEL_WIDTH);
		DrawLeftPanel();
		ImGui::NextColumn();
		DrawRightPanel();
		ImGui::Columns(1);

		ImGui::Separator();
	}

	ImGui::End();
}

void ProjectWindow::RebuildEntries()
{
	entries.clear();
	folderTree.clear();
	BuildFolderTree(SearchPath::getInstance().GetRoot());

	if (!fs::is_directory(selectedDir)) selectedDir = SearchPath::getInstance().GetRoot();

	for (auto& e : fs::directory_iterator(selectedDir)) {
		// filling out the file entry
		FileEntry fe;
		fe.absolutePath = e.path();
		fe.displayName = e.path().filename().string();
		fe.isDir = e.is_directory();
		fe.iconType = GetIconFileEntry(e);
		fe.assetName = e.path().stem().string();

		// getting file type
		std::string ext = e.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // convert to lowercase for extension check
		if (ext == ".obj" || ext == ".fbx") fe.componentType = "MeshComponent";
		else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".mat") fe.componentType = "MaterialComponent";
		else if (ext == ".shader") fe.componentType = "ShaderComponent";
		else if (ext == ".lua") fe.componentType = "ScriptAbstract";
		else if (ext == ".gltf") fe.componentType = "Animation";
		
		entries.push_back(fe);
	}

	// sorting by folders first then file names
	std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
		if (a.isDir != b.isDir) return a.isDir > b.isDir;
		return a.displayName < b.displayName;
		});
}

void ProjectWindow::BuildFolderTree(const fs::path& root)
{
	for (auto& e : fs::directory_iterator(root)) {
		if (e.is_directory()) {
			folderTree.push_back(e.path());
			BuildFolderTree(e.path());
		}
	}
}

void ProjectWindow::DrawLeftPanel()
{
	// this is for the folder hierarchy on the left side of the window
	ImGui::BeginChild("FolderTree", ImVec2(0, 0), false);
	DrawFolderNode(SearchPath::getInstance().GetRoot());

	if (ImGui::BeginPopupContextWindow("FolderTreeCtx", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) { 
		DrawContextMenuEmpty(); 
		ImGui::EndPopup(); 
	}
	
	// dragging asset to folder
	if (ImGui::BeginDragDropTarget()) {
		if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
			auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
			MoveToFolder(fs::path(data->absolutePath), selectedDir);
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::EndChild();
}

void ProjectWindow::DrawRightPanel()
{
	// main asset/file grid section
	ImGui::BeginChild("FileGrid", ImVec2(0, 0), false);
	DrawFileGrid();

	if (ImGui::BeginPopupContextWindow("FileGridCtx", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		DrawContextMenuEmpty();
		ImGui::EndPopup();
	}

	ImGui::EndChild();
}

void ProjectWindow::DrawFolderNode(const fs::path& dir)
{
	bool isRoot = (dir == SearchPath::getInstance().GetRoot());
	std::string label = isRoot ? "Assets" : dir.filename().string();

	bool hasChildren = false;
	for (auto& child : folderTree) {
		if (child.parent_path() == dir) { hasChildren = true; break; }
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth; // | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (hasChildren) flags |= ImGuiTreeNodeFlags_OpenOnArrow;
	else flags |= ImGuiTreeNodeFlags_Leaf;
	if (dir == selectedDir) flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(label.c_str(), flags);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		selectedDir = dir;
		NeedsRefresh();
	}

	if (!isRoot) {
		std::string popupId = "FolderCtx_" + dir.string();
		if (ImGui::BeginPopupContextItem(popupId.c_str())) {
			DrawContextMenuFolder(dir);
			ImGui::EndPopup();
		}
	}

	// dragging asset to folder
	if (ImGui::BeginDragDropTarget()) {
		if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
			auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
			MoveToFolder(fs::path(data->absolutePath), dir);
		}
		ImGui::EndDragDropTarget();
	}

	// only rendering the direct child node, no grandchildren
	if (open) {
		for (auto& child : folderTree) {
			if (child.parent_path() == dir) DrawFolderNode(child);
		}

		ImGui::TreePop();
	}

	if (isRenaming && renamePath == dir) {
		ImGui::SetKeyboardFocusHere();
		ImGui::SetNextItemWidth(PANEL_WIDTH - 20.0f);
		if (ImGui::InputText("##renameFolder", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
			fs::path newDir = dir.parent_path() / renameBuffer;
			if (!fs::exists(newDir)) {
				std::error_code ec;
				fs::rename(dir, newDir, ec);
				if (!ec) {
					if (selectedDir == dir) selectedDir = newDir;
					AssetManager::getInstance().Refresh();
					NeedsRefresh();
				}
			}
			isRenaming = false;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) isRenaming = false;
	}
}

void ProjectWindow::DrawFileGrid()
{
	float panelW = ImGui::GetContentRegionAvail().x;
	float cellW = TILE_SIZE + 12.0f;
	int cols = std::max(1, (int)(panelW / cellW));
	int col = 0;

	for (auto& entry : entries) {
		if (!filter.PassFilter(entry.displayName.c_str())) continue;

		// setting up columns
		if (col > 0 && col % cols != 0) ImGui::SameLine();
		ImGui::BeginGroup();

		bool selected = (entry.absolutePath == selectedFile);
		ImVec4 bg = selected ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImVec4(0, 0, 0, 0);

		// icon for the entry
		GLuint tex = GetIconEntry(entry);
		ImGui::PushID(entry.absolutePath.string().c_str());

		if (ImGui::ImageButton("##tile", (ImTextureID)(intptr_t)tex, ImVec2(TILE_SIZE, TILE_SIZE), ImVec2(0, 0), ImVec2(1, 1), bg)) { 
			// passing selected asset and its info over to the editormanager
			selectedFile = entry.absolutePath;
			EditorManager::SelectedAsset selectedAsset;
			selectedAsset.assetName = entry.assetName;
			selectedAsset.componentType = entry.componentType;
			selectedAsset.absolutePath = entry.absolutePath;
			selectedAsset.isSet = true;
			EditorManager::getInstance().SetSelectedAsset(selectedAsset);
		}

		// extra visability for selected assets
		if (selected) {
			ImVec2 rMin = ImGui::GetItemRectMin();
			ImVec2 rMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRect(rMin, rMax, IM_COL32(100, 170, 255, 230), 4.0f, ImDrawFlags_RoundCornersAll, 2.5f);
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (entry.isDir) {
				selectedDir = entry.absolutePath;
				NeedsRefresh();
			}
			else if (entry.iconType == AssetIcon::Scene) {
				EditorManager::getInstance().LoadScene(entry.absolutePath.stem().string());
			}
		}

		if (ImGui::BeginPopupContextItem("TileCtx")) {
			DrawContextMenuFile(entry);
			ImGui::EndPopup();
		}

		// dragging assets
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			// copying over entry data to payload
			EditorManager::ProjectDragPayload payload{};
			strncpy_s(payload.assetName, entry.assetName.c_str(), 127); // 1 off buffer, null terminator
			strncpy_s(payload.componentType, entry.componentType.c_str(), 63);
			strncpy_s(payload.absolutePath, entry.absolutePath.string().c_str(), 511);
			ImGui::SetDragDropPayload("PROJECT_ASSET", &payload, sizeof(payload));
			if (tex) ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::Text("%s", entry.displayName.c_str());
			ImGui::EndDragDropSource();
		}

		// moving assets to folder inside grid
		if (entry.isDir && ImGui::BeginDragDropTarget()) {
			if (auto* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
				auto* data = static_cast<const EditorManager::ProjectDragPayload*>(payload->Data);
				fs::path src(data->absolutePath);
				if (src != entry.absolutePath &&
					src.parent_path() != entry.absolutePath) {
					MoveToFolder(src, entry.absolutePath);
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();

		// inline file rename (not using string inputtext here, easier with regualr const char* buffer)
		if (isRenaming && renamePath == entry.absolutePath) {
			ImGui::SetNextItemWidth(TILE_SIZE);
			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue)) {
				Rename(entry.absolutePath, renameBuffer);
				isRenaming = false;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) isRenaming = false;
		}
		else {
			std::string label = entry.displayName;
			if (label.size() > 10) label = label.substr(0, 8) + "..."; // better text size

			ImGui::SetNextItemWidth(TILE_SIZE);
			ImGui::Text("%s", label.c_str());

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
				ImGui::SetTooltip("%s", entry.displayName.c_str());
			}
		}

		ImGui::EndGroup();
		col++;
	}

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		EditorManager::getInstance().ClearSelectedAsset();
	}
}

void ProjectWindow::DrawContextMenuEmpty()
{
	if (ImGui::BeginMenu("Create")) {
		if (ImGui::MenuItem("Folder")) CreateFolder("NewFolder");
		ImGui::Separator();
		if (ImGui::MenuItem("Material")) CreateMatManifest("NewMaterial");
		if (ImGui::MenuItem("Shader")) CreateShaderManifest("NewShader");
		ImGui::Separator();
		if (ImGui::MenuItem("Scene")) CreateScene("NewScene");
		if (ImGui::MenuItem("Prefab")) CreatePrefab("NewPrefab");
		ImGui::Separator();
		if (ImGui::MenuItem("Script")) CreateScript("NewScript");
		ImGui::EndMenu();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Refresh")) {
		AssetManager::getInstance().Refresh();
		NeedsRefresh();
	}
}

void ProjectWindow::DrawContextMenuFolder(const fs::path& dir)
{
	if (ImGui::MenuItem("Rename")) {
		isRenaming = true;
		renamePath = dir;
		strncpy_s(renameBuffer, dir.filename().string().c_str(), 255);
	}

	if (ImGui::MenuItem("Duplicate")) {
		std::string newStem = AssetManager::getInstance().GenerateUniqueFileName(dir.parent_path(), dir.filename().string(), "");
		fs::path dst = dir.parent_path() / newStem;
		std::error_code ec;
		fs::copy(dir, dst, fs::copy_options::recursive, ec);
		if (!ec) {
			AssetManager::getInstance().Refresh();
			NeedsRefresh();
		}
	}

	if (ImGui::MenuItem("Delete")) {
		std::error_code ec;
		fs::remove_all(dir, ec);
		if (!ec) {
			if (selectedDir == dir || selectedDir.string().find(dir.string()) == 0) {
				selectedDir = dir.parent_path();
			}
			AssetManager::getInstance().Refresh();
			NeedsRefresh();
		}
	}
}

void ProjectWindow::DrawContextMenuFile(const FileEntry& entry)
{
	if (entry.iconType == AssetIcon::Scene) {
		if (ImGui::MenuItem("Load Scene")) EditorManager::getInstance().LoadScene(entry.absolutePath.stem().string());
	}

	if (entry.iconType == AssetIcon::Texture) {
		ImGui::Separator();
		if (ImGui::MenuItem("Create Material From Texture")) CreateMatFromTexture(entry);
	}

	if (ImGui::MenuItem("Show in Explorer")) {
		std::string cmd = "explorer /select,\"" + entry.absolutePath.string() + "\"";
		system(cmd.c_str());
	}
	ImGui::Separator();

	if (ImGui::MenuItem("Open")) {
		std::string command = "start \"\" \"" + entry.absolutePath.string() + "\"";
		system(command.c_str());
	}

	if (ImGui::MenuItem("Rename")) {
		isRenaming = true;
		renamePath = entry.absolutePath;
		strncpy_s(renameBuffer, entry.assetName.c_str(), 255);
	}
	if (ImGui::MenuItem("Duplicate")) Duplicate(entry.absolutePath);
	if (ImGui::MenuItem("Delete")) Delete(entry.absolutePath);

	ImGui::Separator();
	if (ImGui::MenuItem("Refresh")) {
		AssetManager::getInstance().Refresh();
		NeedsRefresh();
	}
}

void ProjectWindow::CreateFolder(const std::string& name)
{
	fs::path p = selectedDir / name;
	int n = 1;
	while (fs::exists(p)) p = selectedDir / (name + std::to_string(n++));
	fs::create_directory(p);
	folderTree.push_back(p);
	NeedsRefresh();
}

void ProjectWindow::CreateMatManifest(const std::string& name)
{
	std::string stem = AssetManager::getInstance().GenerateUniqueFileName(selectedDir, name, ".mat");
	fs::path out = selectedDir / (stem + ".mat");

	XMLDocument doc;
	doc.InsertFirstChild(doc.NewElement("Material"));
	doc.SaveFile(out.string().c_str());
	NeedsRefresh();
}

void ProjectWindow::CreateShaderManifest(const std::string& name)
{
	std::string stem = AssetManager::getInstance().GenerateUniqueFileName(selectedDir, name, ".shader");
	fs::path out = selectedDir / (stem + ".shader");

	XMLDocument doc;
	doc.InsertFirstChild(doc.NewElement("Shader"));
	doc.SaveFile(out.string().c_str());
	NeedsRefresh();
}

void ProjectWindow::CreateScene(const std::string& name)
{
	fs::path dir = SearchPath::getInstance().EnsureSubfolder("Scenes");
	std::string stem = AssetManager::getInstance().GenerateUniqueFileName(dir, name, ".scene");
	fs::path out = dir / (stem + ".scene");

	XMLDocument doc;
	auto* root = doc.NewElement("Scene");
	auto* tags = doc.NewElement("Tags");
	auto* actors = doc.NewElement("Actors");
	root->InsertEndChild(tags);
	root->InsertEndChild(actors);
	doc.InsertFirstChild(root);
	doc.SaveFile(out.string().c_str());

	AssetManager::getInstance().RefreshSingle(out);
	NeedsRefresh();
}

void ProjectWindow::CreatePrefab(const std::string& name)
{
	fs::path dir = SearchPath::getInstance().EnsureSubfolder("Prefabs");
	std::string stem = AssetManager::getInstance().GenerateUniqueFileName(dir, name, ".prefab");
	fs::path out = dir / (stem + ".prefab");

	XMLDocument doc;
	auto* root = doc.NewElement("Prefab");
	root->SetAttribute("name", stem.c_str());
	doc.InsertFirstChild(root);
	doc.SaveFile(out.string().c_str());

	AssetManager::getInstance().RefreshSingle(out);
	NeedsRefresh();
}

void ProjectWindow::CreateScript(const std::string& name)
{
	fs::path dir = SearchPath::getInstance().EnsureSubfolder("Scripts");
	std::string stem = AssetManager::getInstance().GenerateUniqueFileName(dir, name, ".lua");
	fs::path out = dir / (stem + ".lua");

	std::ofstream f(out);
	f << "function Start()\nend\n\nfunction Update(deltaTime)\nend\n";
	f.close();

	AssetManager::getInstance().RefreshSingle(out);
	NeedsRefresh();
}

void ProjectWindow::CreateMatFromTexture(const FileEntry& entry)
{
	fs::path out = entry.absolutePath.parent_path() / (entry.assetName + ".mat");
	std::string rel = SearchPath::getInstance().MakeRelative(entry.absolutePath).string();
	std::replace(rel.begin(), rel.end(), '\\', '/');
	XMLObjectFile::WriteMatManifest(out, rel);
	AssetManager::getInstance().RefreshSingle(out);
	NeedsRefresh();
}

void ProjectWindow::Rename(const fs::path& path, const std::string& newName)
{
	std::string ext = path.extension().string();
	std::string type;
	if (ext == ".obj" || ext == ".fbx") type = "MeshComponent";
	else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".mat") type = "MaterialComponent";
	else if (ext == ".shader") type = "ShaderComponent";
	else if (ext == ".lua") type = "ScriptAbstract";
	else if (ext == ".gltf") type = "Animation";

	if (!type.empty()) {
		AssetManager::getInstance().RenameAsset(path.stem().string(), newName, type);
	}
	else {
		std::error_code ec;
		fs::rename(path, path.parent_path() / (newName + ext), ec);
	}

	NeedsRefresh();
}

void ProjectWindow::Duplicate(const fs::path& path)
{
	std::string ext = path.extension().string();
	std::string name = path.stem().string();
	std::string type;
	if (ext == ".obj" || ext == ".fbx") type = "MeshComponent";
	else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".mat") type = "MaterialComponent";
	else if (ext == ".shader") type = "ShaderComponent";
	else if (ext == ".lua") type = "ScriptAbstract";
	else if (ext == ".gltf") type = "Animation";
	else if (ext == ".scene") type = "Scene";
	
	if (!type.empty()) {
		if (ext == ".scene") {
			std::string newStem = AssetManager::getInstance().GenerateUniqueFileName(path.parent_path(), name, ext);
			fs::path dst = path.parent_path() / (newStem + ext);

			std::error_code ec;
			fs::copy_file(path, dst, ec);
			if (!ec) {
				fs::path srcGO = path.parent_path().parent_path() / "Game Objects" / name;
				fs::path dstGO = path.parent_path().parent_path() / "Game Objects" / newStem;
				if (fs::exists(srcGO)) {
					fs::copy(srcGO, dstGO, fs::copy_options::recursive, ec);
					if (ec) {
						fs::remove(dst);
						NeedsRefresh();
						return;
					}
				}
				AssetManager::getInstance().RefreshSingle(dst);
			}
		}
		else {
			AssetManager::getInstance().DuplicateAsset(name, type);
		}
	}
	else {
		std::string newStem = AssetManager::getInstance().GenerateUniqueFileName(path.parent_path(), name, ext);
		fs::path dst = path.parent_path() / (newStem + ext);
		std::error_code ec;
		fs::copy_file(path, dst, ec);
		if (!ec) AssetManager::getInstance().RefreshSingle(dst);
	}

	NeedsRefresh();
}

void ProjectWindow::Delete(const fs::path& path)
{
	if (fs::is_directory(path)) {
		std::error_code ec;
		fs::remove_all(path, ec);
		AssetManager::getInstance().Refresh();
	}
	else {
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		std::string stem = path.stem().string();
		std::string type;
		if (ext == ".obj" || ext == ".fbx") type = "MeshComponent";
		else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") type = "MaterialComponent";
		else if (ext == ".mat") type = "MaterialComponent";
		else if (ext == ".shader") type = "ShaderComponent";
		else if (ext == ".lua") type = "ScriptAbstract";
		else if (ext == ".gltf") type = "Animation";

		bool deleted = false;
		if (!type.empty()) deleted = AssetManager::getInstance().DeleteAsset(stem, type);

		// fallback for unregistered files
		if (!deleted) {
			std::error_code ec;
			fs::remove(path, ec);
			deleted = !ec;
		}
		(void)deleted;
	}

	EditorManager::getInstance().ClearSelectedAsset();
	NeedsRefresh();
}

void ProjectWindow::MoveToFolder(const fs::path& filePath, const fs::path& destFolder)
{
	if (!fs::is_regular_file(filePath)) return;
	if (filePath.parent_path() == destFolder) return;

	std::string ext = filePath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	std::string stem = filePath.stem().string();
	fs::path dst = destFolder / filePath.filename();

	if (fs::exists(dst)) {
		std::string newStem = AssetManager::getInstance().GenerateUniqueFileName(destFolder, stem, ext);
		dst = destFolder / (newStem + ext);
	}

	// get type
	std::string type;
	if (ext == ".obj" || ext == ".fbx") type = "MeshComponent";
	else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") type = "MaterialComponent";
	else if (ext == ".mat") type = "MaterialComponent";
	else if (ext == ".shader") type = "ShaderComponent";
	else if (ext == ".lua") type = "ScriptAbstract";
	else if (ext == ".gltf") type = "Animation";

	bool moved = false;
	if (!type.empty()) moved = AssetManager::getInstance().MoveAsset(stem, type, destFolder);

	// fallback for files not in assetmanager
	if (!moved) {
		std::error_code ec;
		fs::rename(filePath, dst, ec);
		if (!ec) {
			AssetManager::getInstance().RefreshSingle(dst);
			moved = true;
		}
	}

	if (moved) NeedsRefresh();
}

GLuint ProjectWindow::GetIconEntry(const FileEntry& e) const
{
	auto em = EditorManager::getInstance().getEditorIcons();

	// helper lambda gets the diffuse id from texture
	auto id = [](const Ref<MaterialComponent>& mat) -> GLuint {
		return mat ? mat->getDiffuseID() : 0;
		};

	// preview thumbnail
	if (e.iconType == AssetIcon::Texture || e.iconType == AssetIcon::Material) {
		if (AssetManager::getInstance().HasAsset<MaterialComponent>(e.assetName)) {
			auto mat = AssetManager::getInstance().GetAsset<MaterialComponent>(e.assetName);
			if (mat && mat->getDiffuseID() != 0) return mat->getDiffuseID();
		}
		return e.iconType == AssetIcon::Texture ? id(em.textureIcon) : id(em.materialIcon);
	}

	switch (e.iconType) {
	case AssetIcon::Folder:    return id(em.folderIcon);
	case AssetIcon::Mesh:      return id(em.meshIcon);
	case AssetIcon::Shader:    return id(em.shaderIcon);
	case AssetIcon::GlslFile:  return id(em.glslIcon);
	case AssetIcon::Script:    return id(em.scriptIcon);
	case AssetIcon::Animation: return id(em.animationIcon);
	case AssetIcon::Scene:     return id(em.sceneIcon);
	case AssetIcon::Prefab:    return id(em.prefabIcon);
	default:                   return id(em.unknownIcon);
	}
}

AssetIcon ProjectWindow::GetIconFileEntry(const fs::directory_entry& e) const
{
	if (e.is_directory()) return AssetIcon::Folder;
	std::string ext = e.path().extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".obj" || ext == ".fbx") return AssetIcon::Mesh;
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") return AssetIcon::Texture;
	if (ext == ".mat") return AssetIcon::Material;
	if (ext == ".shader") return AssetIcon::Shader;
	if (ext == ".glsl") return AssetIcon::GlslFile;
	if (ext == ".lua") return AssetIcon::Script;
	if (ext == ".gltf") return AssetIcon::Animation;
	if (ext == ".scene") return AssetIcon::Scene;
	if (ext == ".prefab") return AssetIcon::Prefab;
	return AssetIcon::Unknown;
}