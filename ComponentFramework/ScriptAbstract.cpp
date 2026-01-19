#include "ScriptAbstract.h"

ScriptAbstract::ScriptAbstract(Component* parent_, const char* filename_):Component(parent_), filename(filename_)
{
}

ScriptAbstract::~ScriptAbstract()
{
}

void ScriptAbstract::Update(const float deltaTime_)
{
}

bool ScriptAbstract::OnCreate()
{
    return true;
}

void ScriptAbstract::OnDestroy()
{
}

void ScriptAbstract::Render() const
{
}
