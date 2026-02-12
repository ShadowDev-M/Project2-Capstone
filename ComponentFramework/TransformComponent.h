#pragma once

class Actor;

using namespace MATH;
class TransformComponent : public Component {
	friend class XMLtest;

private:
	Vec3 pos;
	Vec3 scale;
	Quaternion orientation;

 
public:
	TransformComponent(Component* parent_);
	TransformComponent(Component* parent_,Vec3 pos_, Quaternion orientation_, Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f));
	~TransformComponent();
	bool OnCreate();
	void OnDestroy();
	void Update(const float deltaTime_);
	void Render() const;

	Vec3 GetPosition() const { return pos; }

	float GetX() const { return pos.x; }
	float GetY() const { return pos.y; }
	float GetZ() const { return pos.z; }

	Ref<Actor> getParent();

	Vec3 GetScale() const { return scale; }
	Quaternion GetOrientation() const { return orientation; }
	Matrix4 GetTransformMatrix() const;
	void SetTransform(Vec3 pos_, Quaternion orientation_, Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f) ) {
		pos = pos_;
		orientation = orientation_;
		scale = scale_;
	}

	void SetPos(float x_, float y_, float z_) {
		pos.x = x_;
		pos.y = y_;
		pos.z = z_;
	}

	void SetPos(Vec3 pos_) {
		pos = pos_;
	}

	void SetX(float x_) {
		pos.x = x_;
	}
	void SetY(float x_) {
		pos.x = x_;
	}
	void SetZ(float x_) {
		pos.x = x_;
	}
	void SetOrientation(Quaternion orientation_) {
		orientation = orientation_;
	}

	void SetScale(Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f)) {
		scale = scale_;
	}

	Vec3 GetForward() {
		Vec3 localForward = Vec3(0.0f, 0.0f, -1.0f);
		return orientation * localForward;
	}

};

