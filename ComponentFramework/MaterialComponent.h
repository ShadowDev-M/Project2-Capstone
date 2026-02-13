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
	GLuint roughnessID;

	std::string diffuse;
	std::string specular;
	std::string normal;
	std::string roughness;
public:
	MaterialComponent(Component* parent_, const char* diffuse_, const char* specular_ = "", const char* normal_ = "", const char* roughness_ = "");
	virtual ~MaterialComponent();

	inline GLuint getDiffuseID() const { return diffuseID; }
	inline GLuint getSpecularID() const { return specularID; }
	inline GLuint getNormalID() const { return normalID; }
	inline GLuint getRoughnessID() const { return roughnessID; }

	const char* getDiffuseName() const { return diffuse.c_str(); }
	const char* getSpecularName() const { return specular.c_str(); }
	const char* getNormalName() const { return normal.c_str(); }
	const char* getNormalName() const { return normal.c_str(); }

	void setDiffMap(std::string diffuse_) {
		diffuse = diffuse_;
	}
	
	void setSpecMap(std::string specular_) {
		specular = specular_;
	}
	
	void setNormMap(std::string normal_) {
		normal = normal_;
	}
	
	void setRoughMap(std::string roughness_) {
		roughness = roughness_;
	}


	///
	bool LoadImage(const char* filename, TexType type);


	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

