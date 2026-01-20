#pragma once
#include "MemorySize.h"

#include "SceneGraph.h"

class MemoryManagerWindow
{
	// delete the move and copy constructers
	MemoryManagerWindow(const MemoryManagerWindow&) = delete;
	MemoryManagerWindow(MemoryManagerWindow&&) = delete;
	MemoryManagerWindow& operator = (const MemoryManagerWindow&) = delete;
	MemoryManagerWindow& operator = (MemoryManagerWindow&&) = delete;
	

public:
	explicit MemoryManagerWindow(SceneGraph* sceneGraph_);
	~MemoryManagerWindow() {}

	void ShowMemoryManagerWindow(bool* pOpen);
};


