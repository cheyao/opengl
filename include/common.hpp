#pragma once

#include <SDL3/SDL.h>

#include <cmath>

// Warper functins just incase we change platforms
namespace maths {

const float PI = 3.14159265358979323846;

inline float toRadians(float degrees) { return degrees * (PI / 180.f); }

inline float toDegrees(float radians) { return radians * (180.f / PI); }

inline float cos(float angle) { return cosf(angle); }

inline float sin(float angle) { return sinf(angle); }

inline float tan(float angle) { return tanf(angle); }

inline float arccos(float angle) { return acosf(angle); }

inline float arcsin(float angle) { return asinf(angle); }

inline float arctan(float angle) { return atanf(angle); }

inline float arctan2(float y, float x) { return atan2(y, x); }

inline float sqrt(float number) { return sqrtf(number); }

inline bool nearZero(float number, float epsilon = 0.001f) {
	if (fabs(number) <= epsilon) {
		return true;
	} else {
		return false;
	}
}

}  // namespace maths

class Vector2 {
       public:
	float x;
	float y;

	Vector2() : x(0), y(0) {}

	explicit Vector2(float x, float y) : x(x), y(y) {}

	operator SDL_FPoint() const {
		SDL_FPoint point;
		point.x = x;
		point.y = y;

		return point;
	}

	friend Vector2 operator+(const Vector2& a, const Vector2& b) {
		return Vector2(b.x + a.x, b.y + a.y);
	}
	Vector2& operator+=(const Vector2& right) {
		x += right.x;
		y += right.y;
		return *this;
	}

	friend Vector2 operator-(const Vector2& a, const Vector2& b) {
		return Vector2(b.x - a.x, b.y - a.y);
	}
	Vector2& operator-=(const Vector2& right) {
		x -= right.x;
		y -= right.y;
		return *this;
	}
	Vector2& operator-() {
		x = -x;
		y = -y;
		return *this;
	}

	friend Vector2 operator*(const Vector2& a, const Vector2& b) {
		return Vector2(a.x * b.x, a.y * b.y);
	}
	Vector2& operator*=(const Vector2& right) {
		x *= right.x;
		y *= right.y;
		return *this;
	}

	// Scalar
	friend Vector2 operator*(const Vector2& a, const float scalar) {
		return Vector2(a.x * scalar, a.y * scalar);
	}
	friend Vector2 operator*(const float scalar, const Vector2& a) {
		return Vector2(a.x * scalar, a.y * scalar);
	}
	Vector2& operator*=(const float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	float lengthSquared() const { return (x * x + y * y); }

	float length() const { return maths::sqrt(lengthSquared()); }

	void normalize() {
		float vectorLength = this->length();
		// Error catching
		if (vectorLength == 0) {
			x = 0;
			y = 0;

			return;
		}
		x /= vectorLength;
		y /= vectorLength;
	}

	static Vector2 normalize(const Vector2& vec) {
		Vector2 temp = vec;
		temp.normalize();
		return temp;
	}

	static float dot(const Vector2& a, const Vector2& b) {
		return (a.x * b.x + a.y * b.y);
	}
};

typedef struct Mouse {
	enum type {
		BUTTON_STATE_UP,
		BUTTON_STATE_LEFT = 1,
		BUTTON_STATE_MIDDLE,
		BUTTON_STATE_RIGHT,
		BUTTON_STATE_X1,
		BUTTON_STATE_X2
	} type;
	class Vector2 position;
	bool captured;	// Reserved by program, remember to free!
} TouchEvent;

