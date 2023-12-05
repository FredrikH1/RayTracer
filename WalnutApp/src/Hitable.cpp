#include "Hitable.h"


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



float Triangle::Hit(const Ray& ray) const
{
	glm::mat3 mat{ vec1, vec2, -ray.Direction };

	glm::vec3 u1u2t = glm::inverse(mat) * (ray.Origin - Position);

	if (glm::all(glm::greaterThan(u1u2t, glm::vec3{ 0.0f })) && u1u2t[0] + u1u2t[1] < 1.0f)
		return u1u2t[2];

	return -1.0f;

}

glm::vec3 Triangle::GetNormal(const glm::vec3& hitPosition) const
{
	return glm::normalize(glm::cross(vec1,vec2));
}