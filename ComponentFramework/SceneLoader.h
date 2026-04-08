#pragma once

class SceneLoader
{
	SceneLoader() = default;
	SceneLoader(const SceneLoader&) = delete;
	SceneLoader(SceneLoader&&) = delete;
	SceneLoader& operator = (const SceneLoader&) = delete;
	SceneLoader& operator = (SceneLoader&&) = delete;

public:
	static SceneLoader& getInstance() {
		static SceneLoader instance;
		return instance;
	}
	
	// when a scene from the scenelist is being requested to load, select a type
	enum class RequestType { None, ByName, ById, Next };

	// what a typical request looks like
	struct Request {
		RequestType type = RequestType::None;
		std::string name;
		int id = 0;
	};

	// load scene overloads
	void Load(const std::string& name) { LoadScene(name); }
	void Load(int id) { LoadScene(id); }
	void LoadScene(const std::string& name) {
		pending.type = RequestType::ByName;
		pending.name = name;
	}
	void LoadScene(int id) {
		pending.type = RequestType::ById;
		pending.id = id;
	}
	void LoadNextScene() {
		pending.type = RequestType::Next;
	}

	// active/current scene state getters
	static std::string GetActiveSceneName() { return activeName; }
	static int GetActiveSceneId() { return activeId; }
	static bool IsSceneLoaded() { return loaded; }

	// scene is requesting to load
	bool HasPending() const { return pending.type != RequestType::None; }
	Request ConsumePending() {
		Request r = pending;
		pending = {};
		return r;
	}
	static void SetActiveScene(const std::string& name, int id) {
		activeName = name;
		activeId = id;
		loaded = true;
	}

private:
	Request pending;

	// active/current scene states
	static std::string activeName;
	static int activeId;
	static bool loaded;
};

