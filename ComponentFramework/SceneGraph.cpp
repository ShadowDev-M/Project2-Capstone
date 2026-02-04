#include "pch.h"
#include "SceneGraph.h"
#include "XMLManager.h"
#include "InputManager.h"
#include <chrono>
#include "AnimatorComponent.h"
#include "Skeleton.h"



void SceneGraph::pushMeshToWorker(MeshComponent* mesh) {

	if (!mesh->queryLoadStatus()) {
		auto it = std::find(workerQueue.begin(), workerQueue.end(), mesh);
		auto itsecond = std::find(finishedQueue.begin(), finishedQueue.end(), mesh);
		if (it == workerQueue.end() && itsecond == finishedQueue.end()) {
			workerQueue.push_back(mesh); 
		}
	}
}

void SceneGraph::pushAnimationToWorker(Animation* animation) {

	if (!animation->queryLoadStatus()) {
		auto it = std::find(workerQueue.begin(), workerQueue.end(), animation);
		auto itsecond = std::find(finishedQueue.begin(), finishedQueue.end(), animation);
		if (it == workerQueue.end() && itsecond == finishedQueue.end()) {
			workerQueue.push_back(animation);
		}
	}
}


void SceneGraph::stopMeshLoadingWorker()
{
	shouldStop = true;
	if (workerThread.joinable()) {
		workerThread.join();
	}
}
void SceneGraph::meshLoadingWorker()
{
	while (!shouldStop) {

		MeshComponent* model = nullptr;
		Animation* animation = nullptr;

		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (workerQueue.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}


			model = dynamic_cast<MeshComponent*>(workerQueue.back());
			animation = dynamic_cast<Animation*>(workerQueue.back());
		}

		if (model) {

			std::cout << "Loading Model: " << model->getMeshName() << std::endl;

			model->InitializeMesh();
			workerQueue.pop_back();

			scheduleOnMain([model]() {

				model->storeLoadedModel();
				});
		}
		else if (animation) {
			std::cout << "Loading Animation: " << animation << std::endl;

			animation->InitializeAnimation();
			workerQueue.pop_back();

			

		}
		else {
			workerQueue.pop_back();

		}
	
	}
}
void SceneGraph::processMainThreadTasks() {
	std::unique_lock<std::mutex> lock(taskMutex);
	while (!mainThreadTasks.empty()) {
		auto task = std::move(mainThreadTasks.front());
		mainThreadTasks.pop();
		lock.unlock();  // Release lock before OpenGL
		task();
		lock.lock();
	}
}
void SceneGraph::scheduleOnMain(std::function<void()> task)
{
	{
		std::lock_guard<std::mutex> lock(taskMutex);
		mainThreadTasks.push(std::move(task));
	}
	taskCV.notify_one();
}

void SceneGraph::storeInitializedMeshData() {

	while (!finishedQueue.empty()) {

		if (dynamic_cast<MeshComponent*> (finishedQueue.back())) {
			dynamic_cast<MeshComponent*> (finishedQueue.back())->storeLoadedModel();
		}

	}

}

bool SceneGraph::queryMeshLoadStatus(std::string name)
{
	for (auto& obj : Actors) {
		
		Ref<MeshComponent> queriedMesh = obj.second->GetComponent<MeshComponent>();
		if (queriedMesh && queriedMesh->getMeshName() == name.c_str()) {
			return queriedMesh->queryLoadStatus();
		}

	}


	return false;
}


SceneGraph::SceneGraph()
{


	int w, h;

	w = SCENEWIDTH;
	h = SCENEHEIGHT;


	//		SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
	createFBOPicking(w, h);

	//fbo for the imgui docking
	createDockingFBO(w, h);

	pickerShader->OnCreate();

	ScriptService::loadLibraries();
	startMeshLoadingWorkerThread();

}

