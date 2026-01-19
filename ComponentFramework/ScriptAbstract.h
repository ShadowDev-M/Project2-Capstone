#pragma once

#include "Component.h"
#include <iostream>

class ScriptAbstract : public Component {

	ScriptAbstract(const ScriptAbstract&) = delete;
	ScriptAbstract(ScriptAbstract&&) = delete;
	ScriptAbstract& operator = (const ScriptAbstract&) = delete;
	ScriptAbstract& operator = (ScriptAbstract&&) = delete;

	std::string filename;
	


public:
	ScriptAbstract(Component* parent_, const char* filename_);
	~ScriptAbstract();


	const char* getName() const { return filename.c_str(); }

	void Update(const float deltaTime_) override;
	bool OnCreate() override;
	void OnDestroy() override;
	void Render() const override;


};