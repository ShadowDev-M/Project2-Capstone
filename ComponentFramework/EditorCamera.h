#pragma once

// full freefly camera seperated from actor and cameracomponent
// solely accessed by the scenewindow

// to switch between 2D and 3D
enum class Mode { Mode2D, Mode3D };

class EditorCamera
{
public:
	EditorCamera() = default;
	~EditorCamera() = default;

	void Update(float deltaTime, bool isHovered);

	void ResetPosition();
	void ResetSettings();

	void SetOrientation(float yawDeg, float pitchDeg);
	void MoveToTarget(const Vec3& target, float distance);

	// getters for transform 
	Vec3 GetPosition() const { return m_position; }
	float GetYaw() const { return m_yaw; }
	float GetPitch() const { return m_pitch; }

	// setters for transform
	void SetPosition(const Vec3& pos) { m_position = pos; }
	void SetYaw(float yaw) { m_yaw = yaw; }
	void SetPitch(float pitch) { m_pitch = std::clamp(pitch, -89.0f, 89.0f); }

	// getters for settings
	float GetFOV() const { return m_fov; }
	float GetNearClip() const { return m_nearClip; }
	float GetFarClip() const { return m_farClip; }
	float GetOrthoSize() const { return m_orthoSize; }
	float GetCameraSpeed() const { return m_speed; }
	float GetSpeedMin() const { return m_speedMin; }
	float GetSpeedMax() const { return m_speedMax; }

	// setters for settings
	void SetFOV(float fov) { m_fov = fov; }
	void SetNearClip(float near) { m_nearClip = std::max(0.0001f, near); }
	void SetFarClip(float far) { m_farClip = far; }
	void SetOrthoSize(float size) { m_orthoSize = std::max(0.01f, size); }
	void SetCameraSpeed(float speed) { m_speed = std::clamp(speed, m_speedMin, m_speedMax); }
	void SetSpeedMin(float speedMin) { 
		m_speedMin = std::max(0.0001f, speedMin);
		m_speed = std::clamp(m_speed, m_speedMin, m_speedMax);
	}
	void SetSpeedMax(float speedMax) { 
		m_speedMin = std::min(10000.0f, speedMax);
		m_speed = std::clamp(m_speed, m_speedMin, m_speedMax);
	}

	// calculates the projection and view matrices and returns them (the editor camera itself has no view or projection variables)
	Matrix4 GetViewMatrix() const;
	Matrix4 GetProjectionMatrix(float aspectRatio) const;

	bool isOrtho() const { return m_mode == Mode::Mode2D || m_isOrtho; }
	bool isRMBHeld() const { return ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]; }

private:
	// very heavily based off unitys editor/scene camera
	void HandlePan2D(float deltaX, float deltaY);
	void HandlePan3D(float deltaX, float deltaY);
	void HandleLook3D(float deltaX, float deltaY);
	void HandleFly3D(float deltaTime);

	Vec3 GetForward() const;
	Vec3 GetRight() const;
	Vec3 GetUp() const;

	// transform based variables
	Vec3 m_position = Vec3(0.0f, 0.0f, 0.0f);
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	// camera settings
	float m_fov = 60.0f;
	float m_nearClip = 0.03f;
	float m_farClip = 10000.0f;
	float m_orthoSize = 5.0f;
	float m_speed = 1.0f;
	float m_speedMin = 0.01f;
	float m_speedMax = 2.0f;
	bool m_isOrtho = false;
	Mode m_mode = Mode::Mode3D;

	// TODO: sort of placeholders so theres no magic/floating numbers in the calculations, could make getters and setters for them after
	float m_lookSens = 0.2f;
	float m_panSens = 0.003f;
};