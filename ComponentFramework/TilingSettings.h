#pragma once

class TilingSettings : public Component 
{
	TilingSettings(const TilingSettings&) = delete;
	TilingSettings(TilingSettings&&) = delete;
	TilingSettings& operator = (const TilingSettings&) = delete;
	TilingSettings& operator = (TilingSettings&&) = delete;
private:
	bool isTiled;
	Vec2 tileScale;
	Vec2 tileOffset;

public:
	TilingSettings(Component* parent_, bool isTiled_, Vec2 tileScale_ = Vec2(1.0f, 1.0f), Vec2 tileOffset_ = Vec2(0.0f, 0.0f));
	virtual ~TilingSettings();

	bool getIsTiled() const { return isTiled; }
	Vec2 getTileScale() const { return tileScale; }
	Vec2 getTileOffset() const { return tileOffset; }

	void setIsTiled(bool isTiled_) {
		isTiled = isTiled_;
	}

	void setTileScale(Vec2 tileScale_) {
		tileScale = tileScale_;
	}

	void setTileOffset(Vec2 tileOffset_) {
		tileOffset = tileOffset_;
	}

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

