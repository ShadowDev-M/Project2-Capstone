#pragma once

// creating an enum for FBOs, just to give them names
enum class FBO {
	Scene, // TODO: Change to Edit
	// Play,
	ColorPicker
	// ShadowMap
};

struct FBOData {
	GLuint fbo = 0;
	GLuint texture = 0;
	GLuint depth = 0;
	int width = 0;
	int height = 0;
	bool isCreated = false;
};

class FBOManager
{
private:
	// deleting copy and move constructers, setting up singleton
	FBOManager() = default;
	FBOManager(const FBOManager&) = delete;
	FBOManager(FBOManager&&) = delete;
	FBOManager& operator=(const FBOManager&) = delete;
	FBOManager& operator=(FBOManager&&) = delete;

	// private helpers for creating and destroying fbos
	void createFBO(FBOData& data, int w, int h);
	void destroyFBO(FBOData& data);
	
	std::unordered_map<FBO, FBOData> fbos;

public: 
	// Meyers Singleton (from JPs class)
	static FBOManager& getInstance() {
		static FBOManager instance;
		return instance;
	}

	FBOData& CreateFBO(FBO fbo_, int w, int h);
	void OnDestroy();
	void OnResize(FBO fbo_, int w, int h);

	bool isCreated(FBO fbo_) const;
	void DestroyFBO(FBO fbo_);
	FBOData& getFBO(FBO fbo_);

	std::unordered_map<FBO, FBOData> getAllFBOs() { return fbos; }
};