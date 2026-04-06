#pragma once
#include "ProjectSettings.h"
#include "ProjectSettingsManager.h"

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

	void Initialize(SDL_Window* window_);

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
	float getRenderAspectRatio() const { return static_cast<float>(cfg.renderWidth) / static_cast<float>(cfg.renderHeight); }
	float getDisplayAspectRatio() const { return static_cast<float>(cfg.displayWidth) / static_cast<float>(cfg.displayHeight); }
	const ProjectSettings& getConfig() const { return cfg; }
	SDL_Window* getWindow() const { return window; }

private:
	// deleting copy and move constructers, setting up singleton
	ScreenManager() = default;
	ScreenManager(const ScreenManager&) = delete;
	ScreenManager(ScreenManager&&) = delete;
	ScreenManager& operator=(const ScreenManager&) = delete;
	ScreenManager& operator=(ScreenManager&&) = delete;

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
	ProjectSettings& cfg = ProjectSettingsManager::getInstance().Get();
};