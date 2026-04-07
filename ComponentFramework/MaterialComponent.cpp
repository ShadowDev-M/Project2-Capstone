#include "pch.h"
#include "MaterialComponent.h"

MaterialComponent::MaterialComponent(Component* parent_, const char* diffuse_, const char* specular_, const char* normal_, const char* roughness_, const char* metallic_):
	Component(parent_), diffuseID(0), specularID(0), normalID(0), roughnessID(0), metallicID(0), diffuse(diffuse_), specular(specular_), normal(normal_), roughness(roughness_), metallic(metallic_){}

MaterialComponent::~MaterialComponent() {
	glDeleteTextures(1, &diffuseID);
	glDeleteTextures(1, &specularID);
	glDeleteTextures(1, &normalID);
	glDeleteTextures(1, &roughnessID);
	glDeleteTextures(1, &metallicID);
}

bool MaterialComponent::OnCreate() {
	if (!roughness.empty()) {
		isPBR = true;
	}
	else isPBR = false;

	if (isCreated == true) {
		return true;
	}
	if (LoadImage(diffuse.c_str(), TexType::DIFFUSE)) {
		if (!normal.empty()) {
			if (!LoadImage(normal.c_str(), TexType::NORMAL)) {
				// called normal path fails to load
				isCreated = false;
				return false;
			}
		}
		if (isPBR == false) {
			if (!specular.empty()) {
				if (!LoadImage(specular.c_str(), TexType::SPECULAR)) {
					// called specular path fails to load
					isCreated = false;
					return false;
				}
			}
		}
		else {
			if (!roughness.empty()) {
				if (!LoadImage(roughness.c_str(), TexType::ROUGHNESS)) {
					// called roughness path fails to load
					isCreated = false;
					return false;
				}
			}
			if (!metallic.empty()) {
				if (!LoadImage(metallic.c_str(), TexType::METALLIC)) {
					// called metallic path fails to load
					isCreated = false;
					return false;
				}
			}
		}
		// everything loads correctly
		isCreated = true;
		return true;
	}
	// diffuse fails to return
	isCreated = false;
	return false;
}

bool MaterialComponent::LoadImage(const char* filename, TexType type) {
	GLuint* targetID = nullptr;
	if (type == DIFFUSE) targetID = &diffuseID;
	else if (type == SPECULAR) targetID = &specularID;
	else if (type == NORMAL) targetID = &normalID;
	else if (type == ROUGHNESS) targetID = &roughnessID;
	else if (type == METALLIC) targetID = &metallicID;

	// resolving path here
	fs::path resolved = SearchPath::getInstance().Resolve(filename);
	std::string pathToOpen = resolved.empty() ? filename : resolved.string();

	SDL_Surface* textureSurface = IMG_Load(pathToOpen.c_str());
	if (!textureSurface || !textureSurface->pixels) {
		return false;
	}

	int mode = SDL_PIXELFORMAT_RGBA8888;
	int format = SDL_PIXELFORMAT_RGBA8888;
	switch (textureSurface->format->BytesPerPixel) {
	case 4: 
		mode = GL_SRGB_ALPHA;
		format = GL_RGBA;
		break;
	case 3:
		mode = GL_SRGB;
		format = GL_RGB;
		break;
	case 1:
		mode = GL_SRGB;
		format = GL_LUMINANCE;
		break;
	default:
		SDL_FreeSurface(textureSurface);
		return false;
	}

	glGenTextures(1, targetID);
	glBindTexture(GL_TEXTURE_2D, *targetID);
	
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, textureSurface->w, textureSurface->h, 0, format, GL_UNSIGNED_BYTE, textureSurface->pixels);
	SDL_FreeSurface(textureSurface);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	/// Wrapping and filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); /// Unbind the texture
	return true;
}

void MaterialComponent::OnDestroy() {}
void MaterialComponent::Update(const float deltaTime) {}
void MaterialComponent::Render()const {}
