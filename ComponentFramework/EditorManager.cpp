#include "pch.h"
#include "EditorManager.h"
#include "ScreenManager.h"
#include "AnimationSystem.h"
#include "InputManager.h"

#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ProjectWindow.h"
#include "SceneWindow.h" 
#include "GameWindow.h" 
#include "MemoryWindow.h"

#include "LightingSystem.h"
#include "PhysicsSystem.h"
#include "SceneLoader.h"

EditorManager& EditorManager::getInstance() {
	static EditorManager instance;
	return instance;
}

void EditorManager::CreateEditorIcons()
{
	// helper lambda that handles shared ptr and oncreate
	auto load = [](const char* rel) -> Ref<MaterialComponent> {
		fs::path abs = SearchPath::getInstance().ResolveEngine(rel);
		if (abs.empty()) return nullptr;

		auto mat = std::make_shared<MaterialComponent>(
			nullptr, abs.string().c_str(), "", "");
		if (mat->OnCreate()) return mat;
		return nullptr;
		};

	editorIcons.playIcon = load("Icons/play.png");
	editorIcons.pauseIcon = load("Icons/pause.png");
	editorIcons.stopIcon = load("Icons/stop.png");
	editorIcons.stepIcon = load("Icons/step.png");

	editorIcons.folderIcon = load("Icons/folder.png");
	editorIcons.meshIcon = load("Icons/mesh.png");
	editorIcons.textureIcon = load("Icons/document.png");
	editorIcons.materialIcon = load("Icons/document.png");
	editorIcons.shaderIcon = load("Icons/shader.png");
	editorIcons.scriptIcon = load("Icons/lua.png");
	editorIcons.animationIcon = load("Icons/anim.png");
	editorIcons.sceneIcon = load("Icons/scene.png");
	editorIcons.prefabIcon = load("Icons/prefab.png");
	editorIcons.glslIcon = load("Icons/glsl.png");
	editorIcons.unknownIcon = load("Icons/unknown.png");
}

EditorManager::~EditorManager() = default;

bool EditorManager::Initialize(SDL_Window* window_, SDL_GLContext context_, SceneGraph* sceneGraph_) {
	if (imguiInit) {
		Debug::Warning("EditorManager already initialized!", __FILE__, __LINE__);
		return true;
	}

	sceneGraph = sceneGraph_;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	//io.IniFilename = nullptr; // if this error (Assertion failed: DockContextFindNodeByID(ctx, id) == 0) ever shows up again uncoment this line 

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window_, context_);
	ImGui_ImplOpenGL3_Init();

	// Apply custum editor style
	EditorStyleColors();

	// if scenegraph exists, initalize all the windows
	if (sceneGraph) {
		hierarchyWindow = std::make_unique<HierarchyWindow>(sceneGraph);
		UpdateActorHierarchy();
		inspectorWindow = std::make_unique<InspectorWindow>(sceneGraph);
		projectWindow = std::make_unique<ProjectWindow>();
		sceneWindow = std::make_unique<SceneWindow>(sceneGraph);
		gameWindow = std::make_unique<GameWindow>();
		memoryWindow = std::make_unique<MemoryManagerWindow>(sceneGraph);


		CreateEditorIcons();
	}

	imguiInit = true;
	pendingFocusWindow = "Scene";
	Debug::Info("EditorManager succesfully initalized", __FILE__, __LINE__);
	return true;
}

void EditorManager::Shutdown() {
	if (!imguiInit) {
		return;
	}

	// set unqiue pointer to nullptr
	hierarchyWindow.reset();
	inspectorWindow.reset();
	projectWindow.reset();
	sceneWindow.reset();
	gameWindow.reset();
	memoryWindow.reset();
	windowStates.clear();

	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	imguiInit = false;
	sceneGraph = nullptr;

	Debug::Info("EditorManager shut down succesfully", __FILE__, __LINE__);
}

void EditorManager::HandleEvents(const SDL_Event& event) const {
	if (!imguiInit) {
		return;
	}

	// pass all sdl events to imgui
	ImGui_ImplSDL2_ProcessEvent(&event);
}

