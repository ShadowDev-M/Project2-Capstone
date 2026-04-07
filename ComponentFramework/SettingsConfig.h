#pragma once
#include <string>

// struct that holds all engine/game settings related data
// TODO: simpleini for saving and loading ini/cfg files (possibly could do it through xml too but that might not be the best performance wise)
struct SettingsConfig
{
	// used to set windowtitle to whatever, (like what the current scene is) its a small thing but is still nice to have, this doesn't really need to be saved to a config file unlike the rest
	std::string windowTitle = "Game Engine";
	
	// FBO
	int renderWidth = 1280;
	int renderHeight = 720;

	// actual window dimensions
	int displayWidth = 1280;
	int displayHeight = 720;

	// TODO: implement functionailty for this stuff later
	int targetFPS = 60;
	bool vsync = false;
};

