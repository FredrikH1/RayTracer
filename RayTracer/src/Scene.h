#pragma once
#include "Camera.h"
#include<memory>
#include<vector>
#include "Hitable.h"


struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Triangle> Triangles;
	std::vector<Material> Materials;
	std::vector<std::shared_ptr<Texture>> Textures;
	glm::vec3 LightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	glm::vec3 BackgroundColor{ 0.35f, 0.65f, 0.98f };
};


namespace scene 
{
	void SaveScene(const Scene& scene, const char* Directory);
	bool LoadScene(Scene& scene, const char* Directory);
}