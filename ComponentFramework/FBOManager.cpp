#include "pch.h"
#include "FBOManager.h"

FBOData& FBOManager::CreateFBO(FBO fbo_, int w, int h)
{
	// if the fbo is already in the map, and its already created, call resize just incase
	if (fbos.count(fbo_) && fbos[fbo_].isCreated) {
		OnResize(fbo_, w, h);
		return fbos[fbo_];
	}
	
	// otherwise, create the fbo and pass its data
	FBOData data;
	createFBO(data, w, h);
	fbos[fbo_] = data;
	return fbos[fbo_];
}

void FBOManager::OnDestroy() {
	for (auto& [fbo, data] : fbos) {
		destroyFBO(data);
	}
	fbos.clear();
}

void FBOManager::OnResize(FBO fbo_, int w, int h)
{
	auto it = fbos.find(fbo_);
	if (it == fbos.end()) return;

	FBOData& data = it->second;
	if (data.width == w && data.height == h) return;

	// destroy the old fbo and recreate it with the new width and height
	destroyFBO(data);
	createFBO(data, w, h);
}

bool FBOManager::isCreated(FBO fbo_) const
{
	auto it = fbos.find(fbo_);
	return it != fbos.end() && it->second.isCreated;
}

void FBOManager::DestroyFBO(FBO fbo_)
{
	auto it = fbos.find(fbo_);
	if (it == fbos.end()) return;
	destroyFBO(it->second);
	fbos.erase(it);
}

FBOData& FBOManager::getFBO(FBO fbo_)
{
	auto it = fbos.find(fbo_);
	// if fbo can't be found then just return an empty fbo
	if (it == fbos.end() || !it->second.isCreated) {
		static FBOData data;
		return data;
	}
	return it->second;
}

void FBOManager::createFBO(FBOData& data, int w, int h)
{
	data.width = w;
	data.height = h;

	//create depth buffer
	glGenRenderbuffers(1, &data.depth);
	glBindRenderbuffer(GL_RENDERBUFFER, data.depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//create frame buffer
	glGenFramebuffers(1, &data.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, data.fbo);

	//create texture buffer
	glGenTextures(1, &data.texture);
	glBindTexture(GL_TEXTURE_2D, data.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.texture, 0);

	// magic sauce :>
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT, // 2. attachment point
		GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
		data.depth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		data.isCreated = false;

#ifdef _DEBUG
		std::cerr << "Framebuffer is not complete!" << std::endl;
#endif

	}
	else {
		data.isCreated = true;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FBOManager::destroyFBO(FBOData& data)
{
	if (data.fbo) {
		glDeleteFramebuffers(1, &data.fbo);
		data.fbo = 0;
	}
	if (data.texture) {
		glDeleteTextures(1, &data.texture);
		data.texture = 0;
	}
	if (data.depth) {
		glDeleteRenderbuffers(1, &data.depth);
		data.depth = 0;
	}

	data.isCreated = false;
	data.width = 0;
	data.height = 0;
}