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

    // well I did another hacky fix for the capsule, the axis were stretching out and it didn't look right,
    // so time to build another model matrix
    Matrix4 modelMatrix;
    ColliderShape tempShape;
    const ColliderShape* renderShape = nullptr;

    if (collision_->getType() == ColliderType::Capsule) {
        tempShape = GenerateCapsule(collision_, transform_);
        renderShape = &tempShape;
        modelMatrix = Matrix4();
    }
    else {
        // finding cached collider shape
        auto it = colliderCache.find(collision_);
        if (it == colliderCache.end()) {
            ColliderShape newShape;
            switch (collision_->getType()) {
            case ColliderType::Sphere:
                newShape = GenerateSphere(collision_);
                break;
            case ColliderType::AABB:
                newShape = GenerateAABB(collision_);
                break;
            case ColliderType::OBB:
                newShape = GenerateOBB(collision_);
                break;
            }
            colliderCache[collision_] = newShape;
            it = colliderCache.find(collision_);
        }
        renderShape = &it->second;

        // this is a hacky fix for sphere radius scaling
        // this will probably need to be applied to the capsule at some point, but honestly,
        // if you scaling a capsules x and z to that point, you might as well use a sphere collider
        if (collision_->getType() == ColliderType::Sphere) {
            Vec3 position = transform_->GetPosition();
            Quaternion rotation = transform_->GetOrientation();
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
    }

    if (!renderShape || renderShape->vao == 0) return;

    glUseProgram(debugShader->GetProgram());

    glUniformMatrix4fv(debugShader->GetUniformID("projectionMatrix"), 1, GL_FALSE, projectionMatrix_);
    glUniformMatrix4fv(debugShader->GetUniformID("viewMatrix"), 1, GL_FALSE, viewMatrix_);
    glUniformMatrix4fv(debugShader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);

    Vec3 color = Vec3(0.0f, 1.0f, 0.0f);
    glUniform3fv(debugShader->GetUniformID("colliderColor"), 1, &color.x);
    glUniform1f(debugShader->GetUniformID("uAlpha"), 0.8f);

    glBindVertexArray(renderShape->vao);
    glDrawArrays(GL_LINES, 0, renderShape->dataLength);
    glBindVertexArray(0);

    if (collision_->getType() == ColliderType::Capsule) {
        ColliderShape temp = tempShape;
        ClearShape(temp);
    }
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

    StoreShapeData(shape, vertices);

    return shape;
}

