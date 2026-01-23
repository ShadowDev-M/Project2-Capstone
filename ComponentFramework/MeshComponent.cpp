#include "pch.h"
#include "MeshComponent.h"

using namespace MATH;

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

#include "AnimatorComponent.h"
#include "Skeleton.h"
#include "SceneGraph.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void MeshComponent::storeLoadedModel()
{

    StoreMeshData(GL_TRIANGLES);
}

MeshComponent::MeshComponent(Component *parent_, const char* filename_): Component(parent_), filename(filename_) {}

MeshComponent::~MeshComponent() {}

bool MeshComponent::OnCreate() {
    if (isCreated == true) return true;
    //InitializeMesh();
   // SceneGraph::getInstance().pushMeshToWorker(this);
    //isCreated == true;
    return true;
}

bool MeshComponent::InitializeMesh() {
    LoadModel(filename.c_str());
    return true;
}

void MeshComponent::LoadSkeleton(const char* filename)
{
    

}

void MeshComponent::LoadModel(const char* filename) {


    //based on this 
    //https://nickthecoder.wordpress.com/2013/01/20/mesh-loading-with-assimp/

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);//aiProcessPreset_TargetRealtime_Fast has the configs you'll need

    aiMesh* mesh = scene->mMeshes[0]; //assuming you only want the first mesh

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];

        for (int j = 0; j < 3; j++)
        {
            aiVector3D aiVertex = mesh->mVertices[face.mIndices[j]];
            Vec3 vertex{ aiVertex.x, aiVertex.y, aiVertex.z };


            aiVector3D aiNormal = mesh->mNormals[face.mIndices[j]];
            Vec3 normal{ aiNormal.x, aiNormal.y, aiNormal.z };


            aiVector3D aiUv = mesh->mTextureCoords[0][face.mIndices[j]];
            Vec2 uvCoord{ aiUv.x, aiUv.y };



            vertices.push_back(vertex);
            normals.push_back(normal);
            uvCoords.push_back(uvCoord);


        }
    }

    if (mesh->HasBones()) {
        //Skeleton
        skeleton = std::make_unique<Skeleton>();

        // heirarchy
        int boneId = 0;
        for (int i = 0; i < mesh->mNumBones; i++) {
            aiBone* aiBone = mesh->mBones[i];
            auto bone = std::make_unique<Bone>(aiBone, boneId++);
            skeleton->boneMap[aiBone->mName.C_Str()] = bone.get();
            skeleton->bones.push_back(std::move(bone));
        }

        int boneIdArraySize = mesh->mNumVertices * BONE_WEIGHTS_SIZE;
        boneIds.assign(boneIdArraySize, 0.0f);
        boneWeights.assign(boneIdArraySize, 0.0f);

        // bone weights
        for (int i = 0; i < mesh->mNumBones; i++) {
            aiBone* aiBone = mesh->mBones[i];
            Bone* bone = skeleton->FindBone(aiBone->mName.data);
            unsigned int boneId = bone->id;

            for (int j = 0; j < aiBone->mNumWeights; j++) {
                aiVertexWeight w = aiBone->mWeights[j];
                unsigned int vertexStart = w.mVertexId * BONE_WEIGHTS_SIZE;

                for (int k = 0; k < BONE_WEIGHTS_SIZE; k++) {
                    if (boneWeights[vertexStart + k] == 0) {
                        boneWeights[vertexStart + k] = w.mWeight;
                        boneIds[vertexStart + k] = boneId;
                        break;
                    }
                }
            }
        }
        AnimatorComponent::queryAllAnimators(this);


        printSkeleton(skeleton.get(), this);
    }
    fullyLoaded = true;

}

