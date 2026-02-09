#include "pch.h"
#include "ColliderDebug.h"

bool ColliderDebug::OnCreate() {
	debugShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colliderDebugVert.glsl", "shaders/colliderDebugFrag.glsl");

	if (!debugShader->OnCreate()) return false;

	return true;
}

void ColliderDebug::OnDestroy() {
    ClearAll();

    if (debugShader) {
        debugShader->OnDestroy();
        debugShader = nullptr;
    }
}

void ColliderDebug::Render(Ref<CollisionComponent> collision_, Ref<TransformComponent> transform_, const Matrix4& viewMatrix_, const Matrix4& projectionMatrix_)
{
    if (!collision_ || !debugShader || !transform_) return;

    // finding cached collider shape
    auto it = colliderCache.find(collision_);
    if (it == colliderCache.end()) {
        ColliderShape newShape;
        switch (collision_->getType()) {
        case ColliderType::Sphere:
            newShape = GenerateSphere(collision_);
            break;
        // case
        }

        colliderCache[collision_] = newShape;
        it = colliderCache.find(collision_);
    }

    const ColliderShape& shape = it->second;
    if (shape.vao == 0) return;

    // this is a hacky fix for sphere radius scaling
    // this will probably need to be applied to the capsule at some point, but honestly,
    // if you scaling a capsules x and z to that point, you might as well use a sphere collider

    // basically making a homebrew modelmatrix
    Matrix4 modelMatrix;

    if (collision_->getType() == ColliderType::Sphere) {
        Vec3 position = transform_->GetPosition();
        Quaternion rotation = transform_->GetQuaternion();
        Vec3 scale = transform_->GetScale();

        // converting local to world
        float maxScale = std::max(std::max(scale.x, scale.y), scale.z);
        float worldRadius = collision_->getRadius() * maxScale;

        // manually creating a transformation matrix
        Matrix4 translationMatrix = MMath::translate(position);
        Matrix4 rotationMatrix = MMath::toMatrix4(rotation);
        Matrix4 scaleMatrix = MMath::scale(worldRadius, worldRadius, worldRadius);

        // converting local to world
        Matrix4 worldCentre = MMath::translate(collision_->getCentre() * maxScale);

        // creating model matrix
        modelMatrix = translationMatrix * rotationMatrix * worldCentre * scaleMatrix;
    }
    else {
        modelMatrix = transform_->GetTransformMatrix();

        // converting local to world
        Matrix4 worldCentre = MMath::translate(collision_->getCentre());
        modelMatrix = modelMatrix * worldCentre;
    }

    glUseProgram(debugShader->GetProgram());

    glUniformMatrix4fv(debugShader->GetUniformID("projectionMatrix"), 1, GL_FALSE, projectionMatrix_);
    glUniformMatrix4fv(debugShader->GetUniformID("viewMatrix"), 1, GL_FALSE, viewMatrix_);
    glUniformMatrix4fv(debugShader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);

    Vec3 color = Vec3(0.0f, 1.0f, 0.0f);
    glUniform3fv(debugShader->GetUniformID("colliderColor"), 1, &color.x);
    glUniform1f(debugShader->GetUniformID("uAlpha"), 0.8f);

    glBindVertexArray(shape.vao);
    glDrawArrays(GL_LINES, 0, shape.dataLength);
    glBindVertexArray(0);
}

void ColliderDebug::UpdateDebug(Ref<CollisionComponent> collision_)
{
    auto it = colliderCache.find(collision_);
    if (it != colliderCache.end()) {
        ClearShape(it->second);
        colliderCache.erase(it);
    }
}

