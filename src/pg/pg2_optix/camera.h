#ifndef CAMERA_H_
#define CAMERA_H_

#include "vector3.h"
#include "matrix3x3.h"

/*! \class Camera
\brief A simple pin-hole camera.

\author Tomáš Fabián
\version 1.0
\date 2018
*/
class Camera
{
public:
	Camera() { }

	Camera( const int width, const int height, const float fov_y,
		const Vector3 view_from, const Vector3 view_at );
	float focalLength();
	Matrix3x3 M_c_w();
	Vector3 view_from();
	Vector3 view_at();
	Vector3 up();
	Vector3 basis_y;
	Vector3 basis_x;
	Vector3 basis_z;

	void recalculateMcw();
	void updateViewFrom(const Vector3 view_from);
	void updateUpVector(const Vector3 up);
	void updateViewAt(const Vector3 view_at);
	void updateViewAtAndViewFrom(const Vector3 view_at, const Vector3 view_from);
	void updateFov(const float fov_y);
private:
	int width_{ 640 }; // image width (px)
	int height_{ 480 };  // image height (px)
	float fov_y_{ 0.785f }; // vertical field of view (rad)
	
	Vector3 view_from_; // ray origin or eye or O
	Vector3 view_at_; // target T
	Vector3 up_{ Vector3( 0.0f, 0.0f, 1.0f ) }; // up vector

	float f_y_{ 1.0f }; // focal lenght (px)
	Matrix3x3 M_c_w_;
};

#endif
