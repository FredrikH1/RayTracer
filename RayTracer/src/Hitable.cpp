#include "Hitable.h"


#define PI 3.14159265359

float Sphere::Hit(const Ray& ray) const
{
	glm::vec3 mid2orig = ray.Origin - Position;

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2 * glm::dot(mid2orig, ray.Direction);
	float c = glm::dot(mid2orig, mid2orig) - (Radius * Radius);

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0 || b > 0) // No hit on sphere or sphere behind camera
	{
		return -1.0f;
	}
	//Closest hit
	float t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	//Other solution always further away
	//float t = (-b + glm::sqrt(b * b - 4.0f * a * c)) / (2.0f * a);

	return t;
}

glm::vec3 Sphere::GetNormal(const glm::vec3& hitPosition) const
{
	return glm::normalize(hitPosition - Position);
}

glm::vec2 Sphere::GetUV(const glm::vec3& hitPosition) const
{
	glm::vec3 orig2hit = hitPosition - Position;

	float theta = acos(glm::dot(orig2hit, glm::vec3{0.0f,1.0f,0.0f}) / Radius);
	float phi = atan2(orig2hit[0], orig2hit[2]) + PI;

	glm::vec2 uv{ phi / (2 * PI), theta / PI };

	return uv;
}



float Triangle::Hit(const Ray& ray)
{
	glm::mat3 mat{ vec1, vec2, -ray.Direction };

	uvt = glm::inverse(mat) * (ray.Origin - Position);

	if (glm::all(glm::greaterThan(uvt, glm::vec3{ 0.0f })) && uvt[0] + uvt[1] < 1.0f)
		return uvt[2];		

	return -1.0f;

}

glm::vec3 Triangle::GetNormal(const glm::vec3& hitPosition) const
{
	return glm::normalize(glm::cross(vec1,vec2));
}

glm::vec2 Triangle::GetUV(const glm::vec3& hitPosition) const
{
	return glm::vec2(uvt[1] /glm::length(vec1), uvt[2] / glm::length(vec2));
}
