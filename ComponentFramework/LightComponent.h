#pragma once
#include "Component.h"
#include "glew.h"
#include "AssetManager.h"

enum class LightType {
	Sky,
	Point,
};

class LightComponent : public Component
{
	LightComponent(const LightComponent&) = delete;
	LightComponent(LightComponent&&) = delete;
	LightComponent& operator= (const LightComponent&) = delete;
	LightComponent& operator=(LightComponent&&) = delete;

private:
	LightType type;
	Vec4 spec;
	Vec4 diff;
	float intensity;
	
public:
	LightComponent(Component* parent_, LightType type_, Vec4 spec_, Vec4 diff_, float intensity_) :
		Component(parent_), type(type_),  spec(spec_), diff(diff_), intensity(intensity_) {}

	LightComponent(Component* parent_, Vec4 spec_, Vec4 diff_, float intensity_) :
		Component(parent_), spec(spec_), diff(diff_), intensity(intensity_) {
		type = LightType::Point;
	}

	~LightComponent() {}

	bool OnCreate();
	void OnDestroy();
	void Update(const float deltaTime_) {}
	void Render() const {}
	
	// setters
	void setSpec(Vec4 spec_) {
		spec = spec_;
	}
	void setDiff(Vec4 diff_) {
		diff = diff_;
	}
	void setIntensity(float intensity_) {
		intensity = intensity_;
	}
	void setType(LightType type_) {
		type = type_;
	}
	// getters
	Vec4 getSpec() const { return spec; }
	Vec4 getDiff() const { return diff; }
	GLfloat getIntensity() const { return intensity; }
	LightType getType() { return type; }
};