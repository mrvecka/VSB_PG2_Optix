#include "optixtutorial.h"

struct TriangleAttributes
{
	optix::float3 normal;
	optix::float2 texcoord;
	optix::float3 intersectionPoint;
	optix::float3 vectorToLight;
};

rtBuffer<optix::float3, 1> normal_buffer;
rtBuffer<optix::float2, 1> texcoord_buffer;
rtBuffer<optix::uchar4, 2> output_buffer;

rtDeclareVariable(optix::float3, diffuse, , "diffuse");
rtDeclareVariable(optix::float3, specular, , "specular");
rtDeclareVariable(optix::float3, ambient, , "ambient");
rtDeclareVariable(float, shininess, , "shininess");

rtDeclareVariable(int, tex_diffuse_id, , "diffuse texture id");

rtDeclareVariable( rtObject, top_object, , );
rtDeclareVariable( uint2, launch_dim, rtLaunchDim, );
rtDeclareVariable( uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable( PerRayData_radiance, ray_data, rtPayload, );
rtDeclareVariable(PerRayData_shadow, shadow_ray_data, rtPayload, );
rtDeclareVariable( float2, barycentrics, attribute rtTriangleBarycentrics, );
rtDeclareVariable(TriangleAttributes, attribs, attribute attributes, "Triangle attributes");
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

rtDeclareVariable(optix::float3, view_from, , );
rtDeclareVariable(optix::Matrix3x3, M_c_w, , "camera to worldspace transformation matrix" );
rtDeclareVariable(float, focal_length, , "focal length in pixels" );

RT_PROGRAM void attribute_program(void)
{
	const optix::float3 lightPossition = optix::make_float3(100, 100, 200);
	const optix::float2 barycentrics = rtGetTriangleBarycentrics();
	const unsigned int index = rtGetPrimitiveIndex();
	const optix::float3 n0 = normal_buffer[index * 3 + 0];
	const optix::float3 n1 = normal_buffer[index * 3 + 1];
	const optix::float3 n2 = normal_buffer[index * 3 + 2];

	const optix::float2 t0 = texcoord_buffer[index * 3 + 0];
	const optix::float2 t1 = texcoord_buffer[index * 3 + 1];
	const optix::float2 t2 = texcoord_buffer[index * 3 + 2];

	attribs.normal = optix::normalize(n1 * barycentrics.x + n2 * barycentrics.y + n0 * (1.0f - barycentrics.x - barycentrics.y));
	attribs.texcoord = t1 * barycentrics.x + t2 * barycentrics.y + t0 * (1.0f - barycentrics.x - barycentrics.y);

	if (optix::dot(ray.direction, attribs.normal) > 0) {
		attribs.normal *= -1;
	}

	attribs.intersectionPoint = optix::make_float3(ray.origin.x + ray.tmax * ray.direction.x,
		ray.origin.y + ray.tmax * ray.direction.y,
		ray.origin.z + ray.tmax * ray.direction.z);

	attribs.vectorToLight = lightPossition - attribs.intersectionPoint;
}

RT_PROGRAM void primary_ray( void )
{

	const optix::float3 d_c = make_float3(launch_index.x -
		launch_dim.x * 0.5f, output_buffer.size().y * 0.5f -
		launch_index.y, -focal_length);
	const optix::float3 d_w = optix::normalize(M_c_w * d_c);
	optix::Ray ray(view_from, d_w, 0, 0.01f);

	PerRayData_radiance prd;
	curand_init(launch_index.x + launch_dim.x * launch_index.y, 0, 0, &prd.state);
	rtTrace( top_object, ray, prd );

	// access to buffers within OptiX programs uses a simple array syntax	
	output_buffer[launch_index] = optix::make_uchar4( prd.result.x*255.0f, prd.result.y*255.0f, prd.result.z*255.0f, 255 );
}

RT_PROGRAM void closest_hit_Phong( void )
{
	optix::float3 amb_occ = ambient_occlusion(ray_data.state);
	float ligth = optix::dot(optix::normalize(attribs.vectorToLight), attribs.normal);

	optix::float3 lr = 2 * (ligth)* attribs.normal - optix::normalize(attribs.vectorToLight);
	float shade = shadow_ray(attribs.vectorToLight);
	
	optix::float3 res = ambient + (getDiffuseColor() * ligth) + specular * pow(optix::dot(-ray.direction, lr), shininess);

	ray_data.result = res * amb_occ;

}

RT_PROGRAM void closest_hit_Normal(void)
{
	optix::float3 amb = ambient_occlusion(ray_data.state);
	ray_data.result = attribs.normal * amb * 0.5f;

}

RT_PROGRAM void closest_hit_Lambert(void)
{
	optix::float3 diff = getDiffuseColor();
	float ligth = optix::dot(optix::normalize(attribs.vectorToLight), attribs.normal);

	optix::float3 lr = 2 * (ligth)* attribs.normal - optix::normalize(attribs.vectorToLight);
	optix::float3 res = optix::fmaxf(0, ligth) * diff;
	float shade = shadow_ray(attribs.vectorToLight);
	optix::float3 amb = ambient_occlusion(ray_data.state);
	ray_data.result = res *amb;

}

RT_PROGRAM void any_hit(void)
{
	rtTerminateRay();
}

RT_PROGRAM void shader_hit(void)
{
	shadow_ray_data.visible = 0.0f;
	rtTerminateRay();
}

__device__ optix::float3 ambient_occlusion(curandState_t state)
{
	optix::float3 sum = optix::make_float3(0, 0, 0);
	int N = 32;
	for (int i = 0; i < N; i++)
	{
		float randomX = (float)curand_uniform(&state);
		float randomY = (float)curand_uniform(&state);

		optix::float3 omegai = SampleHemisphere(attribs.normal, randomX,randomY);
		float pdf = 1.0f/ (2* CUDART_PI_F);

		float shade = shadow_ray(omegai);

		optix::float3 whiteColor = optix::make_float3(1, 1, 1);
		
		sum += diffuse * shade * (optix::dot(attribs.normal, omegai) / pdf);
	}
	return sum/N;
}

/* may access variables declared with the rtPayload semantic in the same way as closest-hit and any-hit programs */
RT_PROGRAM void miss_program( void )
{
	ray_data.result = optix::make_float3( 0.0f, 0.0f, 0.0f );
}

RT_PROGRAM void exception( void )
{
	const unsigned int code = rtGetExceptionCode();
	rtPrintf( "Exception 0x%X at (%d, %d)\n", code, launch_index.x, launch_index.y );
	rtPrintExceptionDetails();
	output_buffer[launch_index] = uchar4{ 255, 0, 255, 0 };
}

__device__ optix::float3 getDiffuseColor()
{
	optix::float3 color;
	if (tex_diffuse_id != -1) {
		const optix::float4 value = optix::rtTex2D<optix::float4>(tex_diffuse_id, attribs.texcoord.x, 1 - attribs.texcoord.y);
		color = optix::make_float3(value.x, value.y, value.z);
	}
	else {
		color = diffuse;
	}

	return color;
}

__device__ float shadow_ray(optix::float3 dir)
{
	float L = L2Norm(attribs.vectorToLight);

	optix::Ray ray(attribs.intersectionPoint, dir, 1, 0.01f);

	PerRayData_shadow shadow_prd;
	shadow_prd.visible = 1.0f;
	rtTrace(top_object, ray, shadow_prd);

	return shadow_prd.visible;
}

__device__ float L2Norm(optix::float3 q)
{
	return sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
	
}

__device__ optix::float3 SampleHemisphere(optix::float3 normal, float randomX, float randomY)
{
	float x = 2 * cosf(2 * CUDART_PI_F * randomX) * sqrtf(randomY * (1 - randomY));
	float y = 2 * sinf(2 * CUDART_PI_F * randomX) * sqrtf(randomY * (1 - randomY));
	float z = 1 - 2 * randomY;
	optix::float3 omegaI = optix::make_float3( x, y, z );
	optix::normalize(omegaI);
	if (optix::dot(omegaI, normal) < 0) {
		omegaI *= -1;
	}

	return omegaI;
}

