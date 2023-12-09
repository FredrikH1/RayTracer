#pragma once

#include "glm\glm.hpp"
#include <memory>
#include <vector>
#include <execution>


class Camera
{
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);
	void OnFOV();

	const glm::vec3& GetPosition() const { return m_Position; };
	const glm::vec3& GetDirection() const { return m_ForwardDirection; };
	const glm::vec3& GetRightDirection() const { return m_RightDirection; };
	float& GetVerticalFOV() { return m_VerticalFOV; };

	bool& GetCameraMode() { return m_CameraMode; };

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; };


private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();


private:
	glm::mat4 m_View{ glm::mat4(1.0f) };
	glm::mat4 m_InverseView{ glm::mat4(1.0f) };

	glm::mat4 m_Projection{ glm::mat4(1.0f) };
	glm::mat4 m_InverseProjection{ glm::mat4(1.0f) };

	//Cached sines and cosines for own Camera
	std::vector<float> m_HSines;
	std::vector<float> m_HCosines;
	std::vector<float> m_VSines;
	std::vector<float> m_VCosines;

	//Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec3 m_Position{0.0f,0.0f,0.0f};
	glm::vec3 m_ForwardDirection{ 0.0f,0.0f,1.0f };
	glm::vec3 m_UpDirection{ 0.0f,1.0f,0.0f };
	glm::vec3 m_RightDirection{ 1.0f,0.0f,0.0f };

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	float m_VerticalFOV = 45.0f;
	float m_Nearclip = 0.1f;
	float m_FarClip = 100.0f;
	float m_Speed = 3.0f;
	float m_RotationSpeed = 0.003f;

	bool m_CameraMode = 0; //0 = UpDirection constant, 1 = UpDirection Dynamic

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	glm::vec2 m_LastMousePosition{0.0f,0.0f};

};