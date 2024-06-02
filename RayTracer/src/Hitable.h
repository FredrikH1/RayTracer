#pragma once
#include <glm/glm.hpp>
#include "Ray.h"
#include "Texture.h"

struct Hitable
{
	uint32_t MaterialIndex = 0;

	glm::vec3 Position{ 0.0f };

	virtual glm::vec3 GetNormal(const glm::vec3& hitPosition) const = 0;
};

struct Sphere : Hitable
{
	float Radius = 0.5f;

	float Hit(const Ray& ray) const;
	glm::vec3 GetNormal(const glm::vec3& hitPosition) const override;

	glm::vec2 GetUV(const glm::vec3& hitPosition) const;
};

struct Triangle : Hitable
{
	glm::vec3 vec1{ 1.0f, 0.0f, 0.0f };
	glm::vec3 vec2{ 0.0f, -1.0f,0.0f };
	glm::vec3 uvt{ 0.0f };

	float Hit(const Ray& ray);
	glm::vec3 GetNormal(const glm::vec3& hitPosition) const override;

	glm::vec2 GetUV(const glm::vec3& hitPosition) const;
};


struct Material
{

	glm::vec3 Albedo{ 1.0f };
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(); //= std::make_shared<Texture>((const char*)"earthmap10k.jpg");

	float Metallic = 0.0f;
	float Roughness = 1.0f;

	glm::vec3 EmissionColor{ 0.0f };
	float EmissionPower = 0.0f;

	bool dielectric = false;
	float refractiveIndex = 1.3f;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; };
};