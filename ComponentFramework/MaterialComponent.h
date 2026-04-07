#pragma once

enum TexType {
	DIFFUSE,
	SPECULAR,
	NORMAL,
	ROUGHNESS,
	METALLIC
};

class MaterialComponent : public Component {
private:
	GLuint diffuseID;
	GLuint specularID;
	GLuint normalID;
	GLuint roughnessID;
	GLuint metallicID;

	std::string diffuse;
	std::string specular;
	std::string normal;
	std::string roughness;
	std::string metallic;

	bool isPBR;
public:
	MaterialComponent(Component* parent_, const char* diffuse_, const char* specular_ = "", const char* normal_ = "", const char* roughness_ = "", const char* metallic_ = "");
	virtual ~MaterialComponent();

	// getters: IDs
	inline GLuint getDiffuseID() const { return diffuseID; }
	inline GLuint getSpecularID() const { return specularID; }
	inline GLuint getNormalID() const { return normalID; }
	inline GLuint getRoughnessID() const { return roughnessID; }
	inline GLuint getMetallicID() const { return metallicID; }
	
	// getters: file path
	const char* getDiffuseName() const { return diffuse.c_str(); }
	const char* getSpecularName() const { return specular.c_str(); }
	const char* getNormalName() const { return normal.c_str(); }
	const char* getRoughnessName() const { return roughness.c_str(); }
	const char* getMetallicName() const { return metallic.c_str(); }
	bool getIsPBR() const { return isPBR; }

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
	void setRoughMap(std::string roughness_) {
		roughness = roughness_;
	}
	void setMetalMap(std::string metallic_) {
		metallic = metallic_;
	}
	///
	bool LoadImage(const char* filename, TexType type);

	// for manual material creation
	void InjectDiffuseID(GLuint id) { diffuseID = id; }
	void ForceCreated() { isCreated = true; }

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