SceneGraph::~SceneGraph()
{
	//end the mesh loading thread
	

	RemoveAllActors();
	pickerShader->OnDestroy();
	glDeleteFramebuffers(1, &pickingFBO);
	glDeleteRenderbuffers(1, &pickingDepth);
	glDeleteTextures(1, &pickingTexture);

	glDeleteFramebuffers(1, &dockingFBO);
	glDeleteRenderbuffers(1, &dockingDepth);
	glDeleteTextures(1, &dockingTexture);
}

void SceneGraph::useDebugCamera()
{
	usedCamera = debugCamera->GetComponent<CameraComponent>();

	checkValidCamera();
}

void SceneGraph::setUsedCamera(Ref<CameraComponent> newCam) {
	if (newCam) {
		usedCamera = newCam;
		newCam->fixCameraToTransform();
		checkValidCamera();

	}
	else if (!newCam) {
		useDebugCamera();
	}
}

void SceneGraph::startMeshLoadingWorkerThread()
{
	workerThread = std::thread(&SceneGraph::meshLoadingWorker, this);
	//t.detach();              
}

Ref<CameraComponent> SceneGraph::getUsedCamera() const
{
	if (!usedCamera || !usedCamera->GetUserActor()) {


		return debugCamera->GetComponent<CameraComponent>();


		std::cout << "ERROR: NO CAMERA EXISTS IN SCENEGRAPH" << std::endl;
	}


	return usedCamera;
}

void SceneGraph::checkValidCamera()
{
	if (!usedCamera || !usedCamera->GetUserActor()) {
		std::cout << "usedCamera is invalid" << std::endl;



		std::cout << "try debugCam" << std::endl;

		if (debugCamera && debugCamera->GetComponent<CameraComponent>()) {
			std::cout << "DebugCam is found" << std::endl;
		}
		else {

			std::cout << "DebugCam is invalid" << std::endl;
			debugCamera = std::make_shared<Actor>(nullptr, "cameraDebugOne");
			debugCamera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));
			debugCamera->OnCreate();

			//doesn't need to be added to the sceneGraph
			//sceneGraph.AddActor(debugCamera);

			debugCamera->AddComponent<CameraComponent>(debugCamera, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
			debugCamera->GetComponent<CameraComponent>()->OnCreate();
			debugCamera->GetComponent<CameraComponent>()->fixCameraToTransform();

			setUsedCamera(debugCamera->GetComponent<CameraComponent>());
		}
		usedCamera = debugCamera->GetComponent<CameraComponent>();


	}
}

void SceneGraph::ValidateAllLights()
{
	if (lightActors.size() != 0) {
		for (std::vector<Ref<Actor>>::iterator it = lightActors.begin(); it != lightActors.end(); ++it) {
			if (!(*it)->ValidateLight()) {

				lightActors.erase(it);

			}
		}
	}
}

bool SceneGraph::AddLight(Ref<Actor> actor)
{
	if (!actor->ValidateLight()) return false;

	lightActors.push_back(actor);


	return true;
}

std::vector<Vec3> SceneGraph::GetLightsPos() const
{
	std::vector<Vec3> lightPositions;

	for (auto& obj : lightActors) {
		lightPositions.push_back(obj->GetComponent<TransformComponent>()->GetPosition());
	}

	return lightPositions;
}

bool SceneGraph::GetLightExist(Ref<Actor> actor)
{
	if (lightActors.size() == 0) return false;
	auto it = std::find(lightActors.begin(), lightActors.end(), actor);

	if (it != lightActors.end()) {
		// Element found
		return true;
	}

	return false;
}

