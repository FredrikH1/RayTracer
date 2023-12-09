#include "Camera.h"
#include "Walnut\Input\Input.h"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
{
	m_VerticalFOV = verticalFOV;
	m_Nearclip = nearClip;
	m_FarClip = farClip;
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 3);
}

bool Camera::OnUpdate(float ts)
{
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = mousePos - m_LastMousePosition;
	m_LastMousePosition = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right))
	{
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}


	Input::SetCursorMode(CursorMode::Locked);

	bool moved = false;

	if (m_CameraMode) //Allow rotation of camera
	{

		if (Input::IsKeyDown(KeyCode::Q))
		{
			float rotDelta = ts * m_Speed;
			glm::quat q = glm::normalize(glm::angleAxis(rotDelta, m_ForwardDirection));
			m_UpDirection = glm::rotate(q, m_UpDirection);
			moved = true;
		}
		else if (Input::IsKeyDown(KeyCode::E))
		{
			float rotDelta = - ts * m_Speed;
			glm::quat q = glm::angleAxis(rotDelta, m_ForwardDirection);
			m_UpDirection = glm::rotate(q, m_UpDirection);
			moved = true;
		}
	}
	else
	{
		m_UpDirection = { 0.0f, 1.0f, 0.0f };
	}

	m_RightDirection = glm::cross(m_ForwardDirection,m_UpDirection);

	//Movement

	if (Input::IsKeyDown(KeyCode::W))
	{
		m_Position += ts * m_Speed * m_ForwardDirection;
		moved = true;
	}

	else if (Input::IsKeyDown(KeyCode::S))
	{
		m_Position -= ts * m_Speed * m_ForwardDirection;
		moved = true;
	}

	if (Input::IsKeyDown(KeyCode::A))
	{
		m_Position -= ts * m_Speed * m_RightDirection;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::D))
	{
		m_Position += ts * m_Speed * m_RightDirection;
		moved = true;
	}

	if (Input::IsKeyDown(KeyCode::LeftControl))
	{
		m_Position -= ts * m_Speed * m_UpDirection;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::Space))
	{
		m_Position += ts * m_Speed * m_UpDirection;
		moved = true;
	}


	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * m_RotationSpeed;
		float yawDelta = delta.x * m_RotationSpeed;

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, m_RightDirection), glm::angleAxis(-yawDelta, m_UpDirection)));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);
		m_UpDirection = glm::rotate(q, m_UpDirection);


		moved = true;
	}

	if (moved)
	{	
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;

}

void Camera::OnResize(uint32_t width, uint32_t height)
{

	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

void Camera::OnFOV()
{
	RecalculateProjection();
	RecalculateRayDirections();
}


#define CAMERA_OWN 1
#define CAMERA_MT 1


#if CAMERA_OWN

void Camera::RecalculateProjection()
{

	m_ImageHorizontalIter.resize(m_ViewportWidth);
	m_ImageVerticalIter.resize(m_ViewportHeight);
	for (uint32_t i = 0; i < m_ViewportWidth; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < m_ViewportHeight; i++)
		m_ImageVerticalIter[i] = i;

#define DEG2RAD 0.01745329251

	float aspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
	float horizontalFOV = 0.5f * aspectRatio * m_VerticalFOV * DEG2RAD;
	float verticalFOV = 0.5f * m_VerticalFOV * DEG2RAD;

	m_HSines.resize(m_ViewportWidth);
	m_HCosines.resize(m_ViewportWidth);
	m_VSines.resize(m_ViewportHeight);
	m_VCosines.resize(m_ViewportHeight);
	std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
		[this, horizontalFOV](uint32_t i) {
			float coord = ((float)i / (float)m_ViewportWidth)*2.0f -1.0f;
			m_HSines[i] = sin(horizontalFOV * coord);
			m_HCosines[i] = cos(horizontalFOV * coord);
		}
	);

	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this, verticalFOV](uint32_t i) {
			float coord = ((float)i / (float)m_ViewportHeight) * 2.0f - 1.0f;
			m_VSines[i] = sin(verticalFOV * coord);
			m_VCosines[i] = cos(verticalFOV * coord);
		}
	);

}

void Camera::RecalculateView()
{
	//m_View = glm::mat4(glm::vec4(m_RightDirection, 0.0f), glm::vec4(m_UpDirection, 0.0f), glm::vec4(m_ForwardDirection, 0.0f), glm::vec4(0.0f));

}


void Camera::RecalculateRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y) {
			for (uint32_t x = 0; x < m_ViewportWidth; x++)
			{
	
				glm::vec3 upDirection = glm::cross(-m_ForwardDirection, m_RightDirection);

				glm::vec3 rayDirection = m_HSines[x]*m_RightDirection + m_HCosines[x]*(m_VSines[y] * upDirection  + m_VCosines[y] * m_ForwardDirection);
				
				m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
			}
		}
	);
}


#elif CAMERA_MT


void Camera::RecalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_Nearclip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);

	m_ImageHorizontalIter.resize(m_ViewportWidth);
	m_ImageVerticalIter.resize(m_ViewportHeight);
	for (uint32_t i = 0; i < m_ViewportWidth; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < m_ViewportHeight; i++)
		m_ImageVerticalIter[i] = i;
}

void Camera::RecalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, m_UpDirection);
	m_InverseView = glm::inverse(m_View);

}

void Camera::RecalculateRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y) {
			for (uint32_t x = 0; x < m_ViewportWidth; x++)
			{
				glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
				coord = coord * 2.0f - 1.0f; // -1 -> 1
				glm::vec4 target = m_InverseProjection * glm::vec4(coord, 1, 1);
				glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
				m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
			}
		}
	);
}

#else


void Camera::RecalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_Nearclip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, m_UpDirection);
	m_InverseView = glm::inverse(m_View);

}

void Camera::RecalculateRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f; // -1 -> 1
			glm::vec4 target = m_InverseProjection * glm::vec4(coord, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}


#endif