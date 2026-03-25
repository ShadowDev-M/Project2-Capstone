#include "pch.h"
#include "Renderer.h"
#include "Actor.h"
#include "ShaderComponent.h"
#include "LightingSystem.h"
#include "SceneGraph.h"
#include "CollisionComponent.h"
#include "ColliderDebug.h"
#include "FBOManager.h"
#include "ScreenManager.h"
#include "EditorManager.h"
#include "InputManager.h"

bool Renderer::OnCreate() {
	pickerShader = std::make_shared<ShaderComponent>(nullptr, "shaders/colourPickVert.glsl", "shaders/colourPickFrag.glsl");
	outlineShader = std::make_shared<ShaderComponent>(nullptr, "shaders/MultiPhongVert.glsl", "shaders/outline.glsl");
	animatedShader = std::make_shared<ShaderComponent>(nullptr, "shaders/animationVert.glsl", "shaders/MultiPhongFrag.glsl");
	animatedOutlineShader = std::make_shared<ShaderComponent>(nullptr, "shaders/animationVert.glsl", "shaders/outline.glsl");
	multiPhongShader = std::make_shared<ShaderComponent>(nullptr, "shaders/MultiPhongVert.glsl", "shaders/MultiPhongFrag.glsl");
	
	if (!pickerShader->OnCreate()) return false;
	if (!outlineShader->OnCreate()) return false;
	if (!animatedShader->OnCreate()) return false;
	if (!animatedOutlineShader->OnCreate()) return false;
	if (!multiPhongShader->OnCreate()) return false;

	return true;
}

void Renderer::OnDestroy() {
	if (pickerShader) { pickerShader->OnDestroy(); pickerShader = nullptr; }
	if (outlineShader) { outlineShader->OnDestroy(); outlineShader = nullptr; }
	if (animatedShader) { animatedShader->OnDestroy(); animatedShader = nullptr; }
	if (animatedOutlineShader) { animatedOutlineShader->OnDestroy(); animatedOutlineShader = nullptr; }
	if (multiPhongShader) { multiPhongShader->OnDestroy(); multiPhongShader = nullptr; }
}

void Renderer::RenderSceneView()
{
	FBOData& fbo = FBOManager::getInstance().getFBO(FBO::Scene);
	if (!fbo.isCreated) return;

	// SceneWindow uses editor camera
	auto& editorCamera = EditorManager::getInstance().getEditorCamera();

	UploadLightUniformsToAllShaders();

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glViewport(0, 0, fbo.width, fbo.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	RenderContext ctx;
	ctx.view = editorCamera.GetViewMatrix();
	ctx.projection = editorCamera.GetProjectionMatrix();
	ctx.cameraPos = editorCamera.GetPosition();
	ctx.isGameView = false;

	// What should get rendered to the fbo
	RenderActors(ctx);
	if (showSceneGizmos) RenderColliderDebug(ctx);

	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
}

void Renderer::RenderGameView()
{
#ifndef ENGINE_EDITOR
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	int w = ScreenManager::getInstance().getDisplayWidth();
	int h = ScreenManager::getInstance().getDisplayHeight();
	glViewport(0, 0, w, h);
	return;
#endif
	
	FBOData& fbo = FBOManager::getInstance().getFBO(FBO::Game);
	if (!fbo.isCreated) return;

	// GameWindow uses main camera
	auto mainCamera = SceneGraph::getInstance().GetMainCamera();
	if (!mainCamera) return;
	auto cam = mainCamera->GetComponent<CameraComponent>();
	if (!cam) return;

	UploadLightUniformsToAllShaders();

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glViewport(0, 0, fbo.width, fbo.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	RenderContext ctx;
	ctx.view = cam->GetViewMatrix();
	ctx.projection = cam->GetProjectionMatrix();
	ctx.cameraPos = cam->getWorldPosition();
	ctx.isGameView = true;

	// What should get rendered to the fbo
	RenderActors(ctx);
	if (showGameGizmos) RenderColliderDebug(ctx);

	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
}

void Renderer::UploadLightUniformsToAllShaders()
{
	LightingSystem& ls = LightingSystem::getInstance();

	// sort of a small function, but if there are ever anymore shaders that need lighting upload to them,
	// then its easier just to have this as a maintainer for all of them
	ls.UploadUniforms(multiPhongShader->GetProgram());
	ls.UploadUniforms(animatedShader->GetProgram());
}

void Renderer::RenderActors(const RenderContext& ctx) const
{
	SceneGraph& sg = SceneGraph::getInstance();

	for (const auto& [id, actor] : sg.getAllActors()) {
		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
		Ref<MaterialComponent> material = actor->GetComponent<MaterialComponent>();

		if (!mesh || !material) continue;
		if (!mesh->queryLoadStatus()) continue;

		bool isSelected = !ctx.isGameView && !sg.debugSelectedAssets.empty() 
			&& sg.debugSelectedAssets.find(id) != sg.debugSelectedAssets.end();

		bool isAnimating = mesh->hasSkeleton() && 
			actor->GetComponent<AnimatorComponent>() && 
			actor->GetComponent<AnimatorComponent>()->getAnimationClip().getActiveState();
		
		// Shader Selection 
		Ref<ShaderComponent> shader;
		if (isAnimating) {
			shader = isSelected ? animatedOutlineShader : animatedShader;
		}
		else {
			shader = isSelected ? outlineShader : multiPhongShader;
		}

		if (!shader) continue;

		// Uploading Shader Data 
		glUseProgram(shader->GetProgram());
		glUniformMatrix4fv(shader->GetUniformID("projectionMatrix"), 1, GL_FALSE, ctx.projection);
		glUniformMatrix4fv(shader->GetUniformID("viewMatrix"), 1, GL_FALSE, ctx.view);
		glUniform3fv(shader->GetUniformID("cameraPos"), 1, &ctx.cameraPos.x);

		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, drawMode);

		Matrix4 modelMatrix = actor->GetModelMatrix();
		glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);
		
		// Calculating Bonematrices
		if (isAnimating) {
			Ref<AnimatorComponent> animComp = actor->GetComponent<AnimatorComponent>();
			double timeInTicks = animComp->getAnimationClip().getCurrentTimeInFrames();

			std::vector<Matrix4> finalBoneMatrices(mesh->getBoneCount(), Matrix4());
			animComp->getAnimationClip().getAnim()->calculatePose(timeInTicks, mesh->getSkeleton(), finalBoneMatrices);
		
			GLint loc = shader->GetUniformID("bone_transforms[0]");
			if (loc != -1) {
				glUniformMatrix4fv(loc, (GLsizei)finalBoneMatrices.size(), GL_FALSE,
					reinterpret_cast<const float*>(finalBoneMatrices.data()));
			}
		}

		// Materials/Textures
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

void Renderer::RenderColliderDebug(const RenderContext& ctx) const
{
	SceneGraph& sg = SceneGraph::getInstance();

	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);

	for (const auto& [id, actor] : sg.debugSelectedAssets) {
		if (!actor) continue;
		Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
		Ref<CollisionComponent> CC = actor->GetComponent<CollisionComponent>();
		if (!TC || !CC) continue;
		ColliderDebug::getInstance().Render(CC, TC, ctx.view, ctx.projection);
	}

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);
}

