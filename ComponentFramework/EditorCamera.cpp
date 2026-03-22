#include "pch.h"
#include "EditorCamera.h"
#include "ScreenManager.h"
#include "FBOManager.h"

void EditorCamera::Update(float deltaTime, bool isHovered) {
	ImGuiIO& io = ImGui::GetIO();
	bool rmbDown = io.MouseDown[ImGuiMouseButton_Right];
	bool mmbDown = io.MouseDown[ImGuiMouseButton_Middle];
	float scroll = io.MouseWheel;

	// rmb states
	bool rmbJustPressed = rmbDown && !m_wasRMBDown;
	bool rmbJustReleased = !rmbDown && m_wasRMBDown;
	m_wasRMBDown = rmbDown;

	// mmb states
	bool mmbJustPressed = mmbDown && !m_wasMMBDown;
	bool mmbJustReleased = !mmbDown && m_wasMMBDown;
	m_wasMMBDown = mmbDown;

	// rmb state, skips first delta, saves mouse position and locks cursor
	if (rmbJustPressed && isHovered) {
		SDL_GetGlobalMouseState(&m_RMBCursorX, &m_RMBCursorY);

		if (!m_mmbActive) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}

		m_rmbActive = true;
		m_skipRMBDelta = true;
	}

	// rmb released, unlocks curosr, returns it back to original position
	if (rmbJustReleased && m_rmbActive) {
		m_rmbActive = false;

		if (!m_mmbActive) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			SDL_WarpMouseGlobal(m_RMBCursorX, m_RMBCursorY);
		}
	}

	// mmb state, skips first delta, saves mouse position and locks cursor
	if (mmbJustPressed && isHovered) {
		SDL_GetGlobalMouseState(&m_MMBCursorX, &m_MMBCursorY);

		if (!m_rmbActive) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}

		m_mmbActive = true;
		m_skipMMBDelta = true;
	}

	// mmb released, unlocks curosr, returns it back to original position
	if (mmbJustReleased && m_mmbActive) {
		m_mmbActive = false;

		if (!m_rmbActive) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			SDL_WarpMouseGlobal(m_MMBCursorX, m_MMBCursorY);
		}
	}

	// getting mouse delta
	float deltaX = 0.0f, deltaY = 0.0f;
	if (m_rmbActive || m_mmbActive) {
		int rdX = 0, rdY = 0;
		SDL_GetRelativeMouseState(&rdX, &rdY);
		deltaX = static_cast<float>(rdX);
		deltaY = static_cast<float>(rdY);
	}
	else { // fallback to imgui delta
		deltaX = io.MouseDelta.x;
		deltaY = io.MouseDelta.y;
	}

	if (m_mode == Mode::Mode3D) {
		// 3D controls
		if (m_rmbActive) {
			if (!m_skipRMBDelta) {
				HandleLook3D(deltaX, deltaY);
			}
			m_skipRMBDelta = false;

			HandleFly3D(deltaTime);

			if (scroll != 0.0f) {
				m_speed *= (1.0f + scroll * 0.15f);
				m_speed = std::clamp(m_speed, m_speedMin, m_speedMax);
				m_showSpeedPopup = true;
				m_speedPopupTimer = 1.5f;
			}
		}
		else if (isHovered && scroll != 0.0f) {
			// can go ortho view in 3D mode
			if (m_isOrtho) {
				m_orthoSize -= scroll * m_orthoSize * 0.1f;
				m_orthoSize = std::max(m_orthoSize, 0.01f);
			}
			else {
				m_position = m_position + GetForward() * (scroll * m_scrollSpeed);
			}
		}
		
		// regular panning with middile mouse
		if (m_mmbActive) {
			if (!m_skipMMBDelta) {
				HandlePan3D(deltaX, deltaY);
			}
			m_skipMMBDelta = false;
		}

		if (m_showSpeedPopup) {
			m_speedPopupTimer -= deltaTime;
			if (m_speedPopupTimer <= 0.0f) {
				m_showSpeedPopup = false;
				m_speedPopupTimer = 0.0f;
			}
		}
	}
	// 2D mode
	else {
		Enforce2DModeConstraints();

		// can only pan and zoom in/out
		bool panActive = m_rmbActive || m_mmbActive;
		if (panActive) {
			bool skip = (m_rmbActive && m_skipRMBDelta) || (m_mmbActive && m_skipMMBDelta);

			if (!skip) {
				HandlePan2D(deltaX, deltaY);
			}

			m_skipRMBDelta = false;
			m_skipMMBDelta = false;
		}

		if (isHovered && scroll != 0.0f) {
			m_orthoSize -= scroll * m_orthoSize * 0.1f;
			m_orthoSize = std::max(m_orthoSize, 0.01f);
		}
	}
}

void EditorCamera::HandlePan2D(float deltaX, float deltaY)
{
	float scale = m_orthoSize * m_panSens;
	m_position.x -= deltaX * scale;
	m_position.y += deltaY * scale;
}

void EditorCamera::HandlePan3D(float deltaX, float deltaY)
{
	// delta is relative mouse position, multiplying by scalar
	float scale = m_panSpeed * m_panSens;
	m_position = m_position - GetRight() * deltaX * scale;
	m_position = m_position + GetUp() * deltaY * scale;
}

