#include "pch.h"
#include "MaterialComponent.h"

MaterialComponent::MaterialComponent(Component* parent_,const char* diffuse_, const char* specular_):
	Component(parent_), diffuseID(0), specularID(0), diffuse(diffuse_), specular(specular_) {}

MaterialComponent::~MaterialComponent() {
	glDeleteTextures(1, &diffuseID);
	glDeleteTextures(1, &specularID);
}

bool MaterialComponent::OnCreate() {
	if (isCreated == true) { 
		return true; 
	}
	else if (!specular.empty()) {
		if (LoadImage(diffuse.c_str(), false) && LoadImage(specular.c_str(), true)) {
			isCreated = true;
			return true;
		}
	}
	isCreated = true;

	return LoadImage(diffuse.c_str(), false);
}

bool MaterialComponent::LoadImage(const char* filename, bool isSpec) {
	if (isSpec == false) {
		glGenTextures(1, &diffuseID);
		glBindTexture(GL_TEXTURE_2D, diffuseID);
		SDL_Surface* textureSurface = IMG_Load(filename);
		if (textureSurface == nullptr) {
			return false;
		}
		int mode = (textureSurface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, mode, textureSurface->w, textureSurface->h, 0, mode, GL_UNSIGNED_BYTE, textureSurface->pixels);

		SDL_FreeSurface(textureSurface);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		/// Wrapping and filtering options 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		

		glBindTexture(GL_TEXTURE_2D, 0); /// Unbind the texture
		return true;
	} else {
		glGenTextures(1, &specularID);
		glBindTexture(GL_TEXTURE_2D, specularID);
		SDL_Surface* textureSurface = IMG_Load(filename);
		if (textureSurface == nullptr) {
			return false;
		}
		int mode = (textureSurface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, mode, textureSurface->w, textureSurface->h, 0, mode, GL_UNSIGNED_BYTE, textureSurface->pixels);

		SDL_FreeSurface(textureSurface);
		/// Wrapping and filtering options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0); /// Unbind the texture
		return true;
	}
}

void MaterialComponent::OnDestroy() {}
void MaterialComponent::Update(const float deltaTime) {}
void MaterialComponent::Render()const {}
