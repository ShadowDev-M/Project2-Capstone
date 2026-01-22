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

