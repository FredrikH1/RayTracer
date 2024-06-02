#include "Renderer.h"
#include "Walnut/Random.h"
#include <limits>
#include <execution>

namespace Utils
{
	uint32_t toRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | (r);
	}

	static uint32_t PCG_Hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state << 28u) + 4u))^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed)
	{
		seed = PCG_Hash(seed);
		return (float)seed / (float)UINT32_MAX;
	}

	static glm::vec3 InUnitSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f)
		);
	}

	//Schlick approximation
	static float SchlickReflectance(float cosTheta, float refractionRatio)
	{
		auto r0 = (1 - refractionRatio) / (1 + refractionRatio);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosTheta), 5);
	}
}


void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		//No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	ResetFrameIndex();
	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;

}


void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, sizeof(glm::vec4) * m_FinalImage->GetWidth() * m_FinalImage->GetHeight());

#define MT 1

#if MT
	std::for_each(std::execution::par ,m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(), [this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);

					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::toRGBA(accumulatedColor);
				});
		});
#else

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float) m_FrameIndex;
			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::toRGBA(accumulatedColor);

		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];
	
	glm::vec3 light{ 0.0f };
	glm::vec3 contribution(1.0f);

	uint32_t seed = x + y * m_FinalImage->GetWidth();
	seed *= m_FrameIndex;

	for (int i = 0; i < m_Settings.nbrBounces; i++)
	{	
		seed += i;


		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0)//Miss
		{
			light += m_ActiveScene->BackgroundColor * contribution;
			break; 
		}
		
		const Hitable* hitable = nullptr;
		if (payload.ObjectIndex < m_ActiveScene->Spheres.size())
		{
			hitable = &m_ActiveScene->Spheres[payload.ObjectIndex];
			payload.uv = m_ActiveScene->Spheres[payload.ObjectIndex].GetUV(payload.WorldPosition);
		}	
		else
		{
			hitable = &m_ActiveScene->Triangles[payload.ObjectIndex - m_ActiveScene->Spheres.size()];
		}
	
		
		const Material& material = m_ActiveScene->Materials[hitable->MaterialIndex];
	

		light += material.GetEmission() * contribution;
		
		contribution *= material.texture->value(payload.uv[0], payload.uv[1]) * material.Albedo;


		float refractionRatio;
		float cosTheta = glm::dot(ray.Direction, payload.WorldNormal); //Not exactly cosTheta since ray.Direction might not be normalized, but good enough if material isn't dielectric.s
		if (cosTheta <= 0) // From which side did we hit the surface
		{
			refractionRatio = 1.0f/ material.refractiveIndex;
		}
		else
		{
			refractionRatio = material.refractiveIndex;
			payload.WorldNormal = -payload.WorldNormal; //Hit from inside (backside)
		}



		if (material.dielectric) //Evaluate refraction
		{
			ray.Direction = glm::normalize(ray.Direction);
			cosTheta = -glm::dot(ray.Direction, payload.WorldNormal);
			if (refractionRatio * glm::sqrt(1.0f - (cosTheta * cosTheta)) <= 1.0f && Utils::SchlickReflectance(cosTheta, refractionRatio) < Utils::RandomFloat(seed))
			{
				//Refract
				ray.Origin = payload.WorldPosition - payload.WorldNormal * 0.001f; // Bias to avoid colliding with own object, - to refrect through object
				ray.Direction = glm::refract(ray.Direction, payload.WorldNormal, refractionRatio);
				continue;
			}
			
		}

		{
			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.001f; // Bias to avoid colliding with own object
			glm::vec3 metallic = glm::reflect(ray.Direction, payload.WorldNormal) + Utils::InUnitSphere(seed) * material.Roughness;
			glm::vec3 diffuse = payload.WorldNormal + material.Roughness * Utils::InUnitSphere(seed); //Lambertian diffuse

			ray.Direction = diffuse * (1.0f - material.Metallic) + metallic * material.Metallic;
		}

		
	}
	
		
		return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{

	if (m_ActiveScene->Spheres.empty() && m_ActiveScene->Triangles.empty())
	{
		return Miss(ray);
	}

	float closestT = std::numeric_limits<float>::max();
	int closestHitable = -1;
	glm::vec2 uv;

	for (size_t i = 0; i< m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];

#define HIT 0

#if HIT

		float t = sphere.Hit(ray);

		if (t < closestT && t > 0)
		{
			closestT = t;
			closestHitable = (int)i;
		}
#else 

		glm::vec3 mid2orig = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(mid2orig, ray.Direction);
		float c = glm::dot(mid2orig, mid2orig) - (sphere.Radius * sphere.Radius);

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0) // No hit on sphere
		{
			continue;
		}
		//Closest hit
		float t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		

		if (t <= 0.0f) //Negative t => behind ray origin
		{
			//Check other solution, always further away
			t = (-b + glm::sqrt(b * b - 4.0f * a * c)) / (2.0f * a);
			if (t <= 0)
				continue;
		}
		if (t < closestT) 
		{
			closestT = t;
			closestHitable = (int)i;
		}

#endif
		
	}

	for (size_t i = 0; i < m_ActiveScene->Triangles.size(); i++)
	{
		const Triangle& triangle = m_ActiveScene->Triangles[i];

#if HIT

		float t = triangle.Hit(ray);

		if (t < closestT && t > 0)
		{
			closestT = t;
			closestHitable = (int)i + m_ActiveScene->Spheres.size();
		}

#else 

		glm::mat3 mat{ triangle.vec1, triangle.vec2, -ray.Direction };

		glm::vec3 uvt = glm::inverse(mat) * (ray.Origin - triangle.Position);

		float t;
		if (glm::all(glm::greaterThan(uvt, glm::vec3{ 0.0f })) && uvt[0] < 1.0f && uvt[1] < 1.0f)
			t = uvt[2];
		else
			continue;

		if (t < closestT)
		{
			closestT = t;
			closestHitable = (int)i + m_ActiveScene->Spheres.size();
			uv = glm::vec2(uvt);
		}

#endif
		

	}


	// No hit
	if (closestHitable < 0)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, closestT, closestHitable, uv);

}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex, glm::vec2 uv)
{	
	const Hitable* closestHitable = nullptr;
	if (objectIndex < m_ActiveScene->Spheres.size())
		closestHitable = &m_ActiveScene->Spheres[objectIndex];
	else
		closestHitable = &m_ActiveScene->Triangles[objectIndex - m_ActiveScene->Spheres.size()];

	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;
	payload.WorldPosition = ray.Origin + ray.Direction * hitDistance;
	payload.WorldNormal = closestHitable->GetNormal(payload.WorldPosition);
	payload.uv = uv;
	
	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}