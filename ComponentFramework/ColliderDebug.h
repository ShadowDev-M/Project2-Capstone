#pragma once
#include "CollisionComponent.h"
#include "ShaderComponent.h"

// debug helper class to visualize collider bounds
// a lot of this is based off last semesters physics raycast code
// since I did all the vertice building I'm just going reuse the code
// although some stuff I had to change because of the way this engine is structed
// as well as I want to use lines and not just dots to visualize the collider bounds (like unity/unreal)

using namespace MATH;

class ColliderDebug {
private:
	// deleting copy and move constructers, setting up singleton
	ColliderDebug() = default;
	ColliderDebug(const ColliderDebug&) = delete;
	ColliderDebug(ColliderDebug&&) = delete;
	ColliderDebug& operator=(const ColliderDebug&) = delete;
	ColliderDebug& operator=(ColliderDebug&&) = delete;
	~ColliderDebug() { OnDestroy(); }

	// Shape.h code, modified to be a struct
	// reason for struct, is instead of having invidual class files for each different shape
	// creating a struct and map to just hold it 
	struct ColliderShape {
		GLuint vao = 0;
		GLuint vbo = 0;
		size_t dataLength = 0;
	};
	std::unordered_map<Ref<CollisionComponent>, ColliderShape> colliderCache;

	Ref<ShaderComponent> debugShader = nullptr;

	// helper function for generating circles (used for sphere and capsule end caps)
	std::vector<Vec3> GenerateCircle(const Vec3& centre_, float radius_, const Vec3& axis_, int segments_ = 16);

	ColliderShape GenerateSphere(const Ref<CollisionComponent>& collision_);
	ColliderShape GenerateCapsule(const Ref<CollisionComponent>& collision_, const Ref<TransformComponent>& transform_);
	ColliderShape GenerateAABB(const Ref<CollisionComponent>& collision_);
	ColliderShape GenerateOBB(const Ref<CollisionComponent>& collision_);

	// helper function for pushing vertices to gpu
	void StoreShapeData(ColliderDebug::ColliderShape& shape, std::vector<MATH::Vec3>& vertices);
	
	// cleans up data for a shape
	void ClearShape(ColliderShape& shape_);

public:
	// Meyers Singleton (from JPs class)
	static ColliderDebug& getInstance() {
		static ColliderDebug instance;
		return instance;
	}

	bool OnCreate();
	void OnDestroy();
	void Render(Ref<CollisionComponent> collision_, Ref<TransformComponent> transform_, const Matrix4& viewMatrix_, const Matrix4& projectionMatrix_);
	void UpdateDebug(Ref<CollisionComponent> collision_);
	void ClearAll();
};