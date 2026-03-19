#include "pch.h"
#include "ShadowSettings.h"

ShadowSettings::ShadowSettings(Component* parent_, bool castShadow_) : Component(parent_), castShadow(castShadow_)
{
}

ShadowSettings::~ShadowSettings()
{
}

bool ShadowSettings::OnCreate()
{
    return false;
}

void ShadowSettings::OnDestroy()
{
}

void ShadowSettings::Update(const float deltaTime_)
{
}

void ShadowSettings::Render() const
{
}
