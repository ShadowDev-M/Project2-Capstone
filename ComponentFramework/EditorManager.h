#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include "XMLManager.h"
#include "Debug.h"
#include "imgui_stdlib.h"
#include <filesystem>

#include "SceneGraph.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "AssetManagerWindow.h"
#include "SceneWindow.h" // scene
#include "MemoryWindow.h"

// pass all the windows + scenegraph
class SceneGraph;
class HierarchyWindow;
class InspectorWindow;
class AssetManagerWindow;
class SceneWindow; // scene
class MemoryManagerWindow; // scene


// this will be used later on for the scene window and edit/play modes, for now just setting up the enum and some functions for it
enum class EditorMode {
	Edit,
	Play, 
	Pause
};

// handles everything dealing with ImGui and editor state/debug information
// this will clear up a lot of stuff in the actual scene and also make it way easier to manage all the windows
class EditorManager {
private:
	// deleting copy and move constructers, setting up singleton
	EditorManager() = default;
	EditorManager(const EditorManager&) = delete;
	EditorManager(EditorManager&&) = delete;
	EditorManager& operator=(const EditorManager&) = delete;
	EditorManager& operator=(EditorManager&&) = delete;

	// ImGui startup info
	bool imguiInit = false;

	// editor state
	EditorMode currentMode = EditorMode::Edit;

	// editor manager handles all the windows
	std::unique_ptr<HierarchyWindow> hierarchyWindow;
	std::unique_ptr<InspectorWindow> inspectorWindow;
	std::unique_ptr<AssetManagerWindow> assetManagerWindow;
	std::unique_ptr<SceneWindow> sceneWindow;
	std::unique_ptr<MemoryManagerWindow> memoryWindow;

	// scene graph reference (might change this after since scenegraph is a singleton now, I just don't want anything to break) TODO
	SceneGraph* sceneGraph = nullptr;

	// window states
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

	bool refreshHierarchy = false;
	bool refreshInspector = false;

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

	// ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	// helper functions dealing with window management
	bool IsWindowOpen(const std::string& windowName_) const {
		auto it = windowStates.find(windowName_);
		return it != windowStates.end() ? it->second : false;
	} // conditional operator (if windowname is found return its bool otherwise its false)

	void SetWindowState(const std::string& windowName_, bool isOpen_) { windowStates[windowName_] = isOpen_; }

	bool* GetWindowStatePtr(const std::string& windowName_) { return &windowStates[windowName_]; }

public:
	// Meyers Singleton (from JPs class)
	static EditorManager& getInstance() {
		static EditorManager instance;
		return instance;
	}

	// next couple functions focus on the startup and shutdown of imgui, now that the editor manager handles everything it'll clear a lot of clutter

	bool Initialize(SDL_Window* window_, SDL_GLContext context_, SceneGraph* sceneGraph_);

	void Shutdown();

	// pass sdl events to imgui
	void HandleEvents(const SDL_Event& event);

	// render all ImGui windows
	void RenderEditorUI();

	// This stuff will be used later on when we setup actual functionalilty for each editor mode, but for now I'll leave them here
	//EditorMode GetEditorMode() const { return currentMode; }
	//void SetEditorMode(EditorMode mode) { currentMode = mode; }
	//bool IsEditMode() const { return currentMode == EditorMode::Edit; }
	//bool IsPlayMode() const { return currentMode == EditorMode::Play; }
	//bool IsPaused() const { return currentMode == EditorMode::Pause; }

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

	
	bool IsInitialized() const { return imguiInit; }
	
	//ImVec4 GetClearColor() const { return clearColor; }
	//void SetClearColor(const ImVec4& color) { clearColor = color; }

};