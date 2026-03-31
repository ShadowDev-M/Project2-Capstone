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
	// fallback components
	fallbackMaterial = std::make_shared<MaterialComponent>(nullptr, "", "", "");
	{
		// manually creating a texture
		GLuint texID = 0;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		unsigned char magenta[4] = { 255, 0, 255, 255 }; // RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, magenta);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		fallbackMaterial->InjectDiffuseID(texID);
		fallbackMaterial->ForceCreated();
	}
	fallbackShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/fallbackVert.glsl", "Shaders/fallbackFrag.glsl");

	pickerShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/colourPickVert.glsl", "Shaders/colourPickFrag.glsl");
	outlineShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/MultiPhongVert.glsl", "Shaders/outline.glsl");
	animatedShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/animationVert.glsl", "Shaders/MultiPhongFrag.glsl");
	animatedOutlineShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/animationVert.glsl", "Shaders/outline.glsl");
	multiPhongShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/MultiPhongVert.glsl", "Shaders/MultiPhongFrag.glsl");
	shadowShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/ShadowMappingVert.glsl", "Shaders/ShadowMappingFrag.glsl");
	shadowPointShader = std::make_shared<ShaderComponent>(nullptr, "Shaders/ShadowMappingPointVert.glsl", "Shaders/ShadowMappingPointFrag.glsl", nullptr, nullptr, "Shaders/ShadowMappingPointGeom.glsl");

	shaders = { pickerShader, outlineShader, animatedShader, animatedOutlineShader, multiPhongShader, shadowShader, shadowPointShader, fallbackShader };

	for (auto& shader : shaders) {
		if (!shader->OnCreate()) {
			return false;
		}
	}

	return true;
}

void Renderer::OnDestroy() {
	for (auto& shader : shaders) {
		shader->OnDestroy();
		shader = nullptr;
	}
}

