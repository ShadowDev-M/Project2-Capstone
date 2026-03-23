#pragma once

using namespace MATH;

class ShadowSettings : public Component
{
	ShadowSettings(const ShadowSettings&) = delete;
	ShadowSettings(ShadowSettings&&) = delete;
	ShadowSettings& operator = (const ShadowSettings&) = delete;
	ShadowSettings& operator = (ShadowSettings&&) = delete;
private:
	bool castShadow;

public:
	ShadowSettings(Component* parent_, bool castShadow_);
	virtual ~ShadowSettings();

	bool getCastShadow() const { return castShadow; }

	void setCastShadow(bool castShadow_) {
		castShadow = castShadow_;
	}


	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};