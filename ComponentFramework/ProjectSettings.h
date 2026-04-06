#pragma once
#include <string>

struct SceneListData {
	int id = 0; // id gets assigned from order in list
	std::string name; // file stem
	std::string path; // relative path
};

struct ProjectSettings
{
	// can set windowtitle to whatever, currently is used to show the current scene name
	std::string windowTitle = "Game Engine";

	// FBO
	int renderWidth = 1920;
	int renderHeight = 1080;

	// actual window dimensions
	int displayWidth = 1920;
	int displayHeight = 1080;

	// performance settings
	int targetFPS = 60;
	bool vsync = false;

	// global tag list
	std::vector<std::string> tags = { "Untagged", "MainCamera", "Player", "Ground" };

	// scene list, similar to unitys https://docs.unity3d.com/6000.0/Documentation/ScriptReference/SceneManagement.SceneManager.html
	std::vector<SceneListData> sceneList;

	// default/startup scene
	std::string startupScene;
	
	void RebuildIds() { for (int i = 0; i < static_cast<int>(sceneList.size()); i++) sceneList[i].id = i; }

	int GetSceneIdByName(const std::string& name) const {
		for (auto& s : sceneList) if (s.name == name) return s.id;
		return -1;
	}

	const SceneListData* GetSceneById(int id) const {
		for (auto& s : sceneList) if (s.id == id) return &s;
		return nullptr;
	}

	const SceneListData* GetStartupScene() const {
		if (!startupScene.empty()) return GetSceneById(GetSceneIdByName(startupScene));
		return sceneList.empty() ? nullptr : &sceneList[0];
	}
};