#include "pch.h"
#include "EditorManager.h"
#include "ScreenManager.h"
#include "AnimationSystem.h"
#include "InputManager.h"

void EditorManager::CreateEditorIcons()
{
	editorIcons.playIcon->OnCreate();
	editorIcons.pauseIcon->OnCreate();
	editorIcons.stopIcon->OnCreate();
	editorIcons.stepIcon->OnCreate();
	editorIcons.meshIcon->OnCreate();
	editorIcons.shaderIcon->OnCreate();
}

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
		assetManagerWindow = std::make_unique<AssetManagerWindow>(sceneGraph);
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
	assetManagerWindow.reset();
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

void EditorManager::HandleEvents(const SDL_Event& event) {
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

	if (IsWindowOpen("Hierarchy") && hierarchyWindow) {
		hierarchyWindow->ShowHierarchyWindow(GetWindowStatePtr("Hierarchy"));
	}
	if (IsWindowOpen("Inspector") && inspectorWindow) {
		inspectorWindow->ShowInspectorWindow(GetWindowStatePtr("Inspector"));
	}
	if (IsWindowOpen("AssetManager") && assetManagerWindow) {
		assetManagerWindow->ShowAssetManagerWindow(GetWindowStatePtr("AssetManager"));
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
	fs::path saveDir = SearchPath::getInstance().GetRoot() / "Game Objects" / saveName;
	fs::create_directories(saveDir);
	sceneGraph->SaveFile(saveName);
	AssetManager::getInstance().SaveAssetDatabaseXML();
	ScreenManager::getInstance().setWindowTitle(saveName);
	Debug::Info("Saved Scene: " + saveName, __FILE__, __LINE__);
}

void EditorManager::LoadScene(const std::string& name)
{
	std::string loadName = name.empty() ? sceneGraph->sceneFileName : name;
	if (loadName.empty()) {
		saveLoadDialog.showLoadDialog = true;
		return;
	}

	sceneGraph->RemoveAllActors();
	std::vector<std::string> sceneTags = XMLObjectFile::readSceneTags(loadName);
	for (const auto& tag : sceneTags) sceneGraph->addTag(tag);
	
	//set active memory to stale for later comparison
	MemoryStale();

	XMLObjectFile::addActorsFromFile(sceneGraph, loadName);
	sceneGraph->sceneFileName = loadName;
	ScreenManager::getInstance().setWindowTitle(loadName);
	sceneGraph->OnCreate();
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
	sceneGraph->RemoveAllActors();

	std::vector<std::string> sceneTags = XMLObjectFile::readSceneTags(sceneFile);
	for (const auto& tag : sceneTags) sceneGraph->addTag(tag);

	// removing temporary save file data
	//std::filesystem::remove("Cell Files/" + tempSaveFile + ".xml");
	//std::filesystem::remove_all("Game Objects/" + tempSaveFile);
	
	//Make the memory stale so we can see if its potentially a leak
	MemoryStale();

	XMLObjectFile::addActorsFromFile(sceneGraph, sceneFile);
	sceneGraph->OnCreate();

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

// mostly the same from the original but I updated the some visuals so it looks nicer
void EditorManager::ShowSaveDialog() {
	if (saveLoadDialog.showSaveDialog) {
		ImGui::OpenPopup("Save File");
		saveLoadDialog.showSaveDialog = false;
	}

	// sets the placement and size of the dialog box
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Save File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter the name of the file you want to save:");
		ImGui::InputText("##SaveFileName", &sceneGraph->sceneFileName);
		ImGui::Separator();

		bool canSave = !sceneGraph->sceneFileName.empty();

		if (!canSave) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Save File") && canSave) {
			SaveScene(sceneGraph->sceneFileName);
			//sceneGraph->sceneFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canSave) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			//SceneGraph::getInstance().sceneFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canSave) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Please enter a file name!");
		}

		ImGui::EndPopup();
	}
}

// mostly the same from the original but I updated the some visuals so it looks nicer
void EditorManager::ShowLoadDialog() {
	if (saveLoadDialog.showLoadDialog) {
		ImGui::OpenPopup("Load File");
		saveLoadDialog.showLoadDialog = false;
	}

	// sets the placement and size of the dialog box
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Load File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter the name of the file you want to load:");
		ImGui::InputText("##NameOfLoadFile", &sceneGraph->sceneFileName);
		ImGui::Separator();

		bool canLoad = !sceneGraph->sceneFileName.empty();

		if (!canLoad) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Load File") && canLoad) {
			LoadScene(sceneGraph->sceneFileName);
			//sceneGraph->sceneFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canLoad) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			//sceneGraph->sceneFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canLoad) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Please enter a file name!");
		}

		ImGui::EndPopup();
	}
}

void EditorManager::RenderMainMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save File ##MenuItem", "Ctrl+S")) {
				saveLoadDialog.showSaveDialog = true;
			}
			if (ImGui::MenuItem("Load File ##MenuItem", "Ctrl+L")) {
				saveLoadDialog.showLoadDialog = true;
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
			ImGui::MenuItem("Asset Manager", nullptr, GetWindowStatePtr("AssetManager"));
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