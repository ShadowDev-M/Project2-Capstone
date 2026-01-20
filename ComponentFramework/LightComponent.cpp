#include "pch.h"
#include "LightComponent.h"
#include "SceneGraph.h"

LightComponent::LightComponent(Component* parent_): Component(parent_)
{
	type = LightType::Point;
	diff = Vec4(0.2f, 0.2f, 0.2f, 0.2f);
	spec = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	intensity = 1.0f;

}

LightComponent::LightComponent(Component* parent_, LightType type_, Vec4 spec_, Vec4 diff_, float intensity_)
	: Component{ parent_ }, type{ type_ }, spec{ spec_ }, diff{ diff_ }, intensity{ intensity_ } {
}

bool LightComponent::OnCreate()
{
    SceneGraph::getInstance().ValidateAllLights();

    return true;
}

void LightComponent::OnDestroy()
{

    //SceneGraph::getInstance().ValidateAllLights();

}


