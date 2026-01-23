#include "pch.h"

#include "AnimatorComponent.h"
#include "MeshComponent.h"
#include "Skeleton.h"
#include "SceneGraph.h"
#include <vector>
#include <algorithm>


static std::vector<AnimatorComponent*> animators;

void AnimatorComponent::queryAllAnimators(MeshComponent* caller)
{
	for (auto& animator : animators) {
		if (animator->mesh == caller) {
			animator->copySkeletalData();
		}
	}
}

AnimatorComponent::AnimatorComponent(Component* parent_) : Component(parent_)
{
	animators.push_back(this);

	if (parent) {

		Actor* user = dynamic_cast<Actor*>(parent);

		if (user) {

			auto meshShared = user->GetComponent<MeshComponent>();  
			mesh = meshShared.get();

			std::cout << mesh->getMeshName() << std::endl;
		}
		copySkeletalData();
	}
	else {
#ifdef _DEBUG
		Debug::Error("Attempted to create AnimatorComponent for null actor", __FILE__, __LINE__);
#endif
	}


}

AnimatorComponent::~AnimatorComponent()
{

	auto it = std::find(animators.begin(), animators.end(), this);
	if (it != animators.end()) {
		animators.erase(it);
	}
}

void AnimatorComponent::copySkeletalData()
{


		if (mesh && mesh->skeleton) {
			// Reference the shared skeleton (not copy)
			skeleton = mesh->skeleton.get();
			boneIds = mesh->boneIds;
			boneWeights = mesh->boneWeights;

			// Pre-allocate bone matrices for this animator
			finalBoneMatrices.resize(skeleton->bones.size());

			std::cout << "SUCCESSFUL COPY INTO ANIMATOR" << std::endl;
			std::cout << "SUCCESSFUL COPY INTO ANIMATOR" << std::endl;

			std::cout << "SUCCESSFUL COPY INTO ANIMATOR" << std::endl;

		}
	
	
}

bool AnimatorComponent::OnCreate()
{


	return true;
}

void AnimatorComponent::OnDestroy()
{
}

void AnimatorComponent::Render() const
{
}

void AnimatorComponent::Update(const float deltaTime) {
	

}

std::vector<std::string> AnimatorComponent::getBoneVisualData() {
	std::vector<std::string> bonesVisualData;
	for (size_t i = 0; i < skeleton->bones.size(); i++) {
		std::string visualDataString;
		const Bone* bone = skeleton->bones[i].get();
		visualDataString += "Bone[" + std::to_string(bone->id) + "] \"" + bone->name + "\"\n";
		visualDataString += "  Parent: " + std::string(bone->parent ? bone->parent->name : "none") + "\n";
		visualDataString += "  Children count: " + std::to_string(bone->children.size()) + "\n";
		visualDataString += "  Offset matrix:\n";

		char matrixLine[128];
		for (int row = 0; row < 4; row++) {
			snprintf(matrixLine, sizeof(matrixLine), "    %.3f %.3f %.3f %.3f\n",
				bone->offsetMatrix[row * 4 + 0],
				bone->offsetMatrix[row * 4 + 1],
				bone->offsetMatrix[row * 4 + 2],
				bone->offsetMatrix[row * 4 + 3]);
			visualDataString += matrixLine;
		}
		visualDataString += "\n";

		bonesVisualData.push_back(visualDataString);

	}
	return bonesVisualData;
}

std::vector<std::string> AnimatorComponent::getBoneWeightVisualData() {
	std::vector<std::string> bonesVisualData;
	int numVertsToPrint = std::min(5, (int)mesh->boneIds.size() / 4);
	for (int v = 0; v < numVertsToPrint; v++) {
		std::string visualDataString;
		int offset = v * 4;
		char vertexLine[256];
		snprintf(vertexLine, sizeof(vertexLine), "Vertex %d: ", v);
		visualDataString += vertexLine;

		for (int k = 0; k < 4; k++) {
			snprintf(vertexLine, sizeof(vertexLine), "B%d(%.2f) ",
				(int)mesh->boneIds[offset + k], mesh->boneWeights[offset + k]);
			visualDataString += vertexLine;
		}
		visualDataString += "\n";


		bonesVisualData.push_back(visualDataString);
	}

	return bonesVisualData;
}
std::string AnimatorComponent::getSkeletonVisualData()
{

	std::string visualDataString = R"(
	Skeleton Debug
)";

	visualDataString += "Bones count: " + std::to_string(skeleton->bones.size()) + "\n\n";
	return visualDataString;
	// Print all bones
	

	// Print first few vertex bone weights
	
}

