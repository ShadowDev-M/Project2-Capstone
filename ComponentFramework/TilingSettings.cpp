#include "pch.h"
#include "TilingSettings.h"

TilingSettings::TilingSettings(Component* parent_, bool isTiled_, Vec2 tileScale_, Vec2 tileOffset_) : Component(parent_), isTiled(isTiled_), tileScale(tileScale_), tileOffset(tileOffset_) {}

TilingSettings::~TilingSettings() {}
bool TilingSettings::OnCreate() { return true; }
void TilingSettings::OnDestroy() {}
void TilingSettings::Update(const float deltaTime) {}
void TilingSettings::Render()const {}