void MeshComponent::printSkeleton(const Skeleton* skeleton, MeshComponent* mesh) {
    std::cout << "\n=== SKELETON DEBUG ===\n";
    std::cout << "Bones count: " << skeleton->bones.size() << "\n\n";

    // Print all bones
    for (size_t i = 0; i < skeleton->bones.size(); i++) {
        const Bone* bone = skeleton->bones[i].get();
        std::cout << "Bone[" << bone->id << "] \"" << bone->name << "\"\n";
        std::cout << "  Parent: " << (bone->parent ? bone->parent->name : "none") << "\n";
        std::cout << "  Children count: " << bone->children.size() << "\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  Offset matrix:\n";
        for (int row = 0; row < 4; row++) {
            std::cout << "    " << bone->offsetMatrix[row * 4 + 0] << " "
                << bone->offsetMatrix[row * 4 + 1] << " "
                << bone->offsetMatrix[row * 4 + 2] << " "
                << bone->offsetMatrix[row * 4 + 3] << "\n";
        }
        std::cout << "\n";
    }

    // Print first few vertex bone weights
    std::cout << "=== VERTEX BONE WEIGHTS ===\n";
    int numVertsToPrint = std::min(5, (int)mesh->boneIds.size() / 4);
    for (int v = 0; v < numVertsToPrint; v++) {
        int offset = v * 4;
        std::cout << "Vertex " << v << ": ";
        for (int k = 0; k < 4; k++) {
            std::cout << "B" << (int)mesh->boneIds[offset + k]
                << "(" << std::fixed << std::setprecision(2)
                << mesh->boneWeights[offset + k] << ") ";
        }
        std::cout << "\n";
    }
}


void MeshComponent::StoreMeshData(GLenum drawmode_) {
    drawmode = drawmode_;
/// These just make the code easier for me to read
#define VERTEX_LENGTH 	(vertices.size() * (sizeof(Vec3)))
#define NORMAL_LENGTH 	(normals.size() * (sizeof(Vec3)))
#define TEXCOORD_LENGTH (uvCoords.size() * (sizeof(Vec2)))

	const int verticiesLayoutLocation = 0;
	const int normalsLayoutLocation = 1;
	const int uvCoordsLayoutLocation = 2;

	/// create and bind the VOA
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	/// Create and initialize vertex buffer object VBO
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VERTEX_LENGTH + NORMAL_LENGTH + TEXCOORD_LENGTH, nullptr, GL_STATIC_DRAW);

	/// assigns the addr of "points" to be the beginning of the array buffer "sizeof(points)" in length
	glBufferSubData(GL_ARRAY_BUFFER, 0, VERTEX_LENGTH, &vertices[0]);
	/// assigns the addr of "normals" to be "sizeof(points)" offset from the beginning and "sizeof(normals)" in length  
	glBufferSubData(GL_ARRAY_BUFFER, VERTEX_LENGTH, NORMAL_LENGTH, &normals[0]);
	/// assigns the addr of "texCoords" to be "sizeof(points) + sizeof(normals)" offset from the beginning and "sizeof(texCoords)" in length  
	glBufferSubData(GL_ARRAY_BUFFER, VERTEX_LENGTH + NORMAL_LENGTH, TEXCOORD_LENGTH, &uvCoords[0]);

	glEnableVertexAttribArray(verticiesLayoutLocation);
	glVertexAttribPointer(verticiesLayoutLocation, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(normalsLayoutLocation);
	glVertexAttribPointer(normalsLayoutLocation, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(VERTEX_LENGTH));
	glEnableVertexAttribArray(uvCoordsLayoutLocation);
	glVertexAttribPointer(uvCoordsLayoutLocation, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(VERTEX_LENGTH + NORMAL_LENGTH));


    dataLength = vertices.size();

    /// give back the memory used in these vectors. The data is safely stored in the GPU now
    vertices.clear();
    normals.clear();
    uvCoords.clear();

    /// Don't need these defines sticking around anymore
#undef VERTEX_LENGTH
#undef NORMAL_LENGTH
#undef TEXCOORD_LENGTH

}

void MeshComponent::Render() const {
    glBindVertexArray(vao);
	glDrawArrays(drawmode, 0, dataLength);
	glBindVertexArray(0); // Unbind the VAO
}

void MeshComponent::Render(GLenum drawmode_) const {
    glBindVertexArray(vao);
	glDrawArrays(drawmode_, 0, dataLength);
	glBindVertexArray(0); // Unbind the VAO
}

void MeshComponent::OnDestroy() {
    glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

/// Currently unused.

void MeshComponent::Update(const float deltaTime) {}