std::vector<Vec3> ColliderDebug::GenerateCircle(const Vec3& centre_, float radius_, const Vec3& axis_, int segments_)
{
    std::vector<Vec3> vertices;
    
    // Capsule.cpp 
    // reusing my local coords code I used to create vertices for my capsule
    // this will just make things easier for sphere and capsule generation

    // was reading about parametric surfaces https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/parametric-and-implicit-surfaces.html
    // creating a local cylindrical coordinate system https://en.wikipedia.org/wiki/Cylindrical_coordinate_system
    // polar coordinate system (basically I'm getting the center of the cylinder ring and using the polar cordinates to get the surrondings)
    // x = r cos0
    // y = r sin0
    // z = z

    // (p = perpendicular distance from z axis, phi/azimuth = angle, z = height)

    // creating a local coordinate system (perpendicular axis)  
    Vec3 arbitrary = Vec3(1.0f, 0.0f, 0.0f);
    // a check to avoid parallels
    if (fabs(VMath::dot(axis_, arbitrary)) > 0.9f) {
        arbitrary = Vec3(0.0f, 1.0f, 0.0f);
    }

    // creating perpendicular vectors for local coordinates (orthonormal basis) https://en.wikipedia.org/wiki/Orthonormal_basis
    Vec3 localX = VMath::normalize(VMath::cross(axis_, arbitrary));
    Vec3 localY = VMath::normalize(VMath::cross(axis_, localX));

    const float angleStep = (2.0f * M_PI) / segments_;

    for (int thetaDeg = 0; thetaDeg <= segments_; thetaDeg++)
    {
        float angle = thetaDeg * angleStep;
        // Build a ring
        Vec3 point = centre_ + radius_ * (cos(angle) * localX + sin(angle) * localY);
        // Push the circle point to our vertices array
        vertices.push_back(point);
    }

    return vertices;
}

ColliderDebug::ColliderShape ColliderDebug::GenerateSphere(const Ref<CollisionComponent>& collision_)
{
    ColliderShape shape;
    std::vector<Vec3> vertices;

    Vec3 centre = collision_->getCentre();
    float radius = collision_->getRadius();

    // generating circles for each axis
    std::vector<Vec3> xyCircle = GenerateCircle(centre, radius, Vec3(0, 0, 1));
    std::vector<Vec3> xzCircle = GenerateCircle(centre, radius, Vec3(0, 1, 0));
    std::vector<Vec3> yzCircle = GenerateCircle(centre, radius, Vec3(1, 0, 0));

    // lambda function to help with converting old vertice points to new lines
    auto convertCircleToLines = [&vertices](const std::vector<Vec3>& circle) {
        for (size_t i = 0; i < circle.size() - 1; i++) {
            vertices.push_back(circle[i]);
            vertices.push_back(circle[i + 1]);
        }
    };

    convertCircleToLines(xyCircle);
    convertCircleToLines(xzCircle);
    convertCircleToLines(yzCircle);

    shape.dataLength = vertices.size();

    /// create and bind the VAO
    glGenVertexArrays(1, &shape.vao);
    // Bind means, hey! Im talking to you!
    glBindVertexArray(shape.vao);
    /// Create and initialize vertex buffer object VBO
    glGenBuffers(1, &shape.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);
    // Generate memory for the VRAM of the GPU buffer
    // one giant array
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_STATIC_DRAW);

    // the attributes are the per-vertex stuff like vertices, normals, UVs
    glEnableVertexAttribArray(0);
    // 3 of them is the stride
    // floating point long using GL_FLOAT. So striding by 12 bytes
    // the last argument is a void*, but where does it begin?
    // Its an old C trick to declare a "I don't care" pointer
    // a void * is an integer. A 4 byte unsigned integer
    // and we start at the beginning
    // need a very brutal reinterpret_cast means Dammit!
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), reinterpret_cast<void*>(0));
    glBindVertexArray(0);

    return shape;
}

void ColliderDebug::ClearAll() {
    for (auto& pair : colliderCache) {
        ClearShape(pair.second);
    }
    colliderCache.clear();
}

void ColliderDebug::ClearShape(ColliderShape& shape_)
{
    if (shape_.vao != 0) {
        glDeleteVertexArrays(1, &shape_.vao);
        shape_.vao = 0;
    }
    if (shape_.vbo != 0) {
        glDeleteBuffers(1, &shape_.vbo);
        shape_.vbo = 0;
    }
    shape_.dataLength = 0;
}