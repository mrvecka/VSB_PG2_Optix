#include "pch.h"
#include "camera.h"
#include "raytracer.h"
#include "utils.h"
Camera::Camera( const int width, const int height, const float fov_y,
	const Vector3 view_from, const Vector3 view_at )
{
	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	// TODO compute focal lenght based on the vertical field of view and the camera resolution
	f_y_ = height  / (2 * tanf(fov_y * 0.5f));
	
	// TODO build M_c_w_ matrix	
	
	recalculateMcw();
}

float Camera::focalLength() {
	return f_y_;
}
Matrix3x3 Camera::M_c_w() {
	return M_c_w_;
}
Vector3 Camera::view_from(){
	return view_from_;
}

Vector3 Camera::view_at(){
	return view_at_;
}

void Camera::updateFov(const float fov_y) {
	f_y_ = height_ / (2 * tanf(fov_y * 0.5f));
}

Vector3 Camera::up() {
	return up_;
}

void Camera::updateViewFrom(const Vector3 view_from) {
	view_from_ = view_from;
	recalculateMcw();
}

void Camera::updateUpVector(const Vector3 up) {
	up_ = up;
	recalculateMcw();
}

void Camera::updateViewAt(const Vector3 view_at) {
	view_at_ = view_at;
	recalculateMcw();
}

void Camera::updateViewAtAndViewFrom(const Vector3 view_at, const Vector3 view_from) {
	view_at_ = view_at;
	view_from_ = view_from;
	recalculateMcw();
}

void Camera::recalculateMcw() {

	basis_z = view_from_ - view_at_;
	basis_z.Normalize();

	basis_x = up_.CrossProduct(basis_z);
	basis_x.Normalize();

	basis_y = basis_z.CrossProduct(basis_x);
	basis_y.Normalize();

	up_ = basis_y;

	M_c_w_ = Matrix3x3(basis_x, basis_y, basis_z);
}

