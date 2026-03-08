#pragma once
#include "SettingsConfig.h"

enum class Source {
	EditorUI,
	Script,
	WindowDrag
};

class ScreenManager
{
public:
	// Meyers Singleton (from JPs class)
	static ScreenManager& getInstance() {
		static ScreenManager instance;
		return instance;
	}

	void Initialize(SDL_Window* window_, const SettingsConfig& cfg_ = {});

	/// <summary>
	/// Call anywhere where a window resize might happen, 
	/// has a dispatcher that deals with different type cases
	/// </summary>
	/// <param name="w"></param>
	/// <param name="h"></param> 
	/// <param name="source_">default soruce is scripting, just makes things easier for the actual scripting implementation, 
	/// just make sure that when calling this anywhere else though to provide it with a differet source</param>
	void HandleResize(int w, int h, Source source_ = Source::Script);

	// setters
	void setTargetFPS(int fps_);
	void setVSync(bool enabled_);
	void setWindowTitle(const std::string& title_);
	
	// getters
	int getRenderWidth() const { return cfg.renderWidth; }
	int getRenderHeight() const { return cfg.renderHeight; }
	int getDisplayWidth() const { return cfg.displayWidth; }
	int getDisplayHeight() const { return cfg.displayHeight; }
	int getTargetFPS() const { return cfg.targetFPS; }
	float getAspectRatio() const { return static_cast<float>(cfg.renderWidth) / static_cast<float>(cfg.renderHeight); }
	const SettingsConfig& getConfig() const { return cfg; }
	SDL_Window* getWindow() const { return window; }

	// Event Dispatcher/Callback System 
	// using in cameracomponent so that the camera projection gets updated on resize, 
	// use this anywhere else something similar to this is also needed
	// basically makes it so theres no need to constantly check if something is resized and then to call the specific function
	using ResizeDispatcher = std::function<void(int w, int h)>;
	int OnRenderResize(ResizeDispatcher dispatch) {
		int id = nextDispatchId++;
		renderResizeCallbacks.emplace_back(id, std::move(dispatch));
		return id;
	}
	void RemoveRenderResizeCallback(int id) {
		renderResizeCallbacks.erase(
			std::remove_if(renderResizeCallbacks.begin(), renderResizeCallbacks.end(), 
				[id](const auto& pair) {return pair.first == id; }), 
			renderResizeCallbacks.end());
	}
	int OnDisplayResize(ResizeDispatcher dispatch) {
		int id = nextDispatchId++;
		displayResizeCallbacks.emplace_back(id, std::move(dispatch));
		return id;
	}
	void RemoveDisplayResizeCallback(int id) {
		displayResizeCallbacks.erase(
			std::remove_if(displayResizeCallbacks.begin(), displayResizeCallbacks.end(),
				[id](const auto& pair) {return pair.first == id; }),
			displayResizeCallbacks.end());
	}

private:
	// deleting copy and move constructers, setting up singleton
	ScreenManager() = default;
	ScreenManager(const ScreenManager&) = delete;
	ScreenManager(ScreenManager&&) = delete;
	ScreenManager& operator=(const ScreenManager&) = delete;
	ScreenManager& operator=(ScreenManager&&) = delete;

	void renderResolutionNotifier(int w, int h) { for (auto& [id, dispatch] : renderResizeCallbacks) dispatch(w, h); }
	void displayResolutionNotifier(int w, int h) { for (auto& [id, dispatch] : renderResizeCallbacks) dispatch(w, h); }

	void SetRenderResolution(int w, int h);
	void SetDisplayResolution(int w, int h);

	bool isGameBuild() const {
#ifdef ENGINE_EDITOR
		return false;
#else
		return true;
#endif
	}

	SDL_Window* window = nullptr;
	SettingsConfig cfg;
	std::vector<std::pair<int, ResizeDispatcher>> renderResizeCallbacks;
	std::vector<std::pair<int, ResizeDispatcher>> displayResizeCallbacks;
	int nextDispatchId = 0;
};

