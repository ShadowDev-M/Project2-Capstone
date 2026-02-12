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
#define BONE_WEIGHTS_SIZE 4

void MeshComponent::storeLoadedModel()
{
    //printf("\n=== RAW BONE IDS FROM MESH LOADER ===\n");
    //printf("BONE_WEIGHTS_SIZE = %d\n", BONE_WEIGHTS_SIZE);
    //printf("boneIds.size() = %zu, vertices.size() = %zu\n", boneIds.size(), vertices.size());
    //printf("Expected boneIds: %zu\n", vertices.size() * BONE_WEIGHTS_SIZE);

    //// **FIXED INDEXING** - Verify bounds first
    //size_t expectedSize = vertices.size() * BONE_WEIGHTS_SIZE;
    //if (boneIds.size() != expectedSize) {
    //    printf("*** FATAL ERROR ***: boneIds.size()=%zu != %zu*4=%zu\n",
    //        boneIds.size(), vertices.size(), expectedSize);
    //    return;
    //}

    //for (size_t v = 0; v < std::min<size_t>(10, vertices.size()); v++) {
    //    size_t base = v * BONE_WEIGHTS_SIZE;  // Use constant!
    //    if (base + BONE_WEIGHTS_SIZE <= boneIds.size()) {
    //        printf("V[%3zu] RAW boneIds=[%3d,%3d,%3d,%3d]\n", v,
    //            boneIds[base + 0], boneIds[base + 1], boneIds[base + 2], boneIds[base + 3]);
    //    }
    //}
    //printf("Vertices: %zu, Expected bones: %zu, Got boneIds: %zu\n",
    //    vertices.size(), expectedSize, boneIds.size());


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
void ReadNodeHierarchy(Skeleton* skeleton, aiNode* node, Bone* parentBone) {
    // Check if this node corresponds to a bone
    std::string nodeName(node->mName.C_Str());
    Bone* bone = skeleton->FindBone(nodeName);

    if (bone) {
        // Link to parent
        if (parentBone) {
            bone->parent = parentBone;
            parentBone->children.push_back(bone);
        }

        // Debug print
        std::cout << "Linked " << nodeName
            << " parent: " << (parentBone ? parentBone->name : "ROOT") << std::endl;
    }

    // Recurse children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ReadNodeHierarchy(skeleton, node->mChildren[i], bone ? bone : parentBone);
    }
}


