#pragma once

#include "MMath.h"
#include "assimp/Importer.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/scene.h"     
class ConversionAiMatrix4 {
public:
    static Matrix4 AiToMatrix4(const aiMatrix4x4& aiMat) {
         return Matrix4(
            aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,  // Row 0 = Assimp column 0
            aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,  // Row 1 = Assimp column 1  
            aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,  // Row 2 = Assimp column 2
            aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4   // Row 3 = Assimp column 3
        );
    }
    
};


struct Bone {
    std::string name;
    int id;
    Matrix4 offsetMatrix;  // Bone-to-mesh bind matrix
    Bone* parent = nullptr;
    std::vector<Bone*> children;

    // required for std::make_unique

    Bone() = default;  

    Bone(const aiBone* aiBone, int boneId)
        : name(aiBone->mName.C_Str()),
        id(boneId),
        offsetMatrix(ConversionAiMatrix4::AiToMatrix4(aiBone->mOffsetMatrix)) {
    }

};

class Skeleton {
public:
    Matrix4 globalInverseTransform;
    std::vector<std::unique_ptr<Bone>> bones;
    std::unordered_map<std::string, Bone*> boneMap;

    Bone* FindBone(const std::string& name) {
        auto it = boneMap.find(name);
        return it != boneMap.end() ? it->second : nullptr;
    }
};
