#pragma once
#include <glew.h>
#include "Component.h"
#include <string>
class MaterialComponent: public Component {
private:
	GLuint textureID;
	std::string filename;
	///
public:
	MaterialComponent(Component* parent_,const char* filename_);
	virtual ~MaterialComponent();
	
	inline GLuint getTextureID() const { return textureID; }

	const char* getTextureName() const { return filename.c_str(); }

	///
	bool LoadImage(const char* filename);

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