void EditorManager::RenderEditorUI() {
	if (!imguiInit) {
		Debug::Warning("Cannot render editor UI. EditorManager not initialized", __FILE__, __LINE__);
		return;
	}

	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// create a dockspace (based on ImGui demo dockspace)
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar();

	// TODO: fix so it doesn't have to be stuck inbetween the dockspace
	RenderEditorToolbar();

	// Create the dockspace
	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	// render all the windows and dialogs
	RenderMainMenuBar();
	ShowSaveDialog();
	ShowLoadDialog();
	ShowProjectSettingsWindow();

	if (IsWindowOpen("Hierarchy") && hierarchyWindow) {
		hierarchyWindow->ShowHierarchyWindow(GetWindowStatePtr("Hierarchy"));
	}
	if (IsWindowOpen("Inspector") && inspectorWindow) {
		inspectorWindow->ShowInspectorWindow(GetWindowStatePtr("Inspector"));
	}
	if (IsWindowOpen("Project") && projectWindow) {
		projectWindow->ShowProjectWindowWindow(GetWindowStatePtr("Project"));
	}
	if (IsWindowOpen("Game") && gameWindow) {
		gameWindow->ShowGameWindow(GetWindowStatePtr("Game"));
	}
	if (IsWindowOpen("Scene") && sceneWindow) {
		sceneWindow->ShowSceneWindow(GetWindowStatePtr("Scene"));
	}
	if (IsWindowOpen("Memory") && memoryWindow) {
		memoryWindow->ShowMemoryManagerWindow(GetWindowStatePtr("Memory"));
	}

	if (!pendingFocusWindow.empty()) {
		ImGui::SetWindowFocus(pendingFocusWindow.c_str());
		pendingFocusWindow.clear();
	}

	// Render
	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorManager::SaveScene(const std::string& name)
{
	std::string saveName = name.empty() ? sceneGraph->sceneFileName : name;
	if (saveName.empty()) {
		saveLoadDialog.showSaveDialog = true;
		return;
	}

	sceneGraph->sceneFileName = saveName;
	fs::path saveDir = SearchPath::getInstance().GetEngineRoot() / "Game Objects" / saveName;
	fs::create_directories(saveDir);
	sceneGraph->SaveFile(saveName);
	ScreenManager::getInstance().setWindowTitle(saveName);
	Debug::Info("Saved Scene: " + saveName, __FILE__, __LINE__);
}

void EditorManager::LoadScene(const std::string& name)
{
	sceneGraph->sceneFileName = name;

	std::string loadName = name.empty() ? sceneGraph->sceneFileName : name;
	if (loadName.empty()) {
		saveLoadDialog.showLoadDialog = true;
		return;
	}

	AnimationSystem::getInstance().StopMeshLoadingWorker();
	LightingSystem::getInstance().ClearActors();
	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();
	sceneGraph->OnDestroy();

	//set active memory to stale for later comparison
	MemoryStale();

	// load new scene
	AnimationSystem::getInstance().StartMeshLoadingWorker();
	sceneGraph->OnCreate();
	sceneGraph->sceneFileName = loadName;
	XMLObjectFile::addActorsFromFile(sceneGraph, loadName);
	ScreenManager::getInstance().setWindowTitle(loadName);
	int sceneId = ProjectSettingsManager::getInstance().Get().GetSceneIdByName(loadName);
	SceneLoader::SetActiveScene(loadName, sceneId);
	Debug::Info("Loaded file: " + sceneGraph->sceneFileName, __FILE__, __LINE__);
}

void EditorManager::Play()
{
	if (!AnimationSystem::getInstance().isAllComponentsLoaded()) return;

	SetEditorMode(EditorMode::Play);
	InputManager::getInstance().setGameInputActive(true);

	// on play, save data to a temporary save file
	//std::filesystem::create_directory("Game Objects/" + tempSaveFile);
	//sceneGraph->SaveFile(tempSaveFile);

	sceneGraph->Start();
	pendingFocusWindow = "Game";
}

void EditorManager::Stop()
{
	SetEditorMode(EditorMode::Edit);
	InputManager::getInstance().setGameInputActive(false);
	sceneGraph->Stop();

	std::string sceneFile = sceneGraph->sceneFileName;
	
	LightingSystem::getInstance().ClearActors();
	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();
	sceneGraph->OnDestroy();

	// removing temporary save file data
	//std::filesystem::remove("Cell Files/" + tempSaveFile + ".xml");
	//std::filesystem::remove_all("Game Objects/" + tempSaveFile);

	//Make the memory stale so we can see if its potentially a leak
	MemoryStale();

	XMLObjectFile::addActorsFromFile(sceneGraph, sceneFile);
	sceneGraph->OnCreate();

	int sceneId = ProjectSettingsManager::getInstance().Get().GetSceneIdByName(sceneFile);
	SceneLoader::SetActiveScene(sceneFile, sceneId);

	pendingFocusWindow = "Scene";
}

void EditorManager::Pause()
{
}

void EditorManager::RenderEditorToolbar()
{
	ImVec2 buttonSize = ImVec2(24, 24);
	float width = ImGui::GetWindowSize().x;
	float centreButtonPos = (width - buttonSize.x) / 2;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1.0f, 0.0f });

	ImGui::SetCursorPosX(centreButtonPos);

	bool isLoading = !AnimationSystem::getInstance().isAllComponentsLoaded();

	if (isEditMode()) {
		if (isLoading) ImGui::BeginDisabled();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.314f, 0.314f, 0.314f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.357f, 0.447f, 0.569f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.467f, 0.467f, 0.467f, 1.0f));

		if (ImGui::ImageButton("##PlayButton", ImTextureID(editorIcons.playIcon->getDiffuseID()), buttonSize)) {
			Play();
		}

		ImGui::PopStyleColor(3);

		if (isLoading) {
			ImGui::EndDisabled();
		}
	}
	else if (isPlayMode() || isPaused()) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.298f, 0.373f, 0.471f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.357f, 0.447f, 0.569f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.357f, 0.447f, 0.569f, 1.0f));
		if (ImGui::ImageButton("##StopButton", ImTextureID(editorIcons.stopIcon->getDiffuseID()), buttonSize)) {
			Stop();
		}

		ImGui::PopStyleColor(3);
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.314f, 0.314f, 0.314f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.357f, 0.447f, 0.569f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.467f, 0.467f, 0.467f, 1.0f));
	ImGui::ImageButton("##PauseButton", ImTextureID(editorIcons.pauseIcon->getDiffuseID()), buttonSize);
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	if (isEditMode() || isPlayMode()) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.208f, 0.208f, 0.208f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.208f, 0.208f, 0.208f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.208f, 0.208f, 0.208f, 1.0f));

		ImGui::ImageButton("##StepButton", ImTextureID(editorIcons.stepIcon->getDiffuseID()), buttonSize);

		ImGui::PopStyleColor(3);
	}

	ImGui::PopStyleVar();
}

