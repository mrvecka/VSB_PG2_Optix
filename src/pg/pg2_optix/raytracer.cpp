#include "pch.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"
#include "mymath.h"
#include "omp.h"
#include "utils.h"

void Raytracer::error_handler(RTresult code)
{
	if (code != RT_SUCCESS)
	{
		const char* error_string;
		rtContextGetErrorString(context, code, &error_string);
		printf(error_string);
		throw std::runtime_error("RT_ERROR_UNKNOWN");
	}
}

Raytracer::Raytracer( const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at) : SimpleGuiDX11( width, height )
{
	InitDeviceAndScene();
	camera = Camera(width, height, fov_y, view_from, view_at);
	fov = fov_y;
}

Raytracer::~Raytracer()
{
	ReleaseDeviceAndScene();
}

int Raytracer::InitDeviceAndScene()
{	
	error_handler(rtContextCreate(&context));
	error_handler(rtContextSetRayTypeCount(context, 2));
	error_handler(rtContextSetEntryPointCount(context, 1));
	error_handler(rtContextSetMaxTraceDepth(context, 10));

	RTvariable output;
	error_handler(rtContextDeclareVariable(context, "output_buffer", &output));
	error_handler(rtBufferCreate(context, RT_BUFFER_OUTPUT, &outputBuffer));
	error_handler(rtBufferSetFormat(outputBuffer, RT_FORMAT_UNSIGNED_BYTE4));
	error_handler(rtBufferSetSize2D(outputBuffer, width(), height()));
	error_handler(rtVariableSetObject(output, outputBuffer));

	RTprogram primary_ray;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "primary_ray", &primary_ray));
	error_handler(rtContextSetRayGenerationProgram(context, 0, primary_ray));
	error_handler(rtProgramValidate(primary_ray));

	rtProgramDeclareVariable(primary_ray, "focal_length",
		&focal_length);
	rtProgramDeclareVariable(primary_ray, "view_from",
		&view_from);
	rtProgramDeclareVariable(primary_ray, "M_c_w",
		&M_c_w);

	rtVariableSet3f(view_from, camera.view_from().x, camera.view_from().y, camera.view_from().z);
	rtVariableSet1f(focal_length, camera.focalLength());
	rtVariableSetMatrix3x3fv(M_c_w, 0, camera.M_c_w().data());

	RTprogram exception;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "exception", &exception));
	error_handler(rtContextSetExceptionProgram(context, 0, exception));
	error_handler(rtProgramValidate(exception));
	error_handler(rtContextSetExceptionEnabled(context, RT_EXCEPTION_ALL, 1));

	error_handler(rtContextSetPrintEnabled(context, 1));
	error_handler(rtContextSetPrintBufferSize(context, 4096));

	RTprogram miss_program;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "miss_program", &miss_program));
	error_handler(rtContextSetMissProgram(context, 0, miss_program));
	error_handler(rtProgramValidate(miss_program));

	return S_OK;
}

int Raytracer::ReleaseDeviceAndScene()
{
	error_handler(rtContextDestroy(context));
	return S_OK;
}

int Raytracer::initGraph() {
	error_handler(rtContextValidate(context));

	return S_OK;
}

int Raytracer::get_image(BYTE * buffer) {
	camera.updateFov(fov);
	rtVariableSet3f(view_from, camera.view_from().x, camera.view_from().y, camera.view_from().z);
	rtVariableSet1f(focal_length, camera.focalLength());
	rtVariableSetMatrix3x3fv(M_c_w, 0, camera.M_c_w().data());

	error_handler(rtContextLaunch2D(context, 0, width(), height()));
	optix::uchar4 * data = nullptr;
	error_handler(rtBufferMap(outputBuffer, (void**)(&data)));
	memcpy(buffer, data, sizeof(optix::uchar4) * width() * height());
	error_handler(rtBufferUnmap(outputBuffer));
	return S_OK;
}

