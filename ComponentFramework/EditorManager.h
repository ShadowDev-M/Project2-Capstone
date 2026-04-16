#pragma once
#include "XMLManager.h"

#include "SceneGraph.h"
#include "EditorCamera.h"

// pass all the windows + scenegraph
class SceneGraph;
class HierarchyWindow;
class InspectorWindow;
class ProjectWindow;
class SceneWindow;
class MemoryManagerWindow;
class GameWindow;

// used for edit & play modes
enum class EditorMode {
	Edit,
	Play,
	Pause
};

// handles everything dealing with ImGui and editor state/debug information
// this will clear up a lot of stuff in the actual scene and also make it way easier to manage all the windows
class EditorManager {
	// deleting copy and move constructers, setting up singleton
	EditorManager() = default;
	EditorManager(const EditorManager&) = delete;
	EditorManager(EditorManager&&) = delete;
	EditorManager& operator=(const EditorManager&) = delete;
	EditorManager& operator=(EditorManager&&) = delete;
private:
	bool imguiInit = false;

	// scene graph reference (might change this after since scenegraph is a singleton now, I just don't want anything to break) TODO
	SceneGraph* sceneGraph = nullptr;

	EditorMode currentMode = EditorMode::Edit;
	std::string pendingFocusWindow;
	EditorCamera editorCamera;
	uint32_t lastSelectedActorID = 0;

	// editor manager handles all the windows
	std::unique_ptr<HierarchyWindow> hierarchyWindow;
	std::unique_ptr<InspectorWindow> inspectorWindow;
	std::unique_ptr<ProjectWindow> projectWindow;
	std::unique_ptr<SceneWindow> sceneWindow;
	std::unique_ptr<GameWindow> gameWindow;
	std::unique_ptr<MemoryManagerWindow> memoryWindow;

	std::unordered_map<std::string, bool> windowStates;

	// Renaming was such a headache because information from the inspector window needed to be transfered over to the hierarchy window so that it could update with the actors new name
	// and since each window was its own thing before, information was transfered with the scenegraph but then the actor in the scenegraph would get dereferenced...
	// so now that every window is centerlized here, I'm going to use it to pass information to and from windows
	struct RenameOperation {
		std::string oldName;
		std::string newName;
		bool pending = false;
	};

	RenameOperation pendingRename;

	// save and load dialogs
	struct SaveLoadDialog {
		bool showSaveDialog = false;
		bool showLoadDialog = false;
		std::string fileName = "";
	};

	SaveLoadDialog saveLoadDialog;

	void ShowSaveDialog();
	void ShowLoadDialog();

	void RenderMainMenuBar();
	void RenderEditorToolbar();
	void EditorStyleColors();

	// helper functions dealing with window management
	bool IsWindowOpen(const std::string& windowName_) const {
		auto it = windowStates.find(windowName_);
		return it != windowStates.end() ? it->second : false;
	} // conditional operator (if windowname is found return its bool otherwise its false)

	void SetWindowState(const std::string& windowName_, bool isOpen_) { windowStates[windowName_] = isOpen_; }

	bool* GetWindowStatePtr(const std::string& windowName_) { return &windowStates[windowName_]; }

	struct EditorIcons {
		Ref<MaterialComponent> playIcon;
		Ref<MaterialComponent> pauseIcon;
		Ref<MaterialComponent> stopIcon;
		Ref<MaterialComponent> stepIcon;

		// ProjectWindow
		Ref<MaterialComponent> folderIcon;
		Ref<MaterialComponent> meshIcon;
		Ref<MaterialComponent> textureIcon;
		Ref<MaterialComponent> materialIcon;
		Ref<MaterialComponent> shaderIcon;
		Ref<MaterialComponent> scriptIcon;
		Ref<MaterialComponent> animationIcon;
		Ref<MaterialComponent> sceneIcon;
		Ref<MaterialComponent> prefabIcon;
		Ref<MaterialComponent> glslIcon;
		Ref<MaterialComponent> unknownIcon;
	};

	EditorIcons editorIcons;

	void CreateEditorIcons();

public:
	// Meyers Singleton (from JPs class)
	static EditorManager& getInstance();
	~EditorManager();

	// Editor lifetime
	bool Initialize(SDL_Window* window_, SDL_GLContext context_, SceneGraph* sceneGraph_);
	void Shutdown();
	void HandleEvents(const SDL_Event& event) const;
	void RenderEditorUI();
	bool IsInitialized() const { return imguiInit; }

	// Editor states
	EditorMode GetEditorMode() const { return currentMode; }
	void SetEditorMode(EditorMode mode) { currentMode = mode; }
	bool isEditMode() const { return currentMode == EditorMode::Edit; }
	bool isPlayMode() const { return currentMode == EditorMode::Play; }
	bool isPaused() const { return currentMode == EditorMode::Pause; }

	// EditorManager/Current scene saving/loading
	void SaveScene(const std::string& name = "");
	void LoadScene(const std::string& name = "");

	// editor state functions
	void Play();
	void Stop();
	void Pause();

	/// <summary>
	/// Registers a window to the editor manager (adds it to window states)
	/// </summary>
	/// <param name="windowName">name of the window</param>
	/// <param name="initialState">open or closed, defaulted to true for open</param>
	void RegisterWindow(const std::string& windowName_, bool initialState_ = true) { windowStates[windowName_] = initialState_; };

	// helper functions for dealing with renaming (inspector renames and sends info to the hierarchy)
	void RequestActorRename(const std::string& oldName_, const std::string& newName_);
	bool HasPendingRename() const { return pendingRename.pending; }
	std::pair<std::string, std::string> ConsumePendingRename();

	EditorManager::EditorIcons getEditorIcons() { return editorIcons; }

	// get editor camera overloads
	EditorCamera& getEditorCamera() { return editorCamera; }
	const EditorCamera& getEditorCamera() const { return editorCamera; }

	uint32_t GetLastSelected() const { return lastSelectedActorID; }
	void SetLastSelected(uint32_t id) { lastSelectedActorID = id; ClearSelectedAsset(); }

	void UpdateActorHierarchy();

	// struct that contains the data for an asset payload
	struct ProjectDragPayload {
		char absolutePath[512];
		char assetName[128];
		char componentType[64];
	};

	struct SelectedAsset {
		fs::path absolutePath;
		std::string assetName;
		std::string componentType;
		bool isSet = false;
	};

	void SetSelectedAsset(const SelectedAsset& asset) {
		selectedAsset = asset;
		SceneGraph::getInstance().debugSelectedAssets.clear();
	}
	void ClearSelectedAsset();
	const SelectedAsset& GetSelectedAsset() const { return selectedAsset; }
	void UpdateProjectWindow();

private:
	SelectedAsset selectedAsset;

	bool showProjectSettings = false;
	void ShowProjectSettingsWindow();

	// temporary save file location
	std::string tempSaveFile = "tempsave";
};