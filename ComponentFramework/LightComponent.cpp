#include "LightComponent.h"
#include "SceneGraph.h"
bool LightComponent::OnCreate()
{
    SceneGraph::getInstance().ValidateAllLights();

    return true;
}
