#pragma once
#include "LightComponent.h"

class Actor;
class ShaderComponent;

// matches MultiPhongFarg
// so now instead of the maximum ammount of lights being attached to how many cast shadows,
// it can be separated, so we can have however many lights we want, and however many lights we want to cast shadows,
// of course we still need to create the fbos and everything else for the point lights though
static constexpr int MAX_SHADOW_CASTERS = 4;

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

	// a private struct that holds shadow data,
	// basically is ShadowInfo but with a couple of different things
	struct ShadowInfo {
		// Sky Light
		bool hasSky = false;
		Matrix4 SkyMatrix;
		Vec3 LightDir;

		// Point Light
		int numPointShadows = 0;
		float pointFarPlanes[MAX_SHADOW_CASTERS] = {};

		// Total lights casting shadows
		// -1 no shadow, -2 skylight, <= 0 point lights
		std::vector<int> shadowCasters;
	};

	ShadowInfo info;

	void RenderActors(const RenderContext& ctx) const;
	void RenderColliderDebug(const RenderContext& ctx) const;
	
	// uploading uniforms/variables to shaders
	void UploadLightUniforms(const RenderContext& ctx);
	void UploadTilingUniforms(Ref<ShaderComponent> shader, Ref<Actor> actor) const;
	void UploadShadowUniforms(Ref<ShaderComponent> shader) const;
	void SetUniformShadowPass(Ref<ShaderComponent> shader, Ref<Actor> actor, bool isAnim) const; // sort of separting it into two functions, one that does all the uploading, the other which is a gate

	GLenum drawMode = GL_FILL;

	// all required shaders
	Ref<ShaderComponent> pickerShader;
	Ref<ShaderComponent> outlineShader;
	Ref<ShaderComponent> animatedShader;
	Ref<ShaderComponent> animatedOutlineShader;
	Ref<ShaderComponent> multiPhongShader;
	Ref<ShaderComponent> shadowShader;
	Ref<ShaderComponent> shadowPointShader;
	std::vector<Ref<ShaderComponent>> shaders;

	// fallback components
	Ref<MaterialComponent> fallbackMaterial;
	Ref<ShaderComponent> fallbackShader;

public:
	// Meyers Singleton
	static Renderer& getInstance() {
		static Renderer instance;
		return instance;
	}
	
	bool OnCreate();
	void OnDestroy();

	void ShadowPass(); // called once, both scene and game views use the same shadow pass
	void RenderSceneView(); // uses EditorCamera
	void RenderGameView(); // uses MainCamera

	Ref<Actor> PickActor(int mouseX, int mouseY);

	// for drawmode
	GLenum GetDrawMode() const { return drawMode; }
	void SetDrawMode(GLenum drawMode_) { drawMode = drawMode_; }

	Ref<MaterialComponent> GetFallBackMaterial() const { return fallbackMaterial; }
	Ref<ShaderComponent> GetFallBackShader() const { return fallbackShader; }

	bool showSceneGizmos = true;
	bool showGameGizmos = false;
};

