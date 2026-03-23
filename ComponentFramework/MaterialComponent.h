#pragma once

enum TexType {
	DIFFUSE,
	SPECULAR,
	NORMAL
};

class MaterialComponent : public Component {
private:
	GLuint diffuseID;
	GLuint specularID;
	GLuint normalID;

	std::string diffuse;
	std::string specular;
	std::string normal;
	///
public:
	MaterialComponent(Component* parent_, const char* diffuse_, const char* specular_ = "", const char* normal_ = "");
	virtual ~MaterialComponent();

	// getters: IDs
	inline GLuint getDiffuseID() const { return diffuseID; }
	inline GLuint getSpecularID() const { return specularID; }
	inline GLuint getNormalID() const { return normalID; }
	
	// getters: file path
	const char* getDiffuseName() const { return diffuse.c_str(); }
	const char* getSpecularName() const { return specular.c_str(); }
	const char* getNormalName() const { return normal.c_str(); }

	// setters: file path
	void setDiffMap(std::string diffuse_) {
		diffuse = diffuse_;
	}
	void setSpecMap(std::string specular_) {
		specular = specular_;
	}
	void setNormMap(std::string normal_) {
		normal = normal_;
	}
	///
	bool LoadImage(const char* filename, TexType type);

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

