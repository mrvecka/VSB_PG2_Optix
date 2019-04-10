#include <optix_world.h>
#include <curand_kernel.h>
#include "math_constants.h"

__device__ optix::float3 getDiffuseColor();
__device__ float shadow_ray(optix::float3 dir);
__device__ optix::float3 SampleHemisphere(optix::float3 normal);
__device__ float L2Norm(optix::float3 q);
__device__ float ambient_occlusion();

struct PerRayData_radiance
{
	optix::float3 result;
	float  importance;
	int depth;
};

struct PerRayData_shadow
{
	optix::float3 attenuation;
	float visible;

};
