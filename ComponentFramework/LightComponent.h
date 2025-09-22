#pragma once
#include "Actor.h"
#include "glew.h"
#include "AssetManager.h"

enum class LightType {
	Direction,
	Point,
	Environment,
	Spot
};

class LightComponent : public Actor
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
	LightComponent(Component* parent_, LightType type_, Vec3 pos_, Vec4 spec_, Vec4 diff_, float intensity_) :
		Actor(parent_), type(type_),  spec(spec_), diff(diff_), intensity(intensity_) {}

	LightComponent(Component* parent_, Vec3 pos_, Vec4 spec_, Vec4 diff_, float intensity_) :
		Actor(parent_), spec(spec_), diff(diff_), intensity(intensity_) {
		type = LightType::Point;
	}

	~LightComponent() {

	}

	Vec4 getSpec() { return spec; }
	Vec4 getDiff() { return diff; }
	float getIntensity() { return intensity; }
};