ColliderDebug::ColliderShape ColliderDebug::GenerateCapsule(const Ref<CollisionComponent>& collision_, const Ref<TransformComponent>& transform_)
{
    ColliderShape shape;
    std::vector<Vec3> vertices;

    // this function is a bit different from the others
    // getting the world positions for this one,
    // then in render giving it a macgyvered model matrix
    Vec3 centrePosA = collision_->getWorldCentrePosA(transform_);
    Vec3 centrePosB = collision_->getWorldCentrePosB(transform_);
    
    // manually converting radius to world coords, because of the y 
    Vec3 scale = transform_->GetScale();
    float radius = collision_->getRadius() * std::max(scale.x, scale.z);

    // Capsule.cpp
    // get the axis
    Vec3 axis = centrePosB - centrePosA;
    // normalize the axis to get the axisDirection
    Vec3 axisDirection = VMath::normalize(axis);

    // generating circles for sphere end caps
    std::vector<Vec3> bottomCircle = GenerateCircle(centrePosA, radius, axisDirection);
    std::vector<Vec3> topCircle = GenerateCircle(centrePosB, radius, axisDirection);

    // lambda function to help with converting old vertice points to new lines
    auto convertCircleToLines = [&vertices](const std::vector<Vec3>& circle) {
        for (size_t i = 0; i < circle.size() - 1; i++) {
            vertices.push_back(circle[i]);
            vertices.push_back(circle[i + 1]);
        }
        };

    convertCircleToLines(bottomCircle);
    convertCircleToLines(topCircle);

    // creating vertical lines that will connect the two circles
    int numVert = 4;
    int step = bottomCircle.size() / numVert;
    for (int i = 0; i < numVert; i++) {
        int iStep = i * step;
        if (iStep < bottomCircle.size()) {
            vertices.push_back(bottomCircle[iStep]);
            vertices.push_back(topCircle[iStep]);
        }
    }

    // creating arced lines for the hemispheres
    const int numArcs = 4;
    const int arcSegments = 8;

    for (int arc = 0; arc < numArcs; arc++) {
        // linear interpolation/lerp to get the center of the current cylinders ring at the current height
        int iRing = arc * ((int)bottomCircle.size() / numArcs);
        Vec3 tangent = VMath::normalize(bottomCircle[iRing] - centrePosA);

        for (float seg = 0.0f; seg < arcSegments; seg++) {
            float phi1 = (seg / arcSegments) * (M_PI / 2.0f);
            float phi2 = ((seg + 1) / arcSegments) * (M_PI / 2.0f);
            
            // getting the x,y for the local coords
            Vec3 top1 = centrePosB + axisDirection * (radius * sin(phi1)) + tangent * (radius * cos(phi1));
            Vec3 top2 = centrePosB + axisDirection * (radius * sin(phi2)) + tangent * (radius * cos(phi2));
            vertices.push_back(top1);
            vertices.push_back(top2);

            Vec3 bot1 = centrePosA - axisDirection * (radius * sin(phi1)) + tangent * (radius * cos(phi1));
            Vec3 bot2 = centrePosA - axisDirection * (radius * sin(phi2)) + tangent * (radius * cos(phi2));
            vertices.push_back(bot1);
            vertices.push_back(bot2);
        }
    }

    shape.dataLength = vertices.size();

    StoreShapeData(shape, vertices);

    return shape;
}

ColliderDebug::ColliderShape ColliderDebug::GenerateAABB(const Ref<CollisionComponent>& collision_)
{
    ColliderShape shape;
    std::vector<Vec3> vertices;
    
    Vec3 centre = collision_->getCentre();
    Vec3 halfExtents = collision_->getHalfExtents();

    // Box.cpp
    // calculating the 8 corner positions of the box	
    // to get the location of a specific point on the cube, I need to subtract or add the halfextents from the center (halfExtents represent the distance from the centre)
    Vec3 min = centre - halfExtents;
    Vec3 max = centre + halfExtents;

    // simplyfing corner creation
    Vec3 corners[8] = {
        Vec3(min.x, min.y, min.z), Vec3(max.x, min.y, min.z),
        Vec3(max.x, max.y, min.z), Vec3(min.x, max.y, min.z),
        Vec3(min.x, min.y, max.z), Vec3(max.x, min.y, max.z),
        Vec3(max.x, max.y, max.z), Vec3(min.x, max.y, max.z)
    };
        
    // instead of trinangles, doing lines, so have to push back in a certain order
    
    // Bottom face
    vertices.push_back(corners[0]); 
    vertices.push_back(corners[1]);
    vertices.push_back(corners[1]); 
    vertices.push_back(corners[5]);
    vertices.push_back(corners[5]); 
    vertices.push_back(corners[4]);
    vertices.push_back(corners[4]); 
    vertices.push_back(corners[0]);

    // Top face
    vertices.push_back(corners[3]); 
    vertices.push_back(corners[2]);
    vertices.push_back(corners[2]); 
    vertices.push_back(corners[6]);
    vertices.push_back(corners[6]); 
    vertices.push_back(corners[7]);
    vertices.push_back(corners[7]); 
    vertices.push_back(corners[3]);

    // Edges
    vertices.push_back(corners[0]); 
    vertices.push_back(corners[3]);
    vertices.push_back(corners[1]); 
    vertices.push_back(corners[2]);
    vertices.push_back(corners[5]); 
    vertices.push_back(corners[6]);
    vertices.push_back(corners[4]); 
    vertices.push_back(corners[7]);

    shape.dataLength = vertices.size();

    StoreShapeData(shape, vertices);

    return shape;
}