void EditorManager::ShowSaveDialog() {
	if (!saveLoadDialog.showSaveDialog) return;

	ImGui::SetNextWindowSize(ImVec2(305, 100));
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("Save Scene As", &saveLoadDialog.showSaveDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Scene Name:");
		ImGui::SetNextItemWidth(-1);
		ImGui::InputText("##SaveName", &saveLoadDialog.fileName);
		ImGui::Spacing();

		bool canSave = !saveLoadDialog.fileName.empty();
		if (!canSave) ImGui::BeginDisabled();
		if (ImGui::Button("Save", ImVec2(140, 0))) {
			SaveScene(saveLoadDialog.fileName);
			saveLoadDialog.showSaveDialog = false;
		}
		if (!canSave) ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(140, 0))) {
			saveLoadDialog.showSaveDialog = false;
		}
	}

	ImGui::End();
}

void EditorManager::ShowLoadDialog() {
	if (!saveLoadDialog.showLoadDialog) return;

	const auto& scenePaths = AssetManager::getInstance().GetScenePaths();

	ImGui::SetNextWindowSize(ImVec2(305, 250)); // if there ends up being a lot of scenes after, could extend this out a bit
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("Load Scene", &saveLoadDialog.showLoadDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Available Scenes:");

		ImGui::BeginChild("##SceneList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
		for (const auto& path : scenePaths) {
			std::string stem = path.stem().string();
			bool selected = (saveLoadDialog.fileName == stem);
			if (ImGui::Selectable(stem.c_str(), selected)) saveLoadDialog.fileName = stem;
			if (selected && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				LoadScene(stem);
				saveLoadDialog.showLoadDialog = false;
			}
		}
		ImGui::EndChild();

		bool canLoad = !saveLoadDialog.fileName.empty();
		if (!canLoad) ImGui::BeginDisabled();
		if (ImGui::Button("Load", ImVec2(140, 0))) {
			LoadScene(saveLoadDialog.fileName);
			saveLoadDialog.showLoadDialog = false;
		}
		if (!canLoad) ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(140, 0))) {
			saveLoadDialog.showLoadDialog = false;
		}
	}

	ImGui::End();
}

