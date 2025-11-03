#include "EditorManager.h"

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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window_, context_);
	ImGui_ImplOpenGL3_Init();

	// if scenegraph exists, initalize all the windows
	if (sceneGraph) {
		hierarchyWindow = std::make_unique<HierarchyWindow>(sceneGraph);
		inspectorWindow = std::make_unique<InspectorWindow>(sceneGraph);
		assetManagerWindow = std::make_unique<AssetManagerWindow>(sceneGraph);
		sceneWindow = std::make_unique<DockingWindow>(sceneGraph);
	}

	// register default windows (leaving demo window commented out for now)
	//RegisterWindow("Demo", false);

	imguiInit = true;
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

	// Create the dockspace
	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();


	// render all the windows and dialogs
	RenderMainMenuBar();

	/*if (showDemoWindow) {
		ImGui::ShowDemoWindow(&showDemoWindow);
	}*/

	if (IsWindowOpen("Hierarchy") && hierarchyWindow) {
		hierarchyWindow->ShowHierarchyWindow(GetWindowStatePtr("Hierarchy"));
	}
	if (IsWindowOpen("Inspector") && inspectorWindow) {
		inspectorWindow->ShowInspectorWindow(GetWindowStatePtr("Inspector"));
	}
	if (IsWindowOpen("AssetManager") && assetManagerWindow) {
		assetManagerWindow->ShowAssetManagerWindow(GetWindowStatePtr("AssetManager"));
	}
	if (IsWindowOpen("Scene") && sceneWindow) {
		sceneWindow->ShowDockingWindow(GetWindowStatePtr("Scene"));
	}

	ShowSaveDialog();
	ShowLoadDialog();

	// Render
	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
		ImGui::InputText("##SaveFileName", &sceneGraph->cellFileName);
		ImGui::Separator();

		bool canSave = !sceneGraph->cellFileName.empty();

		if (!canSave) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Save File") && canSave) {
			if (sceneGraph) {
				std::filesystem::create_directory("Game Objects/" + sceneGraph->cellFileName);
				sceneGraph->SaveFile(sceneGraph->cellFileName);
				Debug::Info("Saved file: " + sceneGraph->cellFileName, __FILE__, __LINE__);
			}
			
			sceneGraph->cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canSave) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			SceneGraph::getInstance().cellFileName.clear();
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
		ImGui::InputText("##NameOfLoadFile", &sceneGraph->cellFileName);
		ImGui::Separator();

		bool canLoad = !sceneGraph->cellFileName.empty();

		if (!canLoad) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Load File") && canLoad) {
			if (sceneGraph) {
				sceneGraph->RemoveAllActors();
				XMLObjectFile::addActorsFromFile(sceneGraph, sceneGraph->cellFileName);
				sceneGraph->setUsedCamera(nullptr);
				sceneGraph->checkValidCamera();
				sceneGraph->OnCreate();
				Debug::Info("Loaded file: " + sceneGraph->cellFileName, __FILE__, __LINE__);
			}

			sceneGraph->cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		if (!canLoad) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			sceneGraph->cellFileName.clear();
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

		if (ImGui::BeginMenu("Windows")) {
			//ImGui::MenuItem("Demo", nullptr, GetWindowStatePtr("Demo"));
			ImGui::MenuItem("Hierarchy", nullptr, GetWindowStatePtr("Hierarchy"));
			ImGui::MenuItem("Inspector", nullptr, GetWindowStatePtr("Inspector"));
			ImGui::MenuItem("Asset Manager", nullptr, GetWindowStatePtr("AssetManager"));
			ImGui::MenuItem("Scene", nullptr, GetWindowStatePtr("Scene"));
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
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