bool SceneGraph::AddActor(Ref<Actor> actor)
{
	if (!actor) {
		Debug::Error("Attempted to add null actor", __FILE__, __LINE__);
		return false;
	}

	const std::string& name = actor->getActorName();
	uint32_t id = actor->getId();

	// check if an actor with this ID already exists
	auto it = Actors.find(id);
	if (it != Actors.end()) {
		Debug::Warning("An actor with ID " + std::to_string(id) + " already exists", __FILE__, __LINE__);
		return false;
	}

	// check to see if there is already an actor with the same name in the map
	auto nameIt = ActorNameToId.find(name);
	if (nameIt != ActorNameToId.end() && nameIt->second != id) {
		Debug::Warning("An actor named: " + name + " already exists", __FILE__, __LINE__);
		return false;
	}

	// add the actor using ID as key
	Actors[id] = actor;
	ActorNameToId[name] = id;

	return true;
}

void SceneGraph::Start()
{

	for (auto& actor : Actors) {
		ScriptService::startActorScripts(actor.second);


	}

	if (!GetActor("Mario")->GetComponent<AnimatorComponent>()->activeClip.animation || !GetActor("Mario")->GetComponent<AnimatorComponent>()->activeClip.animation->queryLoadStatus())
	GetActor("Mario")->GetComponent<AnimatorComponent>()->setAnimation(std::make_shared<Animation>(nullptr, "meshes/dancing.gltf"));


	GetActor("Mario")->GetComponent<AnimatorComponent>()->activeClip.Play();
}


void SceneGraph::Stop()
{
	GetActor("Mario")->GetComponent<AnimatorComponent>()->displayDataTest();
	GetActor("Mario")->GetComponent<AnimatorComponent>()->activeClip.StopPlaying();

	for (auto& actor : Actors) {
		ScriptService::stopActorScripts(actor.second);


		Ref<AnimatorComponent> actorAnimator =  actor.second->GetComponent<AnimatorComponent>();
		if (actorAnimator) {
			actorAnimator->activeClip.StopPlaying();
			actorAnimator->activeClip.currentTime = 0.0f;
		}
	}
}