void EditorManager::RenderMainMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Scene")) {
				SaveScene(SceneGraph::getInstance().sceneFileName);

				fs::path dir = SearchPath::getInstance().EnsureSubfolder("Scenes");
				std::string stem = AssetManager::getInstance().GenerateUniqueFileName(dir, "NewScene", ".scene");
				fs::path out = dir / (stem + ".scene");

				XMLDocument doc;
				auto* root = doc.NewElement("Scene");
				root->InsertEndChild(doc.NewElement("Actors"));
				doc.InsertFirstChild(root);
				doc.SaveFile(out.string().c_str());

				AssetManager::getInstance().RefreshSingle(out);
				UpdateProjectWindow();

				LoadScene(stem);
			}
			if (ImGui::MenuItem("Load Scene", "Ctrl+L")) {
				saveLoadDialog.showLoadDialog = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene", "Ctrl+S")) SaveScene(SceneGraph::getInstance().sceneFileName);

			if (ImGui::MenuItem("Save Scene As...")) {
				saveLoadDialog.showSaveDialog = true;
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Project Settings")) {
				showProjectSettings = true;
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit ##MenuItem", "Esc")) {
				// Signal to close the application
				SDL_Event quitEvent{};
				quitEvent.type = SDL_QUIT;
				SDL_PushEvent(&quitEvent);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Hierarchy", nullptr, GetWindowStatePtr("Hierarchy"));
			ImGui::MenuItem("Inspector", nullptr, GetWindowStatePtr("Inspector"));
			ImGui::MenuItem("Project", nullptr, GetWindowStatePtr("Project"));
			ImGui::MenuItem("Scene", nullptr, GetWindowStatePtr("Scene"));
			ImGui::MenuItem("Game", nullptr, GetWindowStatePtr("Game"));
			ImGui::MenuItem("Memory Manager", nullptr, GetWindowStatePtr("Memory"));

			ImGui::EndMenu();
		}

	}
	ImGui::EndMainMenuBar();
}

void EditorManager::RequestActorRename(const std::string& oldName_, const std::string& newName_) {
	pendingRename.oldName = oldName_;
	pendingRename.newName = newName_;
	pendingRename.pending = true;
}

std::pair<std::string, std::string> EditorManager::ConsumePendingRename() {
	std::pair<std::string, std::string> result = std::make_pair(pendingRename.oldName, pendingRename.newName);
	pendingRename.pending = false;
	pendingRename.oldName.clear();
	pendingRename.newName.clear();
	return result;
}

void EditorManager::UpdateActorHierarchy()
{
	if (hierarchyWindow) hierarchyWindow->UpdateHierarchyNextFrame();
}

void EditorManager::ClearSelectedAsset()
{
	selectedAsset.isSet = false; if (projectWindow) projectWindow->ClearSelectedFile();
}

void EditorManager::UpdateProjectWindow()
{
	if (projectWindow) projectWindow->NeedsRefresh();
}

