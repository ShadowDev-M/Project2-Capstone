#pragma once

class ShaderComponent;
class Actor;

// core variables a renderpass needs
struct RenderContext {
	Matrix4 view;
	Matrix4 projection;
	Vec3 cameraPos;
	bool isGameView = false;
};

class Renderer
{
	// delete copy and move constructers
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator = (const Renderer&) = delete;
	Renderer& operator = (Renderer&&) = delete;

	void UploadLightUniformsToAllShaders();
	void RenderActors(const RenderContext& ctx) const;
	void RenderColliderDebug(const RenderContext& ctx) const;

	GLenum drawMode = GL_FILL;

	// all required shaders
	Ref<ShaderComponent> pickerShader;
	Ref<ShaderComponent> outlineShader;
	Ref<ShaderComponent> animatedShader;
	Ref<ShaderComponent> animatedOutlineShader;
	Ref<ShaderComponent> multiPhongShader;

public:
	// Meyers Singleton
	static Renderer& getInstance() {
		static Renderer instance;
		return instance;
	}
	
	bool OnCreate();
	void OnDestroy();

	void RenderSceneView(); // uses EditorCamera
	void RenderGameView(); // uses MainCamera

	Ref<Actor> PickActor(int mouseX, int mouseY);

	// for drawmode
	GLenum GetDrawMode() const { return drawMode; }
	void SetDrawMode(GLenum drawMode_) { drawMode = drawMode_; }

	bool showSceneGizmos = true;
	bool showGameGizmos = false;
};

