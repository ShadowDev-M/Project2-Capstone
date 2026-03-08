#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#include "MemorySize.h"

#include <string>
#include "SceneManager.h"
#include "Debug.h"
#include "SettingsConfig.h"

  
int main(int argc, char* args[]) {
	static_assert(sizeof(void*) == 4, "This program is not ready for 64-bit build");

	Debug::DebugInit("GameEngineLog.txt");
	
	// TODO: When cfg/ini saving and loading is setup replace this with that
	SettingsConfig cfg;
	cfg.windowTitle = "Game Engine";
	cfg.renderWidth = 1280;
	cfg.renderHeight = 720;
	cfg.displayWidth = 1280;
	cfg.displayHeight = 720;
	cfg.targetFPS = 60;
	cfg.vsync = false;

	SceneManager* gsm = new SceneManager();
	if (gsm->Initialize(cfg.windowTitle, cfg.displayWidth, cfg.displayHeight) ==  true) {
		gsm->Run();
	} 
	delete gsm;
	ReportLeaks();

	while (true) {

	}

	_CrtDumpMemoryLeaks();
	std::cout << (int)MEMORY_NUMUSEDBYTES << std::endl;
	exit(0);
}

static const size_t Memory_Total_Size() { return MEMORY_NUMUSEDBYTES; }