Ref<Actor> Renderer::PickActor(int mouseX, int mouseY)
{
	if (!pickerShader) return nullptr;

	SceneGraph& sg = SceneGraph::getInstance();
	FBOData& pickingFBO = FBOManager::getInstance().getFBO(FBO::ColorPicker);

	float aspect = ScreenManager::getInstance().getRenderAspectRatio();
	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();

	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO.fbo);
	glViewport(0, 0, pickingFBO.width, pickingFBO.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	auto& editorCamera = EditorManager::getInstance().getEditorCamera();
	Matrix4 proj = editorCamera.GetProjectionMatrix();
	Matrix4 view = editorCamera.GetViewMatrix();

	//get the special shader for picking and set its uniforms
	glUseProgram(pickerShader->GetProgram());
	glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, proj);
	glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, view);

	for (auto& [id, actor] : sg.getAllActors()) {
		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
		if (!mesh || !mesh->queryLoadStatus()) continue; //no meshcomponent for actor

		//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
		glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor->GetModelMatrix());

		//encode the id of the actor as rgb
		Vec3 idColor = Actor::encodeID(actor->getId());

		//send over the rbg to the shader to use as its rendered colour
		glUniform3fv(pickerShader->GetUniformID("uIDColor"), 1, &idColor.x);
		glBindTexture(GL_TEXTURE_2D, 0);
		mesh->Render();
	}

	// scaled viewport to get uv conversion for fbo coords
	auto* mm = InputManager::getInstance().getMouseMap();
	float vpX = mm->scenePos.x, vpY = mm->scenePos.y;
	float vpW = mm->sceneSize.x, vpH = mm->sceneSize.y;

	float imgX = vpX, imgY = vpY;
	float imgW = vpW, imgH = vpH;
	GLint fboX = (GLint)(((float)mouseX - imgX) / imgW * pickingFBO.width);
	GLint fboY = (GLint)((1.0f - ((float)mouseY - imgY) / imgH) * pickingFBO.height);

	//get the mouse click's rgb pixel data
	unsigned char pixel[3];
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(fboX, fboY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);

	//reverse selected pixel's rgb and decode into an id
	uint32_t pickedID = Actor::decodeID(pixel[0], pixel[1], pixel[2]);

	return sg.GetActorById(pickedID);
}