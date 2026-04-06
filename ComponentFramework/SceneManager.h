#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

class SceneManager {
public:

	SceneManager();
	~SceneManager();
	void Run();
	bool Initialize(std::string name_, int width_, int height_);
	void HandleEvents();

private:
	void Update(float deltaTime);
	void Render();

	void ProcessPendingLoad();
	void LoadSceneFile(const std::string& sceneName);

	class Timer* timer;
	class Window* window;

	bool isRunning;
};

#endif // SCENEMANAGER_H