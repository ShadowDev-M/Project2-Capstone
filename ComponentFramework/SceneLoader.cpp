#include "pch.h"
#include "SceneLoader.h"

// setting statics
std::string SceneLoader::activeName;
int SceneLoader::activeId = -1;
bool SceneLoader::loaded = false;