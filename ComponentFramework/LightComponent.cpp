#include "pch.h"
#include "LightComponent.h"
#include "SceneGraph.h"

LightComponent::LightComponent(Component* parent_) : Component(parent_){}

LightComponent::LightComponent(Component* parent_, LightType type_, Vec4 spec_, Vec4 diff_, float intensity_): Component(parent_), type(type_), diff(diff_), spec(spec_), intensity(intensity_) {}

bool LightComponent::OnCreate()
{
    SceneGraph::getInstance().ValidateAllLights();

    return true;
}

void LightComponent::OnDestroy()
{

    //SceneGraph::getInstance().ValidateAllLights();

}


