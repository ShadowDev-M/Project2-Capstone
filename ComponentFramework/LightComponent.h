#pragma once
#include "AssetManager.h"

enum class LightType {
	Sky,
	Point,
};

// similar to unity Shadow Type (no shadows, hard shadows, light shadows)
// theres no need for light shadows right now
enum class ShadowType {
	None,
	Hard
};

class LightComponent : public Component
{
	friend class LightingSystem;
	LightComponent(const LightComponent&) = delete;
	LightComponent(LightComponent&&) = delete;
	LightComponent& operator= (const LightComponent&) = delete;
	LightComponent& operator=(LightComponent&&) = delete;

private:
	// Light Settings
	LightType type;
	Vec4 spec;
	Vec4 diff;
	float intensity;

	// Shadow Settings
	ShadowType shadowType = ShadowType::None;
	float shadowNear = 0.1f;
	float shadowFar = 200.0f;
	int shadowResolution = 1024;
	float shadowOrthoSize = 60.0f;
	
public:
	LightComponent(Component* parent_);
	LightComponent(Component* parent_ = nullptr, LightType type_ = LightType::Point, Vec4 spec_ = Vec4(1.0f, 1.0f, 1.0f, 1.0f), Vec4 diff_ = Vec4(0.5f, 0.5f, 0.5f, 1.0f), float intensity_ = 1.0f);
	~LightComponent() {}

	bool OnCreate() { return true; }
	void OnDestroy() {}
	void Update(const float deltaTime_) {}
	void Render() const {}
	
	// Getters & Setters for Light Settings
	LightType getType() const { return type; }
	Vec4 getSpec() const { return spec; }
	Vec4 getDiff() const { return diff; }
	GLfloat getIntensity() const { return intensity; }
	void setType(LightType type_) { type = type_;}
	void setSpec(Vec4 spec_) { spec = spec_; }
	void setDiff(Vec4 diff_) { diff = diff_; }
	void setIntensity(float intensity_) { intensity = intensity_; }

	// Getters & Setters for Shadow Settings
	ShadowType getShadowType() const { return shadowType; }
	float getShadowNear() const { return shadowNear; }
	float getShadowFar() const { return shadowFar; }
	int getShadowResolution() const { return shadowResolution; }
	float getShadowOrthoSize() const { return shadowOrthoSize; }
	void setShadowType(ShadowType type_) { shadowType = type_; }
	void setShadowNear(float near) { shadowNear = std::max(0.0001f, near); }
	void setShadowFar(float far) { shadowFar = far; }
	void setShadowResolution(int resolution) { shadowResolution = resolution; }
	void setShadowOrthoSize(float size) { shadowOrthoSize = std::max(1.0f, size); }

	// helper to see if light is casting shadows
	bool castsShadows() const { return shadowType != ShadowType::None; }
};