void Raytracer::LoadScene( const std::string file_name )
{
	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );
	
	int no_triangles = 0;

	for (auto surface : surfaces_)
	{
		no_triangles += surface->no_triangles();
	}

	RTgeometrytriangles geometry_triangles;
	error_handler(rtGeometryTrianglesCreate(context, &geometry_triangles));
	error_handler(rtGeometryTrianglesSetPrimitiveCount(geometry_triangles, no_triangles));

	RTbuffer vertex_buffer;
	error_handler(rtBufferCreate(context, RT_BUFFER_INPUT, &vertex_buffer));
	error_handler(rtBufferSetFormat(vertex_buffer, RT_FORMAT_FLOAT3));
	error_handler(rtBufferSetSize1D(vertex_buffer, no_triangles * 3));
	
	RTvariable normals;
	rtContextDeclareVariable(context, "normal_buffer", &normals);
	RTbuffer normal_buffer;
	error_handler(rtBufferCreate(context, RT_BUFFER_INPUT, &normal_buffer));
	error_handler(rtBufferSetFormat(normal_buffer, RT_FORMAT_FLOAT3));
	error_handler(rtBufferSetSize1D(normal_buffer, no_triangles * 3));

	RTvariable materialIndices;
	rtContextDeclareVariable(context, "material_buffer", &materialIndices);
	RTbuffer material_buffer;
	error_handler(rtBufferCreate(context, RT_BUFFER_INPUT, &material_buffer));
	error_handler(rtBufferSetFormat(material_buffer, RT_FORMAT_UNSIGNED_BYTE));
	error_handler(rtBufferSetSize1D(material_buffer, no_triangles));

	RTvariable texcoords;
	rtContextDeclareVariable(context, "texcoord_buffer", &texcoords);
	RTbuffer texcoord_buffer;
	error_handler(rtBufferCreate(context, RT_BUFFER_INPUT, &texcoord_buffer));
	error_handler(rtBufferSetFormat(texcoord_buffer, RT_FORMAT_FLOAT2));
	error_handler(rtBufferSetSize1D(texcoord_buffer, no_triangles * 3));

	optix::float3* vertexData = nullptr;
	optix::float3* normalData = nullptr;
	optix::uchar1* materialData = nullptr;
	optix::float2* texcoordData = nullptr;

	error_handler(rtBufferMap(vertex_buffer, (void**)(&vertexData)));
	error_handler(rtBufferMap(normal_buffer, (void**)(&normalData)));
	error_handler(rtBufferMap(material_buffer, (void**)(&materialData)));
	error_handler(rtBufferMap(texcoord_buffer, (void**)(&texcoordData)));

	// surfaces loop
	int k = 0, l = 0;
	for ( auto surface : surfaces_ )
	{		

		// triangles loop
		for (int i = 0; i < surface->no_triangles(); ++i, ++l )
		{
			Triangle & triangle = surface->get_triangle( i );

			materialData[l].x = (unsigned char)surface->get_material()->materialIndex;

			// vertices loop
			for ( int j = 0; j < 3; ++j, ++k )
			{
				const Vertex & vertex = triangle.vertex(j);
				vertexData[k].x = vertex.position.x; 
				vertexData[k].y = vertex.position.y;
				vertexData[k].z = vertex.position.z;
				//printf("%d \n", k);
				normalData[k].x = vertex.normal.x;
				normalData[k].y = vertex.normal.y;
				normalData[k].z = vertex.normal.z;

				texcoordData[k].x = vertex.texture_coords->u;
				texcoordData[k].y = vertex.texture_coords->v;
			} // end of vertices loop

		} // end of triangles loop

	} // end of surfaces loop

	rtBufferUnmap(normal_buffer);
	rtBufferUnmap(material_buffer);
	rtBufferUnmap(vertex_buffer);
	rtBufferUnmap(texcoord_buffer);

	rtBufferValidate(texcoord_buffer);
	rtVariableSetObject(texcoords, texcoord_buffer);
	rtBufferValidate(normal_buffer);
	rtVariableSetObject(normals, normal_buffer);

	rtBufferValidate(material_buffer);
	rtVariableSetObject(materialIndices, material_buffer);
	rtBufferValidate(vertex_buffer);

	error_handler(rtGeometryTrianglesSetMaterialCount(geometry_triangles, materials_.size()));
	error_handler(rtGeometryTrianglesSetMaterialIndices(geometry_triangles, material_buffer, 0, sizeof(optix::uchar1), RT_FORMAT_UNSIGNED_BYTE));
	error_handler(rtGeometryTrianglesSetVertices(geometry_triangles, no_triangles * 3, vertex_buffer, 0, sizeof(optix::float3), RT_FORMAT_FLOAT3));

	RTprogram attribute_program;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "attribute_program", &attribute_program));
	error_handler(rtProgramValidate(attribute_program));
	error_handler(rtGeometryTrianglesSetAttributeProgram(geometry_triangles, attribute_program));

	error_handler(rtGeometryTrianglesValidate(geometry_triangles));

	// geometry instance
	RTgeometryinstance geometry_instance;
	error_handler(rtGeometryInstanceCreate(context, &geometry_instance));
	error_handler(rtGeometryInstanceSetGeometryTriangles(geometry_instance, geometry_triangles));
	error_handler(rtGeometryInstanceSetMaterialCount(geometry_instance, materials_.size()));

	/*RTprogram any_hit;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "any_hit", &any_hit));*/
	RTprogram shader_hit;
	error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "shader_hit", &shader_hit));

	for (Material* material : materials_) {
		RTmaterial rtMaterial;
		error_handler(rtMaterialCreate(context, &rtMaterial));
		RTprogram closest_hit;
		
		
		switch (material->shader())
		{
		case Shader::PHONG:
			
			error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "closest_hit_Phong", &closest_hit));
			break;
			
		case Shader::NORMAL:
			
			error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "closest_hit_Normal", &closest_hit));
			break;
			
		case Shader::LAMBERT:
			
			error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "closest_hit_Lambert", &closest_hit));
			break;
			
		default:
			error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "closest_hit_Lambert", &closest_hit));

			break;
		
		}
		error_handler(rtProgramCreateFromPTXFile(context, "optixtutorial.ptx", "closest_hit_Phong", &closest_hit));
		error_handler(createAndSetMaterialColorVariable(rtMaterial, "diffuse", material->diffuse()));
		error_handler(createAndSetMaterialColorVariable(rtMaterial, "specular", material->specular()));
		error_handler(createAndSetMaterialColorVariable(rtMaterial, "ambient", material->ambient()));
		error_handler(createAndSetMaterialScalarVariable(rtMaterial, "shininess", material->shininess));

		RTvariable tex_diffuse_id;
		rtMaterialDeclareVariable(rtMaterial, "tex_diffuse_id", &tex_diffuse_id);

		if (material->texture(material->kDiffuseMapSlot) != NULL) {
			RTtexturesampler textureSampler;
			rtTextureSamplerCreate(context, &textureSampler);
			int texture_id;
			rtTextureSamplerGetId(textureSampler, &texture_id);

			optix::float4* textureData = nullptr;

			rtVariableSet1i(tex_diffuse_id, texture_id);
			Texture* texture = material->texture(material->kDiffuseMapSlot);
			RTbuffer texture_buffer;
			error_handler(rtBufferCreate(context, RT_BUFFER_INPUT, &texture_buffer));
			error_handler(rtBufferSetFormat(texture_buffer, RT_FORMAT_FLOAT4));
			error_handler(rtBufferSetSize2D(texture_buffer, texture->width(), texture->height()));
			error_handler(rtBufferMap(texture_buffer, (void**)(&textureData)));

			for (int i = 0; i < (texture->height() * texture->width()); i++) {
				textureData[i] = optix::make_float4(texture->getData()[3 * i] / 255.0f, texture->getData()[3 * i + 1] / 255.0f, texture->getData()[3 * i+2] / 255.0f, 1);
			}

			rtTextureSamplerSetReadMode(textureSampler, RT_TEXTURE_READ_NORMALIZED_FLOAT);

			error_handler(rtBufferUnmap(texture_buffer));
			error_handler(rtTextureSamplerSetBuffer(textureSampler, 0, 0, texture_buffer));
			error_handler(rtTextureSamplerValidate(textureSampler));
		}
		else {
			rtVariableSet1i(tex_diffuse_id, -1);
		}

		error_handler(rtProgramValidate(closest_hit));
		error_handler(rtMaterialSetClosestHitProgram(rtMaterial, 0, closest_hit));
		error_handler(rtProgramValidate(shader_hit));
		error_handler(rtMaterialSetAnyHitProgram(rtMaterial, 1, shader_hit));
		/*error_handler(rtProgramValidate(any_hit));
		error_handler(rtMaterialSetAnyHitProgram(rtMaterial, 1, any_hit));*/

		error_handler(rtMaterialValidate(rtMaterial));

		error_handler(rtGeometryInstanceSetMaterial(geometry_instance, material->materialIndex, rtMaterial));
	}
	error_handler(rtGeometryInstanceValidate(geometry_instance));


	// acceleration structure
	RTacceleration sbvh;
	error_handler(rtAccelerationCreate(context, &sbvh));
	error_handler(rtAccelerationSetBuilder(sbvh, "Sbvh"));
	//error_handler( rtAccelerationSetProperty( sbvh, "vertex_buffer_name", "vertex_buffer" ) );
	error_handler(rtAccelerationValidate(sbvh));

	// geometry group
	RTgeometrygroup geometry_group;
	error_handler(rtGeometryGroupCreate(context, &geometry_group));
	error_handler(rtGeometryGroupSetAcceleration(geometry_group, sbvh));
	error_handler(rtGeometryGroupSetChildCount(geometry_group, 1));
	error_handler(rtGeometryGroupSetChild(geometry_group, 0, geometry_instance));
	error_handler(rtGeometryGroupValidate(geometry_group));

	RTvariable top_object;
	error_handler(rtContextDeclareVariable(context, "top_object", &top_object));
	error_handler(rtVariableSetObject(top_object, geometry_group));
}

