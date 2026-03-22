#include "pch.h"
#include "AnimationSystem.h"
#include "AnimatorComponent.h"
#include "MeshComponent.h"
#include "SceneGraph.h"

AnimationSystem::~AnimationSystem()
{
	StopMeshLoadingWorker();
}

void AnimationSystem::StartMeshLoadingWorker()
{
	shouldStop = false;
	workerThread = std::thread(&AnimationSystem::MeshLoadingWorker, this);
}

void AnimationSystem::StopMeshLoadingWorker()
{
	shouldStop = true;
	if (workerThread.joinable()) {
		workerThread.join();
	}
}

void AnimationSystem::Update(float deltaTime) {
	AnimationClip::updateClipTimes(deltaTime);
}

void AnimationSystem::ProcessMainThreadTasks()
{
	std::unique_lock<std::mutex> lock(taskMutex);
	while (!mainThreadTasks.empty()) {
		auto task = std::move(mainThreadTasks.front());
		mainThreadTasks.pop();
		lock.unlock();  // Release lock before OpenGL
		task();
		lock.lock();
	}
}

void AnimationSystem::PushMeshToWorker(Ref<MeshComponent> mesh)
{
	if (!mesh->queryLoadStatus()) {
		std::lock_guard<std::mutex> lock(queueMutex);
		auto it = std::find(workerQueue.begin(), workerQueue.end(), mesh);
		if (it == workerQueue.end()) {
			workerQueue.push_back(mesh);
		}
	}
}

void AnimationSystem::PushAnimationToWorker(Ref<Animation> animation)
{
	if (!animation->queryLoadStatus()) {
		std::lock_guard<std::mutex> lock(queueMutex);
		auto it = std::find(workerQueue.begin(), workerQueue.end(), animation);
		if (it == workerQueue.end()) {
			workerQueue.push_back(animation);
		}
	}
}

void AnimationSystem::ScheduleOnMain(std::function<void()> task)
{
	{
		std::lock_guard<std::mutex> lock(taskMutex);
		mainThreadTasks.push(std::move(task));
	}
	taskCV.notify_one();
}

bool AnimationSystem::queryMeshLoadStatus(const std::string& name)
{
	for (auto& [id, actor] : SceneGraph::getInstance().getAllActors()) {
		Ref<MeshComponent> queriedMesh = actor->GetComponent<MeshComponent>();
		if (queriedMesh && queriedMesh->getMeshName() == name.c_str()) {
			return !queriedMesh->queryLoadStatus();
		}
	}
	return false;
}

void AnimationSystem::MeshLoadingWorker()
{
	while (!shouldStop) {
		Ref<MeshComponent> model;
		Ref<Animation> animation;

		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (workerQueue.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}

			model = std::dynamic_pointer_cast<MeshComponent>(workerQueue.back());
			animation = std::dynamic_pointer_cast<Animation>(workerQueue.back());
			workerQueue.pop_back();
		}

		if (model) {
			model->InitializeMesh();
			ScheduleOnMain([model]() {
				model->storeLoadedModel();
			});
		}
		else if (animation) {
			animation->InitializeAnimation();
		}
	}
}