void EditorManager::ShowProjectSettingsWindow()
{
	if (!showProjectSettings) return;

	static ProjectSettings editCopy;
	static char titleBuf[256] = {};
	
	ImGui::SetNextWindowSize(ImVec2(500, 400));
	if (ImGui::Begin("Project Settings", &showProjectSettings, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		if (ImGui::IsWindowAppearing()) {
			editCopy = ProjectSettingsManager::getInstance().Get();
			strncpy_s(titleBuf, editCopy.windowTitle.c_str(), sizeof(titleBuf) - 1);
		}

		static int selectedCategory = 0;
		const char* categories[] = { "Engine Settings", "Tags", "Scene List" };

		ImGui::BeginChild("##PSLeft", ImVec2(150, -23), true);
		for (int i = 0; i < 3; ++i) {
			if (ImGui::Selectable(categories[i], selectedCategory == i)) {
				selectedCategory = i;
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("##PSRight", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

		if (selectedCategory == 0) {
			ImGui::Text("Engine/Performance Settings");
			ImGui::Separator();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Window Title");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			if (ImGui::InputText("##WindowTitle", titleBuf, sizeof(titleBuf))) editCopy.windowTitle = titleBuf;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Render Width");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::InputInt("##RenderWidth", &editCopy.renderWidth);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Render Height");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::InputInt("##RenderHeight", &editCopy.renderHeight);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Display Width");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::InputInt("##DisplayWidth", &editCopy.displayWidth);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Display Height");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::InputInt("##DisplayHeight", &editCopy.displayHeight);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Target FPS");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::InputInt("##TargetFPS", &editCopy.targetFPS);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("VSync");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##VSync", &editCopy.vsync);
		}
		else if (selectedCategory == 1) {
			ImGui::Text("Tags");
			ImGui::Separator();
			int removeIdx = -1;
			for (int i = 0; i < static_cast<int>(editCopy.tags.size()); ++i) {
				ImGui::PushID(i);
				bool isDefault = (editCopy.tags[i] == "Untagged" || editCopy.tags[i] == "MainCamera");
				if (isDefault) ImGui::BeginDisabled();
				if (ImGui::SmallButton("X")) removeIdx = i;
				if (isDefault) ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::TextUnformatted(editCopy.tags[i].c_str());
				ImGui::PopID();
			}
			if (removeIdx >= 0) editCopy.tags.erase(editCopy.tags.begin() + removeIdx);

			ImGui::Separator();
			static std::string newTagBuf;
			ImGui::SetNextItemWidth(140);
			ImGui::InputText("##NewTag", &newTagBuf);
			ImGui::SameLine();
			bool canAdd = !newTagBuf.empty() && std::find(editCopy.tags.begin(), editCopy.tags.end(), newTagBuf) == editCopy.tags.end();
			if (!canAdd) ImGui::BeginDisabled();
			if (ImGui::Button("Add")) { editCopy.tags.push_back(newTagBuf); newTagBuf.clear(); }
			if (!canAdd) ImGui::EndDisabled();
		}
		else if (selectedCategory == 2) {
			ImGui::Text("Scene List");
			ImGui::Separator();

			const char* startupPreview = editCopy.startupScene.empty() ? "(first in list)" : editCopy.startupScene.c_str();
			ImGui::Text("Startup Scene");
			ImGui::SameLine(140);
			ImGui::SetNextItemWidth(-1);
			if (ImGui::BeginCombo("##StartupScene", startupPreview)) {
				if (ImGui::Selectable("(first in list)", editCopy.startupScene.empty())) editCopy.startupScene.clear();
				for (auto& e : editCopy.sceneList) {
					bool sel = (editCopy.startupScene == e.name);
					if (ImGui::Selectable(e.name.c_str(), sel)) editCopy.startupScene = e.name;
				}
				ImGui::EndCombo();
			}
			ImGui::Separator();

			for (int i = 0; i < static_cast<int>(editCopy.sceneList.size()); ++i) {
				SceneListData& entry = editCopy.sceneList[i];
				ImGui::PushID(i);
				ImGui::Text("[%d]", entry.id); ImGui::SameLine(40);
				ImGui::TextUnformatted(entry.name.c_str()); ImGui::SameLine();
				if (i > 0 && ImGui::SmallButton("^")) { std::swap(editCopy.sceneList[i], editCopy.sceneList[i - 1]); editCopy.RebuildIds(); }
				ImGui::SameLine();
				if (i < static_cast<int>(editCopy.sceneList.size()) - 1 && ImGui::SmallButton("v")) { std::swap(editCopy.sceneList[i], editCopy.sceneList[i + 1]); editCopy.RebuildIds(); }
				ImGui::SameLine();
				if (ImGui::SmallButton("X")) {
					if (editCopy.startupScene == entry.name) editCopy.startupScene.clear();
					editCopy.sceneList.erase(editCopy.sceneList.begin() + i);
					editCopy.RebuildIds();
					ImGui::PopID();
					break;
				}
				ImGui::PopID();
			}

			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 36));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("PROJECT_ASSET")) {
					auto* d = static_cast<const ProjectDragPayload*>(p->Data);
					fs::path abs(d->absolutePath);
					if (abs.extension() == ".scene") {
						SceneListData entry;
						entry.name = abs.stem().string();
						entry.path = SearchPath::getInstance().MakeRelative(abs).string();
						entry.id = static_cast<int>(editCopy.sceneList.size());
						bool exists = false;
						for (auto& e : editCopy.sceneList) if (e.name == entry.name) { exists = true; break; }
						if (!exists) editCopy.sceneList.push_back(entry);
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::TextDisabled("^ Drop Scene Files Here ^");
		}

		ImGui::EndChild();

		if (ImGui::Button("Save")) {
			editCopy.windowTitle = titleBuf;
			ProjectSettingsManager::getInstance().Get() = editCopy;
			ProjectSettingsManager::getInstance().SaveDefault();
			showProjectSettings = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			showProjectSettings = false;
		}
	}

	ImGui::End();
}

void EditorManager::EditorStyleColors()
{
	// using imgui style colors dark as baseline
	//ImGui::StyleColorsDark();

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	// making everything a bit rounded so its not as rigid
	style->WindowRounding = 4.0f;
	style->ChildRounding = 4.0f;
	style->FrameRounding = 4.0f;
	style->PopupRounding = 4.0f;
	style->ScrollbarRounding = 4.0f;
	style->TabRounding = 4.0f;

	// going for a more unity/greyish color scheme

	//colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	//colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	//colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
	//colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	//colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	//colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	//colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.165f, 0.165f, 0.165f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.467f, 0.467f, 0.467f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.357f, 0.447f, 0.569f, 1.0f);

	colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

	//colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	//colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

	//colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	//colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	//colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	//colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	//colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	//colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

	colors[ImGuiCol_Button] = ImVec4(0.165f, 0.165f, 0.165f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.467f, 0.467f, 0.467f, 1.0f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.357f, 0.447f, 0.569f, 1.0f);

	colors[ImGuiCol_Header] = ImVec4(0.32f, 0.32f, 0.32f, 0.5f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.467f, 0.467f, 0.467f, 1.0f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.357f, 0.447f, 0.569f, 1.0f);

	//colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	//colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	//colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	//colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	//colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	//colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	//colors[ImGuiCol_InputTextCursor] = colors[ImGuiCol_Text];

	colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	//colors[ImGuiCol_TabSelected] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	//colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
	//colors[ImGuiCol_TabDimmed] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	//colors[ImGuiCol_TabDimmedSelected] = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
	//colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
	//colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);

	//colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	//colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	//colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	//colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	//colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	//colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	//colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
	//colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
	//colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	//colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	//colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];
	//colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	//colors[ImGuiCol_TreeLines] = colors[ImGuiCol_Border];
	//colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	//colors[ImGuiCol_DragDropTargetBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	//colors[ImGuiCol_UnsavedMarker] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	//colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	//colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	//colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

}