#pragma once

#include "Walnut/Image.h"
#include "Scene.h"
#include "Ray.h"
#include "Scene.h"
#include "Hitable.h"
#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
		int nbrBounces = 10;
	};
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene,const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage;};

	Settings& GetSettings() { return m_Settings; };

	void ResetFrameIndex() { m_FrameIndex = 1; };

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;
		glm::vec2 uv; //uv texture coordinates

		int ObjectIndex;
	};

private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y); //RayGen
	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex, glm::vec2 uv);
	HitPayload Miss(const Ray& ray);
private: 
	std::shared_ptr<Walnut::Image> m_FinalImage;
	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;

	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_FrameIndex = 1;
};
