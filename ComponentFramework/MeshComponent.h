#pragma once
#include "Component.h"
#include <glew.h>
#include <vector>
#include "Vector.h"
#include "Raycast.h"
using namespace MATH;

class MeshComponent : public Component {
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent(MeshComponent&&) = delete;
	MeshComponent& operator = (const MeshComponent&) = delete;
	MeshComponent& operator = (MeshComponent&&) = delete;

private:
	// const char* creates an address for the name and shares it between same names
	// std::string compares strings but costs more memory
	std::string filename;
	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvCoords;
	size_t dataLength;
	GLenum drawmode;

	/// Private helper methods
	void LoadModel(const char* filename);
	void StoreMeshData(GLenum drawmode_);
	GLuint vao, vbo;
public:
	
	MeshComponent(Component *parent_,const char* filename_);
	~MeshComponent();
	bool OnCreate() override;
	void OnDestroy() override;
	void Update(const float deltaTime) override;
	void Render() const;
	void Render(GLenum drawmode) const;
	
	std::vector<Vec3> getMeshVertices() { 
		
		LoadModel(filename.c_str());
		
//		std::cout << vertices.size() << std::endl;
		
		return vertices; }

	
	const char* getMeshName() const { return filename.c_str(); }

	size_t getVertices() const {
		return dataLength;
	}
};