int Raytracer::Ui()
{
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin( "Ray Tracer Params" );
	
	ImGui::Text( "Surfaces = %d", surfaces_.size() );
	ImGui::Text( "Materials = %d", materials_.size() );
	ImGui::Separator();
	ImGui::Checkbox( "Vsync", &vsync_ );
	ImGui::Checkbox( "Unify normals", &unify_normals_ );	

	ImGui::SliderFloat( "gamma", &gamma_, 0.1f, 5.0f );
	ImGui::SliderFloat("fov", &fov, 0.1f, 5.0f);
	ImGui::SliderFloat("Mouse sensitivity", &mouseSensitivity, 0.1f, 100.0f);
	ImGui::SliderInt("'Speed", &speed, 0, 10);

	bool arrowUpPressed = GetKeyState(VK_UP) & 0x8000 ? true : false;
	bool arrowDownPressed = GetKeyState(VK_DOWN) & 0x8000 ? true : false;
	bool arrowLeftPressed = GetKeyState(VK_LEFT) & 0x8000 ? true : false;
	bool arrowRightPressed = GetKeyState(VK_RIGHT) & 0x8000 ? true : false;

	if (arrowUpPressed) {
		Vector3 forward = (camera.view_at() - camera.view_from());
		forward.Normalize();
		camera.updateViewAtAndViewFrom(camera.view_at() + speed * forward, camera.view_from() + speed * forward);
	}

	if (arrowDownPressed) {
		Vector3 forward = (camera.view_at() - camera.view_from());
		forward.Normalize();
		camera.updateViewAtAndViewFrom(camera.view_at() - speed * forward, camera.view_from() - speed * forward);
	}

	if (arrowLeftPressed || arrowRightPressed) {
		Vector3 forward = (camera.view_at() - camera.view_from());
		forward.Normalize();
		Vector3 right = forward.CrossProduct(camera.basis_y);
		arrowLeftPressed ? camera.updateViewAtAndViewFrom(camera.view_at() - speed * right, camera.view_from() - speed * right) :
						 camera.updateViewAtAndViewFrom(camera.view_at() + speed * right, camera.view_from() + speed * right);
	}

	bool wPressed, aPressed, sPressed, dPressed = false;

	wPressed = GetKeyState('W') & 0x8000 ? true : false;
	aPressed = GetKeyState('A') & 0x8000 ? true : false;
	sPressed = GetKeyState('S') & 0x8000 ? true : false;
	dPressed = GetKeyState('D') & 0x8000 ? true : false;

	if (aPressed || dPressed) {
		Vector3 forward = (camera.view_at() - camera.view_from());
		double forwardL = forward.L2Norm();
		forward.Normalize();
		Vector3 right = forward.CrossProduct(camera.basis_y);
		int yawRight = aPressed ? -1 : 1;

		Vector3 newViewAt = camera.view_at() + yawRight * right * (aPressed || dPressed ? speed : mouseSensitivity);
		double distanceRatio = forwardL / (newViewAt - camera.view_from()).L2Norm();
		Vector3 newVector = (newViewAt - camera.view_from());
		newVector *= distanceRatio;
		newViewAt = camera.view_from() + newVector;
		camera.updateViewAt(newViewAt);
	}
	
	if (wPressed || sPressed) {
		int pitchUp = wPressed ? -1 : 1;

		Vector3 up = camera.basis_y;
		Vector3 newViewAt = camera.view_at() + pitchUp * up * (wPressed || sPressed ? speed : mouseSensitivity);
		camera.updateViewAt(newViewAt);
	}

	bool zPressed = GetKeyState('Z') & 0x8000 ? true : false;
	bool cPressed = GetKeyState('C') & 0x8000 ? true : false;

	if (zPressed || cPressed) {
		int rollLeft = zPressed ? -1 : 1;
		camera.updateUpVector(camera.basis_y + 0.05 * rollLeft * camera.basis_x);
	}



	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
	ImGui::End();
	return 0;
}
