// CIS565 CUDA Raytracer: A parallel raytracer for Patrick Cozzi's CIS565: GPU Computing at the University of Pennsylvania
// Written by Yining Karl Li, Copyright (c) 2012 University of Pennsylvania
// This file includes code from:
// Yining Karl Li's TAKUA Render, a massively parallel pathtracing renderer: http://www.yiningkarlli.com

#ifndef INTERACTIONS_H
#define INTERACTIONS_H

#include "intersections.h"

struct Fresnel {
  float reflectionCoefficient;
  float transmissionCoefficient;
};

struct AbsorptionAndScatteringProperties{
    glm::vec3 absorptionCoefficient;
    float reducedScatteringCoefficient;
};

enum BSDFRET{DIFFUSE_SCATTER, REFLECTED, TRANSMITTED};


//forward declaration
__host__ __device__ bool calculateScatterAndAbsorption(ray& r, float& depth, AbsorptionAndScatteringProperties& currentAbsorptionAndScattering, glm::vec3& unabsorbedColor, material m, float randomFloatForScatteringDistance, float randomFloat2, float randomFloat3);
__host__ __device__ glm::vec3 getRandomDirectionInSphere(float xi1, float xi2);
__host__ __device__ glm::vec3 calculateTransmission(glm::vec3 absorptionCoefficient, float distance);
__host__ __device__ Fresnel calculateFresnel(glm::vec3 normal, glm::vec3 incident, float incidentIOR, float transmittedIOR, glm::vec3 reflectionDirection, glm::vec3 transmissionDirection);
__host__ __device__ glm::vec3 calculateReflectionDirection(glm::vec3 normal, glm::vec3 incident);
__host__ __device__ Fresnel calculateFresnel(glm::vec3 normal, glm::vec3 incident, float incidentIOR, float transmittedIOR, glm::vec3 reflectionDirection, glm::vec3 transmissionDirection);
__host__ __device__ glm::vec3 calculateRandomDirectionInHemisphere(glm::vec3 normal, float xi1, float xi2);

//TODO (OPTIONAL): IMPLEMENT THIS FUNCTION
__host__ __device__ glm::vec3 calculateTransmission(glm::vec3 absorptionCoefficient, float distance) {
  return glm::vec3(0,0,0);
}

//TODO (OPTIONAL): IMPLEMENT THIS FUNCTION
__host__ __device__ bool calculateScatterAndAbsorption(ray& r, float& depth, AbsorptionAndScatteringProperties& currentAbsorptionAndScattering,
                                                        glm::vec3& unabsorbedColor, material m, float randomFloatForScatteringDistance, float randomFloat2, float randomFloat3){
  return false;
}

//TODO (OPTIONAL): IMPLEMENT THIS FUNCTION
__host__ __device__ glm::vec3 calculateTransmissionDirection(glm::vec3 normal, glm::vec3 incident, float incidentIOR, float transmittedIOR) {
  return glm::vec3(0,0,0);
}

//TODO (OPTIONAL): IMPLEMENT THIS FUNCTION
__host__ __device__ glm::vec3 calculateReflectionDirection(glm::vec3 normal, glm::vec3 incident) {
 
  return glm::normalize(incident - 2.0f * normal * glm::dot(incident, normal));
}

//TODO (OPTIONAL): IMPLEMENT THIS FUNCTION
__host__ __device__ Fresnel calculateFresnel(glm::vec3 normal, glm::vec3 incident, float incidentIOR, float transmittedIOR, glm::vec3 reflectionDirection, glm::vec3 transmissionDirection) {
  Fresnel fresnel;

  fresnel.reflectionCoefficient = 1;
  fresnel.transmissionCoefficient = 0;
  return fresnel;
}

