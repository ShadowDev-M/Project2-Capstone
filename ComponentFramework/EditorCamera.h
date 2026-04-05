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

	void ResetView();
	void ResetSettings();

	void SetOrientation(float yawDeg, float pitchDeg);
	void FrameTarget(const Vec3& target, float distance = 15.0f);

	// 2D Mode helpers
	void EnterMode2D(const std::vector<Vec3>& positions);
	void Enforce2DModeConstraints();

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
	float GetScrollSpeed() const { return m_scrollSpeed; }
	Mode GetMode() const { return m_mode; }

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
		m_speedMax = std::min(10000.0f, speedMax);
		m_speed = std::clamp(m_speed, m_speedMin, m_speedMax);
	}
	void SetScrollSpeed(float speed) { m_scrollSpeed = speed; }
	void SetMode(Mode mode) { m_mode = mode; }

	// calculates the projection and view matrices and returns them (the editor camera itself has no view or projection variables)
	Matrix4 GetViewMatrix() const;
	Matrix4 GetProjectionMatrix() const;

	// speed popup
	bool GetShowSpeedPopup() const { return m_showSpeedPopup; }
	float GetSpeedPopupAlpha() const { return std::min(1.0f, m_speedPopupTimer / 0.4f); }

	void SetIsOrtho(bool ortho) { m_isOrtho = ortho; m_position.z = 100.0f; }
	bool isOrtho() const { return m_mode == Mode::Mode2D || m_isOrtho; }
	bool isRMBHeld() const { return m_rmbActive; }
	bool isMMBHeld() const { return m_mmbActive; }

	Vec3 GetForward() const;

private:
	// no longer using lookAt, now building the rotation quaternion
	Quaternion BuildRotationQuat() const;

	// very heavily based off unitys editor/scene camera
	void HandlePan2D(float deltaX, float deltaY);
	void HandlePan3D(float deltaX, float deltaY);
	void HandleLook3D(float deltaX, float deltaY);
	void HandleFly3D(float deltaTime);

	Vec3 GetRight() const;
	Vec3 GetUp() const;

	// for speed popup
	bool m_showSpeedPopup = false;
	float m_speedPopupTimer = 0.0f;

	// transform based variables
	Vec3 m_position = Vec3(0.0f, 0.0f, 10.0f);
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	// camera settings
	float m_fov = 60.0f;
	float m_nearClip = 0.03f;
	float m_farClip = 10000.0f;
	float m_orthoSize = 5.0f;
	float m_speed = 5.0f;
	float m_speedMin = 0.1f;
	float m_speedMax = 100.0f;
	float m_scrollSpeed = 5.0f;
	float m_panSpeed = 10.0f;
	bool m_isOrtho = false;
	Mode m_mode = Mode::Mode3D;

	// TODO: sort of placeholders so theres no magic/floating numbers in the calculations, could make getters and setters for them after
	float m_lookSens = 0.15f;
	float m_panSens = 0.003f;

	// rmb state
	bool m_rmbActive = false;
	bool m_wasRMBDown = false;
	bool m_skipRMBDelta = false;
	int m_RMBCursorX = 0;
	int m_RMBCursorY = 0;

	// mmb state
	bool m_mmbActive = false;
	bool m_wasMMBDown = false;
	bool m_skipMMBDelta = false;
	int m_MMBCursorX = 0;
	int m_MMBCursorY = 0;
};