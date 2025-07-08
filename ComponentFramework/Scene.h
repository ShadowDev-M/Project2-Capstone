#ifndef SCENE_H
#define SCENE_H

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

union SDL_Event;
class Scene  {
public:	
	virtual ~Scene() {}
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void Update(const float deltaTime) = 0;
	virtual void Render() const  = 0 ;
	virtual void HandleEvents(const SDL_Event &sdlEvent) = 0;
	
};
#endif