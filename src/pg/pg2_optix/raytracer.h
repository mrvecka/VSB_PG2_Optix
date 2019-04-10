#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"

/*! \class Raytracer
\brief General ray tracer class.

\author Tomáš Fabián
\version 0.1
\date 2018
*/
class Raytracer : public SimpleGuiDX11
{
public:
	Raytracer( const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at);
	~Raytracer();

	int InitDeviceAndScene();
	int initGraph();
	int get_image(BYTE * buffer) override;
	int ReleaseDeviceAndScene();

	void LoadScene( const std::string file_name );
	int Ui();

private:	
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;			
	
	RTcontext context = {0};
	RTbuffer outputBuffer = { 0 };
	RTvariable focal_length;
	RTvariable view_from;
	RTvariable M_c_w;

	Camera camera;
	float fov;

	bool unify_normals_{ true };
	void error_handler(RTresult code);

};
