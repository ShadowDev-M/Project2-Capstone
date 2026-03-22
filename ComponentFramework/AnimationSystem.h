#pragma once

// calling it an AnimationSystem but its a bit different from the other systems
// could add more or refactor exisiting animation code into here,
// for now though this is a way for me to move all the thread stuff for the animator out of the scenegraph

class MeshComponent;
class Animation;

class AnimationSystem
{
	AnimationSystem() = default;
	AnimationSystem(const AnimationSystem&) = delete;
	AnimationSystem(AnimationSystem&&) = delete;
	AnimationSystem& operator=(const AnimationSystem&) = delete;
	AnimationSystem& operator=(AnimationSystem&&) = delete;

	// Loader	
	std::vector<Ref<Component>> workerQueue;
	std::thread workerThread;
	std::atomic<bool> shouldStop{ false }; 
	std::queue<std::function<void()>> mainThreadTasks;
	std::mutex queueMutex;
	std::mutex taskMutex;
	std::condition_variable taskCV;

	void MeshLoadingWorker();

public: 
	static AnimationSystem& getInstance() {
		static AnimationSystem instance;
		return instance;
	}
	
	~AnimationSystem();

	void StartMeshLoadingWorker();
	void StopMeshLoadingWorker();

	void Update(float deltaTime);

	// Animation functions
	void ProcessMainThreadTasks();
	void PushMeshToWorker(Ref<MeshComponent> mesh);
	void PushAnimationToWorker(Ref<Animation> animation);
	void ScheduleOnMain(std::function<void()> task);
	
	bool queryMeshLoadStatus(const std::string& name);
	bool isAllComponentsLoaded() { return (workerQueue.empty()); }
};