//LOOK: This function demonstrates cosine weighted random direction generation in a sphere!
__host__ __device__ glm::vec3 calculateRandomDirectionInHemisphere(glm::vec3 normal, float xi1, float xi2) {
    
    //crucial difference between this and calculateRandomDirectionInSphere: THIS IS COSINE WEIGHTED!
    
    float up = sqrt(xi1); // cos(theta)
    float over = sqrt(1 - up * up); // sin(theta)
    float around = xi2 * TWO_PI;
    
    //Find a direction that is not the normal based off of whether or not the normal's components are all equal to sqrt(1/3) or whether or not at least one component is less than sqrt(1/3). Learned this trick from Peter Kutz.
    
    glm::vec3 directionNotNormal;
    if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
      directionNotNormal = glm::vec3(1, 0, 0);
    } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
      directionNotNormal = glm::vec3(0, 1, 0);
    } else {
      directionNotNormal = glm::vec3(0, 0, 1);
    }
    
    //Use not-normal direction to generate two perpendicular directions
    glm::vec3 perpendicularDirection1 = glm::normalize(glm::cross(normal, directionNotNormal));
    glm::vec3 perpendicularDirection2 = glm::normalize(glm::cross(normal, perpendicularDirection1));
    
    return ( up * normal ) + ( cos(around) * over * perpendicularDirection1 ) + ( sin(around) * over * perpendicularDirection2 );
}

//TODO: IMPLEMENT THIS FUNCTION
//Now that you know how cosine weighted direction generation works, try implementing non-cosine (uniform) weighted random direction generation.
//This should be much easier than if you had to implement calculateRandomDirectionInHemisphere.
//Note that here, xi1 and xi2 are random numbers in the range of (0,1)
//http://mathworld.wolfram.com/SpherePointPicking.html
__host__ __device__ glm::vec3 getRandomDirectionInSphere(float xi1, float xi2) 
{
	float theta = 2.0f * PI * xi1;
	float phi = acosf(2 * xi2 - 1);

	float x = sin(phi) * sin(theta);
	float y = sin(phi) * cos(theta);
	float z = cos(phi);

	return glm::normalize(glm::vec3(x,y,z));
}

//TODO (PARTIALLY OPTIONAL): IMPLEMENT THIS FUNCTION
//returns 0 if diffuse scatter, 1 if reflected, 2 if transmitted.
__host__ __device__ int calculateBSDF(ray& r, glm::vec3 intersect, glm::vec3 normal, glm::vec3 emittedColor,
                                       AbsorptionAndScatteringProperties& currentAbsorptionAndScattering,
                                       glm::vec3& color, glm::vec3& unabsorbedColor, material m)
{
	return 0;
};

//TODO (PARTIALLY OPTIONAL): IMPLEMENT THIS FUNCTION
//returns 0 if diffuse scatter, 1 if reflected, 2 if transmitted.
__host__ __device__ int calculateBSDF(ray& r, const glm::vec3& intersect, const glm::vec3& normal, 
									  glm::vec3& color, const material& m, const float seed)
{
	float eps = 0.01;

	if (m.hasReflective != 0)
	{
		// perfect reflect
		glm::vec3 reflectedRay = calculateReflectionDirection(normal, r.direction);		
		r.direction = reflectedRay;
		r.origin = intersect + normal * eps;
		color = m.color;

		return BSDFRET::REFLECTED;

	}
	else if (m.hasRefractive > 0)
	{
		// check if we are within the object or not
		bool inObj = false;
		glm::vec3 isectNormal = normal;
		if (glm::dot(r.direction, normal) > 0.0f)
		{
			isectNormal = -1.0f * normal;
			inObj = true;
		}
		

		float airIOR = 1.0;
		float materialIOR = m.indexOfRefraction;

		glm::vec3 incidentDirection = r.direction;
		glm::vec3 refractedDirection = glm::vec3(0,0,0);

		float theta = 0.0f;

		if (inObj)
			theta = materialIOR / airIOR;
		else
			theta = airIOR / materialIOR;

		float cosTheta1 = glm::dot(-1.0f * incidentDirection, isectNormal);
		float cosTheta2 = 1.0f - theta * theta * (1.0f - cosTheta1 * cosTheta1);

		refractedDirection = glm::refract(incidentDirection, isectNormal, theta);
		r.direction = glm::normalize(refractedDirection);
		r.origin = intersect + r.direction * eps;
		color = m.specularColor;
		return BSDFRET::TRANSMITTED;
	}
	else
	{
		// diffuse scatter -- assume seed is already hashed
		//thrust::default_random_engine rng(hash((int)seed));
		thrust::default_random_engine rng(seed);
		thrust::uniform_real_distribution<float> u01(0,1);

		glm::vec3 direction = glm::normalize(calculateRandomDirectionInHemisphere(normal, (float)u01(rng), (float)u01(rng)));

		r.direction = direction;
		r.origin = intersect + r.direction * eps;

		color = m.color;

		return BSDFRET::DIFFUSE_SCATTER;
	}

};

#endif