ColliderDebug::ColliderShape ColliderDebug::GenerateOBB(const Ref<CollisionComponent>& collision_)
{
    ColliderShape shape;
    std::vector<Vec3> vertices;

    Vec3 centre = collision_->getCentre();
    Vec3 halfExtents = collision_->getHalfExtents();
    Quaternion orientation = collision_->getOrientation();

    // basically the extact same as the AABB, but this time with local axes... wow!

    // rotating the orienation around x,y,z axis in order to get local coords
    Vec3 localCoords[3] = { QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), orientation),
    QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), orientation),
    QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), orientation) };

    // calculating the 8 corner positions of the box	
    // to get the location of a specific point on the cube, I need to subtract or add the halfextents from the center (halfExtents represent the distance from the centre)
    // I'm also multiplying each halfextent axis by its local coord axis in order to transform it back into world coords

    // I did corners for the aabb so now I have to reformat this, but it's easier that way, and less confusing tbh, 
    // (when I had the variables as like topLeftFront or bottomLeftBack I could never figure out what the correct corner vertice actually was, but numbers are easy)
    // other than that though this is litteraly just ripped code from what I did last semester
    Vec3 corners[8];
    corners[0] = centre + (-halfExtents.x * localCoords[0]) + (-halfExtents.y * localCoords[1]) + (-halfExtents.z * localCoords[2]);
    corners[1] = centre + (halfExtents.x * localCoords[0]) + (-halfExtents.y * localCoords[1]) + (-halfExtents.z * localCoords[2]);
    corners[2] = centre + (halfExtents.x * localCoords[0]) + (halfExtents.y * localCoords[1]) + (-halfExtents.z * localCoords[2]);
    corners[3] = centre + (-halfExtents.x * localCoords[0]) + (halfExtents.y * localCoords[1]) + (-halfExtents.z * localCoords[2]);
    corners[4] = centre + (-halfExtents.x * localCoords[0]) + (-halfExtents.y * localCoords[1]) + (halfExtents.z * localCoords[2]);
    corners[5] = centre + (halfExtents.x * localCoords[0]) + (-halfExtents.y * localCoords[1]) + (halfExtents.z * localCoords[2]);
    corners[6] = centre + (halfExtents.x * localCoords[0]) + (halfExtents.y * localCoords[1]) + (halfExtents.z * localCoords[2]);
    corners[7] = centre + (-halfExtents.x * localCoords[0]) + (halfExtents.y * localCoords[1]) + (halfExtents.z * localCoords[2]);

    // instead of trinangles, doing lines, so have to push back in a certain order

    // Bottom face
    vertices.push_back(corners[0]);
    vertices.push_back(corners[1]);
    vertices.push_back(corners[1]);
    vertices.push_back(corners[5]);
    vertices.push_back(corners[5]);
    vertices.push_back(corners[4]);
    vertices.push_back(corners[4]);
    vertices.push_back(corners[0]);

    // Top face
    vertices.push_back(corners[3]);
    vertices.push_back(corners[2]);
    vertices.push_back(corners[2]);
    vertices.push_back(corners[6]);
    vertices.push_back(corners[6]);
    vertices.push_back(corners[7]);
    vertices.push_back(corners[7]);
    vertices.push_back(corners[3]);

    // Edges
    vertices.push_back(corners[0]);
    vertices.push_back(corners[3]);
    vertices.push_back(corners[1]);
    vertices.push_back(corners[2]);
    vertices.push_back(corners[5]);
    vertices.push_back(corners[6]);
    vertices.push_back(corners[4]);
    vertices.push_back(corners[7]);

    shape.dataLength = vertices.size();

    StoreShapeData(shape, vertices);

    return shape;
}

void ColliderDebug::StoreShapeData(ColliderDebug::ColliderShape& shape, std::vector<MATH::Vec3>& vertices)
{
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