// sort of merged ShadowPass and CalculateLightSpaceMatrix into one function
void Renderer::ShadowPass()
{
	info = ShadowInfo{};

	const auto& lightActors = LightingSystem::getInstance().GetLightActors();
	info.shadowCasters.assign(lightActors.size(), -1);

	if (lightActors.empty()) return;

	int pointLightCount = 0;

	for (size_t i = 0; i < lightActors.size(); i++) {
		const auto& light = lightActors[i];
		Ref<LightComponent> LC = light->GetComponent<LightComponent>();
		if (!LC || !LC->castsShadows()) continue;

		LightType type = LC->getType();

		// Skylight
		if (type == LightType::Sky && !info.hasSky) {
			info.hasSky = true;
			info.shadowCasters[i] = -2;

			Ref<TransformComponent> TC = light->GetComponent<TransformComponent>();
			if (!TC) continue;

			//Vec3 lightDir = TC->GetForward();
			//Vec3 lightPos = -(lightDir * (LC->getShadowFar() * 0.5f));
			//float orthoSize = LC->getShadowOrthoSize();

			//Matrix4 lightProjection = MMath::orthographic(
			//	-orthoSize, orthoSize,
			//	-orthoSize, orthoSize,
			//	LC->getShadowNear(), LC->getShadowFar());

			//Matrix4 lightView = MMath::lookAt(lightPos, lightPos + lightDir, Vec3(0.0f, 1.0f, 0.0f));

			//info.SkyMatrix = lightProjection * lightView;
			//info.LightDir = lightDir;
			
			// possibly find a way to make it work for both cameras (editor and main)
			// its not a big deal though we could just use the main camera for now, 
			// was trying to get something working where you didn't need to pass a camera but it never looked quite right
			Vec3 cameraWorldPos;
			Ref<Actor> mainCam = SceneGraph::getInstance().GetMainCamera();
			if (mainCam) {
				cameraWorldPos = Vec3(mainCam->GetModelMatrix().getColumn(Matrix4::Colunm::three));
			}
			Quaternion orientation = TC->GetOrientation();
			
			// CalculateLightSpaceMatrix
			//Rotation for the 'sun' position in the sky
			Matrix4 cameraWorldTransform = MMath::toMatrix4(orientation);
			
			//Orbit's the view like the sun around the camera (kinda similar to how a skybox works)	
			Matrix4 lightView =
				MMath::translate(Vec3(0.0f, 0.0f, -100.0f)) *
				MMath::inverse(cameraWorldTransform) *
				MMath::translate(-cameraWorldPos);

			float orthoSize = LC->getShadowOrthoSize();

			Matrix4 lightProjection = MMath::orthographic(
				-orthoSize, orthoSize,
				-orthoSize, orthoSize,
				LC->getShadowNear(), LC->getShadowFar());

			//needs -z for some reason unless im doing this wrong ¯\_()_/¯
			Vec3 lightDirFromMatrix = Vec3(lightView[8], lightView[9], -lightView[10]);

			info.SkyMatrix = lightProjection * lightView;
			info.LightDir = lightDirFromMatrix;

			// ShadowPass
			int resolution = LC->getShadowResolution();
			FBOManager::getInstance().OnResize(FBO::ShadowMap, resolution, resolution);
			FBOData& fbo = FBOManager::getInstance().getFBO(FBO::ShadowMap);
			if (!fbo.isCreated) continue;

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
			glViewport(0, 0, fbo.width, fbo.height);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(shadowShader->GetProgram());
			glUniformMatrix4fv(shadowShader->GetUniformID("lightSpaceMatrix"), 1, GL_FALSE, info.SkyMatrix);

			for (const auto& [id, actor] : SceneGraph::getInstance().getAllActors()) {
				Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
				if (!mesh || !mesh->queryLoadStatus()) continue;
				Ref<ShadowSettings> settings = actor->GetComponent<ShadowSettings>();
				if (settings && !settings->getCastShadow()) continue;
				
				bool isAnimating = mesh->hasSkeleton() &&
					actor->GetComponent<AnimatorComponent>() &&
					actor->GetComponent<AnimatorComponent>()->getAnimationClip().getActiveState();
				
				SetUniformShadowPass(shadowShader, actor, isAnimating);
				mesh->Render();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glCullFace(GL_BACK);
		}
		else if (type == LightType::Point && pointLightCount < MAX_SHADOW_CASTERS) {
			info.shadowCasters[i] = pointLightCount;
			info.pointFarPlanes[pointLightCount] = LC->getShadowFar();

			// CalculateLightSpaceMatrix
			Vec3 lightPos = Vec3(light->GetModelMatrix().getColumn(Matrix4::Colunm::three));
			Vec3 lightPosY = Vec3(lightPos.x, -lightPos.y, -lightPos.z);
			float near = LC->getShadowNear();
			float far = LC->getShadowFar();

			Matrix4 lightProjection = MMath::perspective(90.0f, 1.0f, near, far);

			std::vector<Matrix4> faceViews(6);
			faceViews[0] = lightProjection * MMath::lookAt(lightPos, lightPos + Vec3(1, 0, 0), Vec3(0, -1, 0)); // +X
			faceViews[1] = lightProjection * MMath::lookAt(lightPos, lightPos + Vec3(-1, 0, 0), Vec3(0, -1, 0)); // -X
			faceViews[2] = lightProjection * MMath::lookAt(lightPosY, lightPosY + Vec3(0, -1, 0), Vec3(0, 0, -1)); // +Y
			faceViews[3] = lightProjection * MMath::lookAt(lightPosY, lightPosY + Vec3(0, 1, 0), Vec3(0, 0, 1)); // -Y
			faceViews[4] = lightProjection * MMath::lookAt(lightPos, lightPos + Vec3(0, 0, 1), Vec3(0, -1, 0)); // +Z
			faceViews[5] = lightProjection * MMath::lookAt(lightPos, lightPos + Vec3(0, 0, -1), Vec3(0, -1, 0)); // -Z

			// ShadowPass
			int resolution = LC->getShadowResolution();
			//each pointlight gets its own cubemap buffer (you could do it in one but the expensive part isn't the buffer but the content so its easier to do with existing framework)
			static const FBO CubeMapFBO[4] = {
				FBO::ShadowCubeMap, FBO::ShadowCubeMap1,
				FBO::ShadowCubeMap2, FBO::ShadowCubeMap3
			};
			FBOManager::getInstance().OnResize(CubeMapFBO[pointLightCount], resolution, resolution);
			FBOData& fbo = FBOManager::getInstance().getFBO(CubeMapFBO[pointLightCount]);
			if (!fbo.isCreated) { pointLightCount++; continue; }

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glCullFace(GL_BACK); // normal culling, not front
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(3.0f, 6.0f);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
			glViewport(0, 0, fbo.width, fbo.height);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(shadowPointShader->GetProgram());
			glUniformMatrix4fv(shadowPointShader->GetUniformID("shadowMatrices[0]"), 6, GL_FALSE, reinterpret_cast<const float*>(faceViews.data()));
			glUniform3fv(shadowPointShader->GetUniformID("lightPos"), 1, &lightPos.x);
			glUniform1f(shadowPointShader->GetUniformID("farPlane"), far);

			for (const auto& [id, actor] : SceneGraph::getInstance().getAllActors()) {
				Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();
				if (!mesh || !mesh->queryLoadStatus()) continue;
				Ref<ShadowSettings> settings = actor->GetComponent<ShadowSettings>();
				if (settings && !settings->getCastShadow()) continue;

				bool isAnimating = mesh->hasSkeleton() &&
					actor->GetComponent<AnimatorComponent>() &&
					actor->GetComponent<AnimatorComponent>()->getAnimationClip().getActiveState();

				SetUniformShadowPass(shadowPointShader, actor, isAnimating);
				mesh->Render();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glCullFace(GL_BACK);

			pointLightCount++;
		}
	}
}

void Renderer::RenderSceneView()
{
	FBOData& fbo = FBOManager::getInstance().getFBO(FBO::Scene);
	if (!fbo.isCreated) return;

	// SceneWindow uses editor camera
	auto& editorCamera = EditorManager::getInstance().getEditorCamera();

	RenderContext ctx;
	ctx.view = editorCamera.GetViewMatrix();
	ctx.projection = editorCamera.GetProjectionMatrix();
	ctx.cameraPos = editorCamera.GetPosition();
	ctx.isGameView = false;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glViewport(0, 0, fbo.width, fbo.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, drawMode);

	UploadLightUniforms(ctx);

	// What should get rendered to the fbo
	RenderActors(ctx);
	if (showSceneGizmos) RenderColliderDebug(ctx);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	RenderContext ctx;
	ctx.view = cam->GetViewMatrix();
	ctx.projection = cam->GetProjectionMatrix();
	ctx.cameraPos = cam->getWorldPosition();
	ctx.isGameView = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glViewport(0, 0, fbo.width, fbo.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	UploadLightUniforms(ctx);

	// What should get rendered to the fbo
	RenderActors(ctx);
	if (showGameGizmos) RenderColliderDebug(ctx);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::UploadLightUniforms(const RenderContext& ctx)
{
	LightingSystem& ls = LightingSystem::getInstance();

	// sort of a small function, but if there are ever anymore shaders that need lighting upload to them,
	// then its easier just to have this as a maintainer for all of them
	Ref<ShaderComponent> litShaders[] = { multiPhongShader, animatedShader };
	for (auto& shader : litShaders) {
		if (!shader) continue;
		ls.UploadUniforms(shader->GetProgram(), info.shadowCasters);
		glUseProgram(shader->GetProgram());
		glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, &ctx.cameraPos.x);
	}
}

void Renderer::UploadTilingUniforms(Ref<ShaderComponent> shader, Ref<Actor> actor) const
{
	glUseProgram(shader->GetProgram());

	Ref<TilingSettings> TS = actor->GetComponent<TilingSettings>();
	bool isTiled = TS && TS->getIsTiled();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "isTiled"), isTiled ? 1 : 0);
	if (isTiled) {
		Vec2 scale = TS->getTileScale();
		Vec2 offset = TS->getTileOffset();
		glUniform2f(glGetUniformLocation(shader->GetProgram(), "tileScale"), scale.x, scale.y);
		glUniform2f(glGetUniformLocation(shader->GetProgram(), "tileOffset"), offset.x, offset.y);
		glUniform3f(glGetUniformLocation(shader->GetProgram(), "uvTiling"), 1.0f, 1.0f, 1.0f); // TODO: if we expose it later
	}
	else {
		glUniform2f(glGetUniformLocation(shader->GetProgram(), "tileScale"), 1.0f, 1.0f);
		glUniform2f(glGetUniformLocation(shader->GetProgram(), "tileOffset"), 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(shader->GetProgram(), "uvTiling"), 1.0f, 1.0f, 1.0f);
	}
}

void Renderer::UploadShadowUniforms(Ref<ShaderComponent> shader) const
{
	glUseProgram(shader->GetProgram());

	// Skylight
	FBOData& shadowFBO = FBOManager::getInstance().getFBO(FBO::ShadowMap);
	if (shadowFBO.isCreated && info.hasSky) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowFBO.texture);
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "shadowMap"), 3);
		glUniform3fv(glGetUniformLocation(shader->GetProgram(), "shadowLightDir"), 1, &info.LightDir.x);
	}

	static const FBO CubeMapFBO[MAX_SHADOW_CASTERS] = {
		FBO::ShadowCubeMap, FBO::ShadowCubeMap1,
		FBO::ShadowCubeMap2, FBO::ShadowCubeMap3
	};
	for (int i = 0; i < MAX_SHADOW_CASTERS; i++) {
		FBOData& cubeFBO = FBOManager::getInstance().getFBO(CubeMapFBO[i]);
		if (!cubeFBO.isCreated) continue;

		glActiveTexture(GL_TEXTURE4 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeFBO.texture);
		std::string name = "pointShadowMaps[" + std::to_string(i) + "]";
		glUniform1i(glGetUniformLocation(shader->GetProgram(), name.c_str()), 4 + i);
	}

	for (int i = 0; i < MAX_SHADOW_CASTERS; i++) {
		std::string name = "pointShadowFarPlanes[" + std::to_string(i) + "]";
		glUniform1f(glGetUniformLocation(shader->GetProgram(), name.c_str()), info.pointFarPlanes[i]);
	}
}