void EditorCamera::HandleLook3D(float deltaX, float deltaY)
{
	// scaling the look by the fov and max pitch degrees,
	// that way if zoomed in or out, it stays constitent
	float fovScale = m_fov / 90.0f;
	float sens = m_lookSens * fovScale;

	m_yaw += deltaX * sens;
	m_pitch -= deltaY * sens;
	m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
	if (m_yaw >= 360.0f) m_yaw -= 360.0f;
	if (m_yaw < 0.0f) m_yaw += 360.0f;
}

void EditorCamera::HandleFly3D(float deltaTime)
{
	float speed = m_speed * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 2.0f : 1.0f) * deltaTime;
	Vec3 fwd = GetForward();
	Vec3 right = GetRight();
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

	if (ImGui::IsKeyDown(ImGuiKey_W)) m_position = m_position + fwd * speed;
	if (ImGui::IsKeyDown(ImGuiKey_A)) m_position = m_position - right * speed;
	if (ImGui::IsKeyDown(ImGuiKey_S)) m_position = m_position - fwd * speed;
	if (ImGui::IsKeyDown(ImGuiKey_D)) m_position = m_position + right * speed;
	if (ImGui::IsKeyDown(ImGuiKey_Q)) m_position = m_position - up * speed;
	if (ImGui::IsKeyDown(ImGuiKey_E)) m_position = m_position + up * speed;
}

Matrix4 EditorCamera::GetProjectionMatrix() const {
	// getting the scenes fbo so that the view is accruate
	FBOData& fbo = FBOManager::getInstance().getFBO(FBO::Scene);
	float aspectRatio = (fbo.isCreated && fbo.height > 0) ? (float)fbo.width / (float)fbo.height : ScreenManager::getInstance().getRenderAspectRatio();
	
	bool isOrtho = (m_mode == Mode::Mode2D) || m_isOrtho;
	if (isOrtho) {
		float h = m_orthoSize;
		float w = h * aspectRatio;
		return MMath::orthographic(-w, w, -h, h, m_nearClip, m_farClip);
	}
	else {
		return MMath::perspective(m_fov, aspectRatio, m_nearClip, m_farClip);
	}
}

Matrix4 EditorCamera::GetViewMatrix() const
{
	return MMath::lookAt(m_position, m_position + GetForward(), Vec3(0.0f, 1.0f, 0.0f));
}

void EditorCamera::SetOrientation(float yawDeg, float pitchDeg)
{
	m_yaw = yawDeg;
	m_pitch = std::clamp(pitchDeg, -89.0f, 89.0f);
}

void EditorCamera::FrameTarget(const Vec3& target, float distance)
{
	m_position = target - GetForward() * distance;
}

void EditorCamera::EnterMode2D(const std::vector<Vec3>& positions)
{
	m_mode = Mode::Mode2D;
	m_yaw = 0.0f; 
	m_pitch = 0.0f;

	// if no actors, just setting camera to default z
	if (positions.empty()) {
		m_position.z = 10.0f;
		return;
	}

	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;

	for (const auto& pos : positions) {
		minX = std::min(minX, pos.x); maxX = std::max(maxX, pos.x);
		minY = std::min(minY, pos.y); maxY = std::max(maxY, pos.y);
	}

	// centering camera pos
	m_position.x = (minX + maxX) * 0.5f;
	m_position.y = (minY + maxY) * 0.5f;
	
	// pushing z back so the cam can see everything
	m_position.z = 100.0f;

	// caluculating the best width and height to frame to camera
	float boundsW = std::max(maxX - minX, 1.0f);
	float boundsH = std::max(maxY - minY, 1.0f);
	float padding = 1.2f;

	// to ensure accruate framing
	FBOData& fbo = FBOManager::getInstance().getFBO(FBO::Scene);
	float aspectRatio = (fbo.isCreated && fbo.height > 0) ? (float)fbo.width / (float)fbo.height : ScreenManager::getInstance().getRenderAspectRatio();

	float fromWidth = (boundsW * 0.5f / aspectRatio) * padding;
	float fromHeight = (boundsH * 0.5f) * padding;
	m_orthoSize = std::max(fromHeight, fromWidth);
}

void EditorCamera::Enforce2DModeConstraints()
{
	m_yaw = 0.0f;
	m_pitch = 0.0f;
}

Vec3 EditorCamera::GetForward() const {
	float y = m_yaw * DEGREES_TO_RADIANS;
	float p = m_pitch * DEGREES_TO_RADIANS;

	return Vec3(std::cos(p) * std::sin(y), std::sin(p), -std::cos(p) * std::cos(y));
}

Vec3 EditorCamera::GetRight() const {
	float y = m_yaw * DEGREES_TO_RADIANS;

	return Vec3(std::cos(y), 0.0f, std::sin(y));
}

Vec3 EditorCamera::GetUp() const {
	return VMath::cross(GetRight(), GetForward());
}

void EditorCamera::ResetView()
{
	m_position = Vec3(0.0f, 0.0f, 0.0f);
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_isOrtho = false;
}

void EditorCamera::ResetSettings()
{
	m_fov = 60.0f;
	m_nearClip = 0.03f;
	m_farClip = 10000.0f;
	m_speed = 5.0f;
	m_speedMin = 0.1f;
	m_speedMax = 100.0f;
	m_orthoSize = 5.0f;
	m_mode = Mode::Mode3D;
}