void SceneGraph::LoadActor(const char* name_, Ref<Actor> parent) {
	
	Ref<Actor> actor_ = std::make_shared<Actor>(parent.get(), name_);

	// if statements to check whether or not a specific component exists
	// added this because before the engine would crash because it would be trying to add an actor that didn't have a mesh/material/shader
	if (XMLObjectFile::hasComponent<MaterialComponent>(name_)) {
		std::string materialName = XMLObjectFile::getComponent<MaterialComponent>(name_);
		if (!materialName.empty()) {
			Ref<MaterialComponent> materialComponent = AssetManager::getInstance().GetAsset<MaterialComponent>(materialName);
			if (materialComponent) {
				actor_->ReplaceComponent<MaterialComponent>(materialComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<ShaderComponent>(name_)) {
		std::string shaderName = XMLObjectFile::getComponent<ShaderComponent>(name_);
		if (!shaderName.empty()) {
			Ref<ShaderComponent> shaderComponent = AssetManager::getInstance().GetAsset<ShaderComponent>(shaderName);
			if (shaderComponent) {
				actor_->ReplaceComponent<ShaderComponent>(shaderComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<MeshComponent>(name_)) {
		std::string meshName = XMLObjectFile::getComponent<MeshComponent>(name_);
		if (!meshName.empty()) {
			Ref<MeshComponent> meshComponent = AssetManager::getInstance().GetAsset<MeshComponent>(meshName);
			if (meshComponent) {
				actor_->ReplaceComponent<MeshComponent>(meshComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<ScriptComponent>(name_)) {
		std::string scriptName = XMLObjectFile::getComponent<ScriptComponent>(name_);
		for (int i = 1; !scriptName.empty(); i++) {

			actor_->AddComponent<ScriptComponent>(actor_.get(), AssetManager::getInstance().GetAsset<ScriptAbstract>(scriptName));
			scriptName = XMLObjectFile::getComponent<ScriptComponent>(name_, i);

			//in case of infinite error
			if (i == 40) break;
		}
	}
	
	actor_->AddComponent<TransformComponent>(std::apply([](auto&&... args) {
		return new TransformComponent(args...);
		}, XMLObjectFile::getComponent<TransformComponent>(name_)));
	


	if (XMLObjectFile::hasComponent<CameraComponent>(name_)) {
		std::cout << "Has a Camera" << std::endl; 
		actor_->AddComponent<CameraComponent>(actor_, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	}

	if (XMLObjectFile::hasComponent<LightComponent>(name_)) {
		std::cout << "Has a Light" << std::endl;

		//tried to apply it directly to addcomponent but it always defaulted yet this works fine ¯\_()_/¯			
		Ref<LightComponent> lightG = Ref<LightComponent>(std::apply([](auto&&... args) {
			return new LightComponent(args...);
			}, XMLObjectFile::getComponent<LightComponent>(name_)));

		if (!actor_->GetComponent<LightComponent>()) {

			actor_->AddComponent(lightG);

			//Light will be Added to scenegraph after entire actor is loaded fully to avoid issues (in LoadActor function)
			
		}


	}

	actor_->OnCreate();
	AddActor(actor_);


}

Ref<Actor> SceneGraph::MeshRaycast(Vec3 start, Vec3 end)
{
	/*float minDistance = 0;
	Ref<Actor> closestSelected = nullptr;

	for (auto& actor : Actors) {
		Ref<Actor> targetActor = actor.second;

		Ref<TransformComponent> actorTransform = targetActor->GetComponent<TransformComponent>();

		Vec3 dir = VMath::normalize(end - start);

		//skip to next if no transform
		if (actorTransform == nullptr) { continue; }

		//if actor's origin isn't within a 30 degrees cone to the camera we can skip to make less expensive by assuming it's probably not intersecting (May have to be increased)
		//if (!Raycast::isInRayCone(actorTransform->GetPosition(), start, dir, 0.8660254f)) { continue; }

		//	std::cout << targetActor->getActorName() << std::endl;

		Vec3 intersectSpot;
		if (targetActor->GetIntersectTriangles(start, dir, &intersectSpot)) {

			float actorDistance = VMath::distance(start, intersectSpot);
			std::cout << targetActor->getActorName() << " : " << actorDistance << std::endl;
			if (actorDistance < minDistance || minDistance == 0) {
				minDistance = actorDistance;
				closestSelected = targetActor;
			}
		}
	}

	if (closestSelected) {
		return closestSelected;
	}


	*/
	return nullptr;
}

Ref<Actor> SceneGraph::GetActor(const std::string& actorName) const
{
	// try to find the actor by name
	auto nameIt = ActorNameToId.find(actorName);
	if (nameIt == ActorNameToId.end()) {
		Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
		return nullptr;
	}

	// try to find actor by ID
	uint32_t actorId = nameIt->second;
	auto actorIt = Actors.find(actorId);

	if (actorIt != Actors.end()) {
		return actorIt->second;
	}

	// if the actor can't be found by name or by ID
	Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
	return nullptr;
}

std::vector<std::string> SceneGraph::GetAllActorNames() const
{
	std::vector<std::string> allActorNames;
	allActorNames.reserve(Actors.size());

	for (const auto& pair : Actors) {
		allActorNames.push_back(pair.second->getActorName());
	}

	return allActorNames;
}

bool SceneGraph::RemoveActor(const std::string& actorName)
{
	auto nameIt = ActorNameToId.find(actorName);
	if (nameIt == ActorNameToId.end()) {
		Debug::Warning("Actor: " + actorName + " does not exist!", __FILE__, __LINE__);
		return false;
	}

	uint32_t actorId = nameIt->second;
	auto actorIt = Actors.find(actorId);

	if (actorIt == Actors.end()) {
		Debug::Error("Actor: " + actorName + " ID does not exist!", __FILE__, __LINE__);
		return false;
	}

	Ref<Actor> actorToRemove = actorIt->second;
	actorToRemove->DeleteComponent<CameraComponent>();

	// if the actor that is being removed is parented or a parent, get all children
	std::vector<std::string> childrenToRemove;
	for (const auto& pair : Actors) {
		if (pair.second->getParentActor() == actorToRemove.get()) {
			childrenToRemove.push_back(pair.second->getActorName());
		}
	}

	// recursivly remove each child actor
	for (const std::string& childName : childrenToRemove) {
		RemoveActor(childName);
	}

	// also remove the actor from the debug
	debugSelectedAssets.erase(actorId);

	actorToRemove->OnDestroy();

	// remove from both maps
	Actors.erase(actorId);
	ActorNameToId.erase(actorName);

	return true;
}

void SceneGraph::ListAllActors() const
{
	std::cout << "All actors in the scene: " << std::endl;
	for (const auto& pair : Actors) {
		std::cout << pair.second->getActorName() << " (ID: " << pair.first << ")" << std::endl;
	}
}

void SceneGraph::RemoveAllActors()
{
	std::cout << "Deleting All Actors In The Scene" << std::endl;

	// call the OnDestroy for each actor 
	for (auto& pair : Actors) {
		if (pair.second) {
			pair.second->OnDestroy();
		}
	}

	// clear the maps
	Actors.clear();
	ActorNameToId.clear();
	debugSelectedAssets.clear();
}
static double testval = 0.0;

void SceneGraph::Update(const float deltaTime)
{
	processMainThreadTasks();


	AnimationClip::updateClipTimes(deltaTime);
	//Load any models that the worker thread finishes loading through assimp
	//storeInitializedMeshData();

	getUsedCamera()->fixCameraToTransform();

	//	std::cout << usedCamera << std::endl;


	//ScriptService::callActorScripts(GetActor("Cube"), deltaTime);
	ScriptService::updateAllScripts(deltaTime);

	for (auto& pair : Actors) {




		// get the second value from the pair (actor)
		Ref<Actor> actor = pair.second;
		// get the physics component from the actor
		Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();
		// if the actor has a physicis component, call the update
		if (PC) {
			PC->UpdateP(deltaTime, actor);
		}
	}
	/*if (!(usedCamera == nullptr)) {
		setUsedCamera();
	}*/
}

void SceneGraph::createFBOPicking(int w, int h)
{
	//create depth buffer
	glGenRenderbuffers(1, &pickingDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, pickingDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//create picking buffer
	glGenFramebuffers(1, &pickingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

	//create texture buffer
	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

	// magic sauce :>
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT, // 2. attachment point
		GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
		pickingDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
#ifdef _DEBUG
		std::cerr << "Framebuffer is not complete!" << std::endl;
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneGraph::createDockingFBO(int w, int h)
{
	//create depth buffer
	glGenRenderbuffers(1, &dockingDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, dockingDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//create picking buffer
	glGenFramebuffers(1, &dockingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, dockingFBO);

	//create texture buffer
	glGenTextures(1, &dockingTexture);
	glBindTexture(GL_TEXTURE_2D, dockingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dockingTexture, 0);

	// magic sauce :>
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT, // 2. attachment point
		GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
		dockingDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
#ifdef _DEBUG
		std::cerr << "Framebuffer is not complete!" << std::endl;
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Ref<Actor> SceneGraph::pickColour(int mouseX, int mouseY) {


	//Width of SDL Window
	int w, h;
	w = SCENEWIDTH;
	h = SCENEHEIGHT;

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = (GLsizei)windowSize.x;
		ySize = (GLsizei)(windowSize.x / aspectRatio);
	}
	else
	{
		ySize = (GLsizei)windowSize.y;
		xSize = (GLsizei)(windowSize.y * aspectRatio);
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += (GLint)imagePos.x;
	yPos += (GLint)imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
	glViewport(xPos, glY, xSize, ySize);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	//get the special shader for picking and set its uniforms
	glUseProgram(pickerShader->GetProgram());

	glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

	for (auto& actor : Actors) {

		if (!actor.second->GetComponent<MeshComponent>()) { continue; }//no meshcomponent for actor

		//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
		glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor.second->GetModelMatrix(getUsedCamera()));

		//encode the id of the actor as rgb
		Vec3 idColor = Actor::encodeID(actor.second->getId());

		//send over the rbg to the shader to use as its rendered colour
		glUniform3fv(pickerShader->GetUniformID("uIDColor"), 1, &idColor.x);

		glBindTexture(GL_TEXTURE_2D, 0);
		actor.second->GetComponent<MeshComponent>()->Render();
	}

	//rgb pixel data
	unsigned char pixel[3];

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	//get the mouse click's rgb pixel data
	glReadPixels(mouseX, h - mouseY, 1, 1,
		GL_RGB, GL_UNSIGNED_BYTE, pixel);

	//reverse selected pixel's rgb and decode into an id
	uint32_t pickedID = Actor::decodeID(pixel[0], pixel[1], pixel[2]);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);

	//check if the clicked pixel's colour id is the same as any of the actors
	for (auto& actor : Actors) {
		if (actor.second->getId() == pickedID) return actor.second;
	}

	return nullptr; // nothing clicked
}



void SceneGraph::Render() const
{
	
	int w, h;
	//SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
	w = SCENEWIDTH;
	h = SCENEHEIGHT;

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = (GLsizei)windowSize.x;
		ySize = (GLsizei)(windowSize.x / aspectRatio);
	}
	else
	{
		ySize = (GLsizei)windowSize.y;
		xSize = (GLsizei)(windowSize.y * aspectRatio);
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += (GLint)imagePos.x;
	yPos += (GLint)imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
	glViewport(xPos, glY, xSize, ySize);

	if (!RENDERMAINSCREEN) {
		Vec4 clearColour = Vec4(0.1f, 0.1f, 0.15f, 0.0f);


		std::vector<Vec3> lightPos;
		std::vector<Vec4> lightSpec;
		std::vector<Vec4> lightDiff;
		std::vector<float> lightIntensity;
		std::vector<GLuint> lightTypes;
		if (!lightActors.empty()) {
			for (auto& light : lightActors) {
				if (light->GetComponent<LightComponent>()->getType() == LightType::Point) {
					// you shouldn't have to pass a corrected light position into the shader, it should correct itself.
					lightPos.push_back(light->GetComponent<TransformComponent>()->GetPosition());// - usedCamera->GetUserActor()->GetComponent<TransformComponent>()->GetPosition());
					lightTypes.push_back(1u);
				}
				else {

					lightPos.push_back(light->GetComponent<TransformComponent>()->GetForward());
					lightTypes.push_back(0u);
				}
				lightSpec.push_back(light->GetComponent<LightComponent>()->getSpec());
				lightDiff.push_back(light->GetComponent<LightComponent>()->getDiff());
				lightIntensity.push_back(light->GetComponent<LightComponent>()->getIntensity());
			}

			glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("lightPos[0]"), lightActors.size(), lightPos[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("diffuse[0]"), lightActors.size(), lightDiff[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("specular[0]"), lightActors.size(), lightSpec[0]);
			glUniform1fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("intensity[0]"), lightActors.size(), lightIntensity.data());
			glUniform1uiv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("lightType[0]"), lightActors.size(), lightTypes.data());
			
			glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());

			glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("lightPos[0]"), lightActors.size(), lightPos[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("diffuse[0]"), lightActors.size(), lightDiff[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("specular[0]"), lightActors.size(), lightSpec[0]);
			glUniform1fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("intensity[0]"), lightActors.size(), lightIntensity.data());
			glUniform1uiv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("lightType[0]"), lightActors.size(), lightTypes.data());

		}
		glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetProgram());

		glUniform1ui(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("numLights"), lightActors.size());

		// Without an Ambient the light components won't work at all
		glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("ambient"), 1, clearColour);


		glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());

		glUniform1ui(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("numLights"), lightActors.size());

		glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("ambient"), 1, clearColour);

		//use the picking buffer so it is seperated
		glBindFramebuffer(GL_FRAMEBUFFER, dockingFBO);
		glViewport(0, 0, SCENEWIDTH, SCENEHEIGHT);
		glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//glDisable(GL_BLEND);

	// go through all actors
	for (const auto& pair : Actors) {

		Ref<Actor> actor = pair.second;

		// getting the shader, mesh, and mat for each indivual actor, using mainly for the if statement to check if the actor has each of these components
		Ref<ShaderComponent> shader = actor->GetComponent<ShaderComponent>();

		

		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
		if (actor->GetComponent<AnimatorComponent>() && mesh->skeleton &&
			actor->GetComponent<AnimatorComponent>()->activeClip.getActiveState()) {
			shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated");

		}
		if (shader) glUseProgram(shader->GetProgram());

		Ref<MaterialComponent> material = actor->GetComponent<MaterialComponent>();

		if (!shader || !mesh || !material) { continue; }

		glUniformMatrix4fv(shader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
		glUniformMatrix4fv(shader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

		// if the actor has a shader, mesh, and mat component then render it
		if (shader && mesh && material) {


			Matrix4 modelMatrix = actor->GetModelMatrix();

			


			glEnable(GL_DEPTH_TEST);

			glPolygonMode(GL_FRONT_AND_BACK, drawMode);

			bool isSelected = !debugSelectedAssets.empty() && debugSelectedAssets.find(actor->getId()) != debugSelectedAssets.end();

			if (isSelected) {
				glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
				glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);
			}
			else {
				if (pair.second->GetComponent<ShaderComponent>()) {
					glUseProgram(shader->GetProgram());

					if (pair.second->GetComponent<ShaderComponent>() == AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")) {
						// use the new material component elements from the struct
						glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);
					}
					else {
						glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);
					}
				}
				


				//				glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());

			}



			if (actor->GetComponent<AnimatorComponent>() && mesh->skeleton &&
				actor->GetComponent<AnimatorComponent>()->activeClip.getActiveState()) {
				glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());

				

				Ref<AnimatorComponent> animComp = actor->GetComponent<AnimatorComponent>();

				double timeInTicks = animComp->activeClip.getCurrentTimeInFrames();

				std::vector<Matrix4> finalBoneMatrices(mesh->skeleton->bones.size(), Matrix4());
				animComp->activeClip.animation->calculatePose(timeInTicks, mesh->skeleton.get(), finalBoneMatrices);


				
				GLint loc = shader->GetUniformID("bone_transforms[0]");
				if (loc == -1) {
					printf("ERROR: bone_transforms uniform not found!\n");
				}
				else {

					glUniformMatrix4fv(loc, (GLsizei)finalBoneMatrices.size(), GL_FALSE,
						reinterpret_cast<const float*>(finalBoneMatrices.data()));
				}

			

			}

			
			///glUseProgram(pickerShader->GetProgram());

			//Vec3 idColor = Actor::encodeID(actor->id);

			glUniform1i(shader->GetUniformID("diffuseTexture"), 0);
			glUniform1i(shader->GetUniformID("specularTexture"), 1);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material->getDiffuseID());
			if (material->getSpecularID() != 0) {
				glUniform1i(shader->GetUniformID("hasSpec"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, material->getSpecularID());
			}
			else {
				glUniform1i(shader->GetUniformID("hasSpec"), 0);
			}
			
			
			
			mesh->Render(GL_TRIANGLES);
			glBindTexture(GL_TEXTURE_2D, 0);



		}

	}

	if (!RENDERMAINSCREEN) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
	}

}

bool SceneGraph::OnCreate()
{
	// if an actor was setup wrong throw an error
	for (auto& actor : Actors) {
		if (!actor.second->OnCreate()) {
			Debug::Error("Actor failed to initialize: " + actor.second->getActorName(), __FILE__, __LINE__);
			return false;
		}



	}


	return true;
}
