#include "pch.h"
#include "raytracer.h"
#include "mymath.h"

/* OptiX error reporting function */
void error_handler( RTresult code )
{
	if ( code != RT_SUCCESS )
	{

		throw std::runtime_error( "RT_ERROR_UNKNOWN" );
	}
}

/* create OptiX context and trace a single triangle with an ortho camera */
int tutorial_1()
{
	int width = 640;
	int height = 480;

	unsigned int version, count;
	{
		error_handler( rtGetVersion( &version ) );
		error_handler( rtDeviceGetDeviceCount( &count ) );
		const int major = version / 10000;
		const int minor = ( version - major * 10000 ) / 100;
		const int micro = version - major * 10000 - minor * 100;
		printf( "Nvidia OptiX %d.%d.%d, %d device(s) found\n", major, minor, micro, count );
	}

	/*int rtx_mode = 1;
	error_handler( rtGlobalSetAttribute( RT_GLOBAL_ATTRIBUTE_ENABLE_RTX, sizeof( rtx_mode ), &rtx_mode ) ); */

	for ( unsigned int i = 0; i < count; ++i )
	{
		char name[64];
		error_handler( rtDeviceGetAttribute( i, RT_DEVICE_ATTRIBUTE_NAME, 64, name ) );
		RTsize memory_size = 0;
		error_handler( rtDeviceGetAttribute( i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof( RTsize ), &memory_size ) );
		printf( "%d : %s (%0.0f MB)\n", i, name, memory_size / ( 1024.0f * 1024.0f ) );
	}

	RTcontext context = 0;
	error_handler( rtContextCreate( &context ) );
	error_handler( rtContextSetRayTypeCount( context, 1 ) );
	error_handler( rtContextSetEntryPointCount( context, 1 ) );
	error_handler( rtContextSetMaxTraceDepth( context, 10 ) );

	RTvariable output;
	error_handler( rtContextDeclareVariable( context, "output_buffer", &output ) );
	// OptiX buffers are used to pass data between the host and the device
	RTbuffer output_buffer;
	error_handler( rtBufferCreate( context, RT_BUFFER_OUTPUT, &output_buffer ) );
	// before using a buffer, its size, dimensionality and element format must be specified
	error_handler( rtBufferSetFormat( output_buffer, RT_FORMAT_UNSIGNED_BYTE4 ) );
	error_handler( rtBufferSetSize2D( output_buffer, width, height ) );
	// sets a program variable to an OptiX object value
	error_handler( rtVariableSetObject( output, output_buffer ) );
	// host access to the data stored within a buffer is performed with the rtBufferMap function
	// all buffers must be unmapped via rtBufferUnmap before context validation will succeed


	RTprogram primary_ray;
	// https://devtalk.nvidia.com/default/topic/1043216/optix/how-to-generate-ptx-file-using-visual-studio/
	error_handler( rtProgramCreateFromPTXFile( context, "optixtutorial.ptx", "primary_ray", &primary_ray ) );
	error_handler( rtContextSetRayGenerationProgram( context, 0, primary_ray ) );
	error_handler( rtProgramValidate( primary_ray ) );

	RTprogram exception;
	error_handler( rtProgramCreateFromPTXFile( context, "optixtutorial.ptx", "exception", &exception ) );
	error_handler( rtContextSetExceptionProgram( context, 0, exception ) );
	error_handler( rtProgramValidate( exception ) );
	error_handler( rtContextSetExceptionEnabled( context, RT_EXCEPTION_ALL, 1 ) );

	error_handler( rtContextSetPrintEnabled( context, 1 ) );
	error_handler( rtContextSetPrintBufferSize( context, 4096 ) );

	RTprogram miss_program;
	error_handler( rtProgramCreateFromPTXFile( context, "optixtutorial.ptx", "miss_program", &miss_program ) );
	error_handler( rtContextSetMissProgram( context, 0, miss_program ) );
	error_handler( rtProgramValidate( miss_program ) );
	
	// RTgeometrytriangles type provides OptiX with built-in support for triangles	
	// geometry
	RTgeometrytriangles geometry_triangles;
	error_handler( rtGeometryTrianglesCreate( context, &geometry_triangles ) );
	error_handler( rtGeometryTrianglesSetPrimitiveCount( geometry_triangles, 1 ) );
	RTbuffer vertex_buffer;
	error_handler( rtBufferCreate( context, RT_BUFFER_INPUT, &vertex_buffer ) );
	error_handler( rtBufferSetFormat( vertex_buffer, RT_FORMAT_FLOAT3 ) );
	error_handler( rtBufferSetSize1D( vertex_buffer, 3 ) );
	{
		optix::float3 * data = nullptr;
		error_handler( rtBufferMap( vertex_buffer, ( void** )( &data ) ) );
		data[0].x = 0.0f; data[0].y = 0.0f; data[0].z = 0.0f;
		data[1].x = 200.0f; data[1].y = 0.0f; data[1].z = 0.0f;
		data[2].x = 0.0f; data[2].y = 150.0f; data[2].z = 0.0f;
		error_handler( rtBufferUnmap( vertex_buffer ) );
		data = nullptr;
	}
	error_handler( rtGeometryTrianglesSetVertices( geometry_triangles, 3, vertex_buffer, 0, sizeof( optix::float3 ), RT_FORMAT_FLOAT3 ) );
	//rtGeometryTrianglesSetTriangles();
	error_handler( rtGeometryTrianglesValidate( geometry_triangles ) );

	// material
	RTmaterial material;
	error_handler( rtMaterialCreate( context, &material ) );
	RTprogram closest_hit;
	error_handler( rtProgramCreateFromPTXFile( context, "optixtutorial.ptx", "closest_hit", &closest_hit ) );
	error_handler( rtProgramValidate( closest_hit ) );
	error_handler( rtMaterialSetClosestHitProgram( material, 0, closest_hit ) );
	//rtMaterialSetAnyHitProgram( material, 0, any_hit );	
	error_handler( rtMaterialValidate( material ) );

	// geometry instance
	RTgeometryinstance geometry_instance;
	error_handler( rtGeometryInstanceCreate( context, &geometry_instance ) );
	error_handler( rtGeometryInstanceSetGeometryTriangles( geometry_instance, geometry_triangles ) );
	error_handler( rtGeometryInstanceSetMaterialCount( geometry_instance, 1 ) );
	error_handler( rtGeometryInstanceSetMaterial( geometry_instance, 0, material ) );
	error_handler( rtGeometryInstanceValidate( geometry_instance ) );
	// ---

	// acceleration structure
	RTacceleration sbvh;
	error_handler( rtAccelerationCreate( context, &sbvh ) );
	error_handler( rtAccelerationSetBuilder( sbvh, "Sbvh" ) );
	//error_handler( rtAccelerationSetProperty( sbvh, "vertex_buffer_name", "vertex_buffer" ) );
	error_handler( rtAccelerationValidate( sbvh ) );

	// geometry group
	RTgeometrygroup geometry_group;
	error_handler( rtGeometryGroupCreate( context, &geometry_group ) );
	error_handler( rtGeometryGroupSetAcceleration( geometry_group, sbvh ) );
	error_handler( rtGeometryGroupSetChildCount( geometry_group, 1 ) );
	error_handler( rtGeometryGroupSetChild( geometry_group, 0, geometry_instance ) );
	error_handler( rtGeometryGroupValidate( geometry_group ) );

	RTvariable top_object;
	error_handler( rtContextDeclareVariable( context, "top_object", &top_object ) );
	error_handler( rtVariableSetObject( top_object, geometry_group ) );

	// group

	error_handler( rtContextValidate( context ) );
	error_handler( rtContextLaunch2D( context, 0, width, height ) );

	optix::uchar4 * data = nullptr;
	error_handler( rtBufferMap( output_buffer, ( void** )( &data ) ) );
	FILE * file = fopen( "output.ppm", "wt" );
	fprintf( file, "P3\n%d %d\n255\n", width, height );
	for ( int y = 0, z = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			fprintf( file, "%03d %03d %03d\t", data[x + y * width].x, data[x + y * width].y, data[x + y * width].z );
			if ( z++ == 4 )
			{
				z = 0;
				fprintf( file, "\n" );
			}
		}
	}
	fclose( file );
	file = nullptr;
	printf( "%d %d %d %d\n", data[0].x, data[0].y, data[0].z, data[0].w );
	error_handler( rtBufferUnmap( output_buffer ) );
	data = nullptr;

	// cleanup
	error_handler( rtGeometryTrianglesDestroy( geometry_triangles ) );
	error_handler( rtBufferDestroy( output_buffer ) );
	error_handler( rtProgramDestroy( primary_ray ) );

	error_handler( rtContextDestroy( context ) );

	return EXIT_SUCCESS;
}

/* a simple example showing how to display a bitmap with traced image at intaractive frame rates */
int tutorial_2( const std::string file_name )
{
	Raytracer raytracer(640, 480, deg2rad(45.0), Vector3(175, -140, 130), Vector3(0, 0, 35));
	raytracer.InitDeviceAndScene();
	raytracer.LoadScene( file_name );
	raytracer.initGraph();
	raytracer.MainLoop();

	return EXIT_SUCCESS;
}
