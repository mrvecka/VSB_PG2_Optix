#include <optix_world.h>
#include <curand_kernel.h>
#include "math_constants.h"

__device__ optix::float3 getDiffuseColor();
__device__ float shadow_ray(optix::float3 dir);
__device__ optix::float3 SampleHemisphere(optix::float3 normal, float randomX, float randomY);
__device__ float L2Norm(optix::float3 q);
__device__ optix::float3 ambient_occlusion(curandState_t state);
__device__ optix::float3 orthogonal(const optix::float3 & v);

struct PerRayData_radiance
{
	optix::float3 result;
	float  importance;
	int depth;
	curandState_t state;
};

struct PerRayData_shadow
{
	optix::float3 attenuation;
	float visible;

};
