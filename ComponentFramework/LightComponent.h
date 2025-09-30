#pragma once
#include "Component.h"
#include "glew.h"
#include "AssetManager.h"

enum class LightType {
	Direction,
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
	 
	Vec4 getSpec() { return spec; }
	Vec4 getDiff() { return diff; }
	GLfloat getIntensity() { return intensity; }
	LightType getType() { return type; }
};