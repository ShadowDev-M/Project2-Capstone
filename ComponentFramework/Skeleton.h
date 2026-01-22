#pragma once

#include "MMath.h"
#include "assimp/Importer.hpp"
#include "assimp/matrix4x4.h"


class ConversionAiMatrix4 {
public:
    static Matrix4 AiToMatrix4(const aiMatrix4x4& aiMat) {
        return Matrix4(
            aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,  // Column 0
            aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,  // Column 1
            aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,  // Column 2
            aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4   // Column 3
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
    std::vector<std::unique_ptr<Bone>> bones;
    std::unordered_map<std::string, Bone*> boneMap;

    Bone* FindBone(const std::string& name) {
        auto it = boneMap.find(name);
        return it != boneMap.end() ? it->second : nullptr;
    }
};