void MeshComponent::LoadModel(const char* filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP::%s\n", importer.GetErrorString());
        return;
    }

    aiMesh* mesh = scene->mMeshes[0];
    vertices.clear(); normals.clear(); uvCoords.clear();

    printf("Mesh vertices: %u, faces: %u\n", mesh->mNumVertices, mesh->mNumFaces);

    std::vector<unsigned int> renderToUnique;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) continue;
        for (int j = 0; j < 3; j++) {
            unsigned int uniqueIdx = face.mIndices[j];
            vertices.push_back({ mesh->mVertices[uniqueIdx].x, mesh->mVertices[uniqueIdx].y, mesh->mVertices[uniqueIdx].z });
            normals.push_back({ mesh->mNormals[uniqueIdx].x, mesh->mNormals[uniqueIdx].y, mesh->mNormals[uniqueIdx].z });
            if (mesh->HasTextureCoords(0))
                uvCoords.push_back({ mesh->mTextureCoords[0][uniqueIdx].x, mesh->mTextureCoords[0][uniqueIdx].y });
            else
                uvCoords.push_back({ 0.0f, 0.0f });
            renderToUnique.push_back(uniqueIdx);

        }
    }

    if (mesh->HasBones()) {
        skeleton = std::make_unique<Skeleton>();

        printf("Creating %u bones...\n", mesh->mNumBones);
        int boneId = 0;
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* aiBone = mesh->mBones[i];
            printf("Bone %2d: %s\n", boneId, aiBone->mName.C_Str());

            auto bone = std::make_unique<Bone>(aiBone, boneId);
            skeleton->boneMap[aiBone->mName.C_Str()] = bone.get();
            skeleton->bones.push_back(std::move(bone));
            boneId++;
        }

        boneIds.assign(vertices.size() * BONE_WEIGHTS_SIZE, 0);
        boneWeights.assign(vertices.size() * BONE_WEIGHTS_SIZE, 0.0f);

        ReadNodeHierarchy(skeleton.get(), scene->mRootNode, nullptr);

        skeleton->globalInverseTransform = (ConversionAiMatrix4::AiToMatrix4(scene->mRootNode->mTransformation));

        std::vector<std::vector<size_t>> uniqueToRender(mesh->mNumVertices);
        for (size_t renderIdx = 0; renderIdx < renderToUnique.size(); renderIdx++) {
            uniqueToRender[renderToUnique[renderIdx]].push_back(renderIdx);
        }

        int assignedWeights = 0;
        for (unsigned int b = 0; b < mesh->mNumBones; b++) {
            aiBone* aiBone = mesh->mBones[b];
            Bone* bone = skeleton->boneMap[aiBone->mName.C_Str()];

            if (!bone) {
                printf("ERROR: Bone %s (index %d) NOT FOUND in boneMap!\n", aiBone->mName.C_Str(), b);
                continue;
            }

            unsigned int boneIndex = bone->id;
            printf("Processing bone %d (%s): %u weights\n", boneIndex, aiBone->mName.C_Str(), aiBone->mNumWeights);

            for (unsigned int w = 0; w < aiBone->mNumWeights; w++) {
                aiVertexWeight& weight = aiBone->mWeights[w];
                if (weight.mVertexId >= mesh->mNumVertices) continue;

                for (size_t renderIdx : uniqueToRender[weight.mVertexId]) {
                    size_t boneDataIdx = renderIdx * BONE_WEIGHTS_SIZE;
                    for (int slot = 0; slot < BONE_WEIGHTS_SIZE; slot++) {
                        if (boneWeights[boneDataIdx + slot] < 0.001f) {
                            boneWeights[boneDataIdx + slot] = weight.mWeight;
                            boneIds[boneDataIdx + slot] = boneIndex;
                            assignedWeights++;
                            break;
                        }
                    }
                }
            }
        }

        printf("Total bone weights assigned: %d\n", assignedWeights);

        // Normalize
        for (size_t renderIdx = 0; renderIdx < vertices.size(); renderIdx++) {
            float sum = 0.0f;
            size_t start = renderIdx * BONE_WEIGHTS_SIZE;
            for (int k = 0; k < BONE_WEIGHTS_SIZE; k++) {
                sum += boneWeights[start + k];
            }
            if (sum > 0.001f) {
                for (int k = 0; k < BONE_WEIGHTS_SIZE; k++) {
                    boneWeights[start + k] /= sum;
                }
            }
        }

        // REPLACE the FINAL DEBUG section in LoadModel:
        printf("=== FINAL BONE IDS ===\n");
        printf("boneIds.size() = %zu, vertices.size() = %zu\n", boneIds.size(), vertices.size());

        // **FIND vertices with Hips (bone 0) weights**
        int hipsFound = 0;
        for (size_t i = 0; i < vertices.size() && hipsFound < 10; i++) {
            size_t base = i * BONE_WEIGHTS_SIZE;
            if (boneIds[base + 0] == 0 || boneIds[base + 1] == 0 ||
                boneIds[base + 2] == 0 || boneIds[base + 3] == 0) {
                printf("V[%6zu] boneIds=[%2d,%2d,%2d,%2d]\n", i,
                    boneIds[base + 0], boneIds[base + 1], boneIds[base + 2], boneIds[base + 3]);
                hipsFound++;
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
    printf("=== HIERARCHY DEBUG ===\n");
    int totalChildren = 0;
    for (auto& meshBonePtr : skeleton->bones) {
        printf("Bone %s (OF MAIN MESH) children: %zu\n", meshBonePtr->name.c_str(), meshBonePtr->children.size());
        totalChildren += meshBonePtr->children.size();
    }
    printf("Total children links: %d (should = %zu)\n", totalChildren, skeleton->bones.size() - 1);
}



void MeshComponent::StoreMeshData(GLenum drawmode_) {
    drawmode = drawmode_;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (skeleton) {
        //SceneGraph::useAnimShader();
        std::vector<Vertex> interleaved(vertices.size());

        // SAFETY: Check bone data sizes match vertex count
        size_t expectedBoneCount = vertices.size() * 4;
        printf("Vertices: %zu, Expected bones: %zu, Got boneIds: %zu\n",
            vertices.size(), expectedBoneCount, boneIds.size());

        for (size_t i = 0; i < vertices.size(); i++) {
            interleaved[i].pos = vertices[i];
            interleaved[i].normal = normals[i];
            interleaved[i].uv = uvCoords[i];

            // SAFE BOUNDS - don't crash!
            interleaved[i].boneIds[0] = (i * 4 + 0 < boneIds.size()) ? boneIds[i * 4 + 0] : 0;
            interleaved[i].boneIds[1] = (i * 4 + 1 < boneIds.size()) ? boneIds[i * 4 + 1] : 0;
            interleaved[i].boneIds[2] = (i * 4 + 2 < boneIds.size()) ? boneIds[i * 4 + 2] : 0;
            interleaved[i].boneIds[3] = (i * 4 + 3 < boneIds.size()) ? boneIds[i * 4 + 3] : 0;

            interleaved[i].boneWeights[0] = (i * 4 + 0 < boneWeights.size()) ? boneWeights[i * 4 + 0] : 0.0f;
            interleaved[i].boneWeights[1] = (i * 4 + 1 < boneWeights.size()) ? boneWeights[i * 4 + 1] : 0.0f;
            interleaved[i].boneWeights[2] = (i * 4 + 2 < boneWeights.size()) ? boneWeights[i * 4 + 2] : 0.0f;
            interleaved[i].boneWeights[3] = (i * 4 + 3 < boneWeights.size()) ? boneWeights[i * 4 + 3] : 1.0f; // Default identity

        }

        // SINGLE BUFFER - totalSize = vertices * sizeof(Vertex)
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            interleaved.size() * sizeof(Vertex),  //  YOUR TOTAL SIZE
            interleaved.data(),
            GL_STATIC_DRAW
        );

        // REFERENCE ATTRIBUTE SETUP
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * 4));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * 4));
        glEnableVertexAttribArray(3); glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)(8 * 4));   // boneIds
        glEnableVertexAttribArray(4); glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(12 * 4)); // boneWeights

    }
    else {
        // NON-SKINNED - use your existing sequential layout
        size_t totalSize = vertices.size() * sizeof(Vec3) +
            normals.size() * sizeof(Vec3) +
            uvCoords.size() * sizeof(Vec2);  // SIMPLIFIED

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

        size_t offset = 0;
        glBufferSubData(GL_ARRAY_BUFFER, offset, vertices.size() * sizeof(Vec3), vertices.data()); offset += vertices.size() * sizeof(Vec3);
        glBufferSubData(GL_ARRAY_BUFFER, offset, normals.size() * sizeof(Vec3), normals.data()); offset += normals.size() * sizeof(Vec3);
        glBufferSubData(GL_ARRAY_BUFFER, offset, uvCoords.size() * sizeof(Vec2), uvCoords.data());

        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * sizeof(Vec3)));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset));

        glDisableVertexAttribArray(3); glVertexAttribI4i(3, 0, 0, 0, 0);
        glDisableVertexAttribArray(4); glVertexAttrib4f(4, 0, 0, 0, 1);
    }

    dataLength = vertices.size();
    //vertices.clear(); normals.clear(); uvCoords.clear();
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