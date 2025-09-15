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

class LightActor : public Actor
{
	LightActor(const LightActor&) = delete;
	LightActor(LightActor&&) = delete;
	LightActor& operator= (const LightActor&) = delete;
	LightActor& operator=(LightActor&&) = delete;

private:
	LightType type;
	Vec3 pos;
	Vec4 spec;
	Vec4 diff;
	float intensity;
public:
	LightActor(Component* parent_, LightType type_, Vec3 pos_, Vec4 spec_, Vec4 diff_, float intensity_): 
		Actor(parent_), type(type_), pos(pos_), spec(spec_), diff(diff_), intensity(intensity_){}

	LightActor(Component* parent_, Vec3 pos_, Vec4 spec_, Vec4 diff_, float intensity_) :
		Actor(parent_), pos(pos_), spec(spec_), diff(diff_), intensity(intensity_) {
		type = LightType::Point;
	}

	Vec3 getPos() { return pos; }
	Vec4 getSpec() { return spec; }
	Vec4 getDiff() { return diff; }
	float getIntensity() { return intensity; }

	void render(int numLights);
};