void Renderer::SetUniformShadowPass(Ref<ShaderComponent> shader, Ref<Actor> actor, bool isAnim) const
{
	glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
	glUniform1i(shader->GetUniformID("isAnimated"), isAnim ? 1 : 0);

	// Calculating Bonematrices
	if (isAnim) {
		Ref<AnimatorComponent> animComp = actor->GetComponent<AnimatorComponent>();
		double timeInTicks = animComp->getAnimationClip().getCurrentTimeInFrames();
		
		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();

		std::vector<Matrix4> finalBoneMatrices(mesh->getBoneCount(), Matrix4());
		animComp->getAnimationClip().getAnim()->calculatePose(timeInTicks, mesh->getSkeleton(), finalBoneMatrices);

		GLint loc = shader->GetUniformID("bone_transforms[0]");
		if (loc != -1) {
			glUniformMatrix4fv(loc, (GLsizei)finalBoneMatrices.size(), GL_FALSE,
				reinterpret_cast<const float*>(finalBoneMatrices.data()));
		}
	}
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
		glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
		glUniform3fv(shader->GetUniformID("cameraPos"), 1, &ctx.cameraPos.x);
		
		// Uploading Light, Tiling, and Shader
		if (info.hasSky) {
			glUniformMatrix4fv(shader->GetUniformID("lightSpaceMatrix"), 1, GL_FALSE, info.SkyMatrix);
		}
		UploadTilingUniforms(shader, actor);
		UploadShadowUniforms(shader);

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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material->getDiffuseID());
		glUniform1i(shader->GetUniformID("diffuseTexture"), 0);
		
		bool hasSpec = material->getSpecularID() != 0;
		bool hasNorm = material->getNormalID() != 0;
		glUniform1i(shader->GetUniformID("hasSpec"), hasSpec ? 1 : 0);
		glUniform1i(shader->GetUniformID("hasNorm"), hasNorm ? 1 : 0);

		if (hasSpec) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material->getSpecularID());
			glUniform1i(shader->GetUniformID("specularTexture"), 1);
		}
		if (hasNorm) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material->getNormalID());
			glUniform1i(shader->GetUniformID("normalTexture"), 2);
		}

		mesh->Render(GL_TRIANGLES);
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