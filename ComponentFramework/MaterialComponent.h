#pragma once

class MaterialComponent : public Component {
private:
	GLuint diffuseID;
	GLuint specularID;

	std::string diffuse;
	std::string specular;
	
	bool isTiled = false;
	Vec2 tileScale = Vec2(1, 1);
	Vec2 tileOffset;

public:
	MaterialComponent(Component* parent_, const char* diffuse_, const char* specular_ = "");
	virtual ~MaterialComponent();

	inline GLuint getDiffuseID() const { return diffuseID; }
	inline GLuint getSpecularID() const { return specularID; }

	const char* getDiffuseName() const { return diffuse.c_str(); }
	const char* getSpecularName() const { return specular.c_str(); }
	bool getIsTiled() const { return isTiled; }
	Vec2 getTileScale() const { return tileScale; }
	Vec2 getTileOffset() const { return tileOffset; }

	void setDiffMap(std::string diffuse_) {
		diffuse = diffuse_;
	}
	
	void setSpecMap(std::string specular_) {
		specular = specular_;
	}

	void setIsTiled(bool isTiled_) {
		isTiled = isTiled_;
	}

	void setTileScale(Vec2 tileScale_) {
		tileScale = tileScale_;
	}

	void setTileOffset(Vec2 tileOffset_) {
		tileOffset = tileOffset_;
	}

	///
	bool LoadImage(const char* filename, bool isSpec);

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

