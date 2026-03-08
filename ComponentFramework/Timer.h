#ifndef TIMER_H
#define TIMER_H
#include <SDL.h>


class Timer {
public:
	Timer();
	~Timer();

	Timer(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator=(const Timer&) = delete;
	Timer& operator=(Timer&&) = delete;

	void Start();
	void UpdateFrameTicks();
	float GetDeltaTime() const;
	void LimitFrameRate(int targetFPS_, bool vsync_);
	static void SetSingleEvent(Uint32 interval,void* param);
private:	
	Uint64 frameStart;
	Uint64 frequency;
	float deltaTime = 0.0f;
	static Uint32 singleEventID;
	static Uint32 callBackFuncion(Uint32 interval, void* singleEventParam);
};


#endif
