#include <typeinfo>
#include <iostream>
#include "Timer.h"

Timer::Timer() : frameStart{0}, frequency{0} {}

Timer::~Timer() {}

void Timer::Start() {
	frameStart = SDL_GetPerformanceCounter();
	frequency = SDL_GetPerformanceFrequency();
	deltaTime = 0.0f;
}

void Timer::UpdateFrameTicks() {
	Uint64 now = SDL_GetPerformanceCounter();
	deltaTime = static_cast<float>(now - frameStart) / static_cast<float>(frequency);
	frameStart = now;
}

float Timer::GetDeltaTime() const {
	return deltaTime;
}

void Timer::LimitFrameRate(int targetFPS_, bool vsync_)
{
	// if vsync is enabled then that will handle the fps cap
	if (vsync_ || targetFPS_ <= 0) return;

	const float targetDelta = 1.0f / static_cast<float>(targetFPS_);
	Uint64 now = SDL_GetPerformanceCounter();
	float elapsed = static_cast<float>(now - frameStart) / static_cast<float>(frequency);

	if (elapsed < targetDelta) {
		float remaining = targetDelta - elapsed;

		// Covers SDL_Delay millisecond margin of error
		if (remaining > 0.002f) {
			SDL_Delay(static_cast<Uint32>((remaining - 0.001f) * 1000.0f));
		}

		while (true) {
			now = SDL_GetPerformanceCounter();
			elapsed = static_cast<float>(now - frameStart) / static_cast<float>(frequency);
			if (elapsed >= targetDelta) break;
		}
	}
}

/// Single event stuff
Uint32 Timer::singleEventID = 0; /// initialize the static member

void Timer::SetSingleEvent(Uint32 interval, void* param){ 
	SDL_TimerID id = SDL_AddTimer(interval, callBackFuncion, (void*)param);
	
}


Uint32 Timer::callBackFuncion(Uint32 interval, void* param) {
		SDL_Event event;
		SDL_UserEvent userevent;
		  
		userevent.type = SDL_USEREVENT;
		userevent.code = 0;
		userevent.data1 = (void*)singleEventID;
		userevent.data2 = param;
		
		++singleEventID; /// Inc the ID of each event registered 

		event.type = SDL_USEREVENT;
		event.user = userevent;

		SDL_PushEvent(&event);
		return(0); /// Stop the timer
}