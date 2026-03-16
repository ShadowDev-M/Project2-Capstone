#include "pch.h"
#include "EditorCamera.h"
#include "ScreenManager.h"

void EditorCamera::Update(float deltaTime, bool isHovered) {
	ImGuiIO& io = ImGui::GetIO();
	bool mmbDown = io.MouseDown[ImGuiMouseButton_Middle];
	ImVec2 delta = io.MouseDelta;
	float scroll = io.MouseWheel;

	if (m_mode == Mode::Mode3D) {
		// 3D controls
		if (isRMBHeld() && isHovered) {
			HandleLook3D(delta.x, delta.y);
			HandleFly3D(deltaTime);

			if (scroll != 0.0f) {
				m_speed *= (1.0f + scroll * 0.15f);
				m_speed = std::clamp(m_speed, m_speedMin, m_speedMax);
			}
		}
		else if (isHovered && scroll != 0.0f) {
			// can go ortho view in 3D mode
			if (m_isOrtho) {
				m_orthoSize -= scroll * m_orthoSize * 0.1f;
				m_orthoSize = std::max(m_orthoSize, 0.01f);
			}
			else {
				m_position = m_position + GetForward() * (scroll * m_speed);
			}
		}
		
		// regular panning with middile mouse
		if (mmbDown && isHovered) {
			HandlePan3D(delta.x, delta.y);
		}
	}
	// 2D mode
	else {
		// can only pan and zoom in/out
		if ((isRMBHeld() || mmbDown) && isHovered) {
			HandlePan2D(delta.x, delta.y);
		}

		if (isHovered && scroll != 0.0f) {
			m_orthoSize -= scroll * m_orthoSize * 0.1f;
			m_orthoSize = std::max(m_orthoSize, 0.01f);
		}
	}
}

void EditorCamera::HandlePan2D(float deltaX, float deltaY)
{
	// delta is relative mouse position, multiplying by scalar
	float scale = m_speed * m_panSens;
	m_position.x -= deltaX * scale;
	m_position.y += deltaY * scale;
}

void EditorCamera::HandlePan3D(float deltaX, float deltaY)
{
	float scale = m_speed * m_panSens;
	m_position = m_position - GetRight() * deltaX * scale;
	m_position = m_position + GetUp() * deltaY * scale;
}

void EditorCamera::HandleLook3D(float deltaX, float deltaY)
{
	m_yaw += deltaX * m_lookSens;
	m_pitch -= deltaY * m_lookSens;
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

Matrix4 EditorCamera::GetProjectionMatrix(float aspectRatio) const {
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

void EditorCamera::MoveToTarget(const Vec3& target, float distance)
{
	m_position = target - GetForward() * distance;
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

void EditorCamera::ResetPosition()
{
	m_position = Vec3(0.0f, 0.0f, 0.0f);
	m_yaw = 0.0f;
	m_pitch = 0.0f;
}

void EditorCamera::ResetSettings()
{
	m_fov = 60.0f;
	m_nearClip = 0.03f;
	m_farClip = 10000.0f;
	m_speed = 1.0f;
	m_speedMin = 0.01f;
	m_speedMax = 2.0f;
	m_orthoSize = 5.0f;
	m_isOrtho = false;
	m_mode = Mode::Mode3D;
}