#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "Camera.h"

#include "Scene.h"
#include <vector>

#include <imgui_internal.h>


using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) {}

	virtual void OnUpdate(float ts) override
	{
		Timer timer;

		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();

		m_LastCameraTime = timer.ElapsedMillis();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Last camera: %.3fms", m_LastCameraTime);

		if (ImGui::DragFloat("FOV", &m_Camera.GetVerticalFOV(), 1.0f, 30.0f, 100.0f))
		{
			m_Camera.OnFOV();
		}

		glm::vec3 Position = m_Camera.GetPosition();
		ImGui::Text("Position: x: %0.3f, y: %0.3f, z: %0.3f", Position.x, Position.y, Position.z);
		
		glm::vec3 forwardDirection = m_Camera.GetDirection();
		ImGui::Text("ForwardDirection: x: %0.3f, y: %0.3f, z: %0.3f", forwardDirection.x, forwardDirection.y, forwardDirection.z );

		glm::vec3 rightDirection = m_Camera.GetRightDirection();
		ImGui::Text("RightDirection: x: %0.3f, y: %0.3f, z: %0.3f", rightDirection.x, rightDirection.y, rightDirection.z);

		ImGui::Checkbox("Free Camera Rotation", &m_Camera.GetCameraMode());
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
		ImGui::DragInt("Number of Bounces", &m_Renderer.GetSettings().nbrBounces, 0.01f, 1, 10);


		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}


		ImGui::End();


		ImGui::Begin("Materials");

		int i = 0;
		if (m_Scene.Materials.empty())
			m_Scene.Materials.push_back(Material{});


		for (Material& material : m_Scene.Materials)
		{
			ImGui::PushID(i);
			ImGui::Text("Material %u", i);
			if (ImGui::Button("Delete Material"))
			{
				m_Scene.Materials.erase(m_Scene.Materials.begin() + i);
				for (Sphere& sphere : m_Scene.Spheres)
				{
					if (sphere.MaterialIndex == i)
						sphere.MaterialIndex = 0;
				}

				for (int k = i + 1; k < m_Scene.Spheres.size(); k++)
				{
					Sphere& sphere = m_Scene.Spheres[k];
					if (sphere.MaterialIndex != 0)
						sphere.MaterialIndex--;

				}
			}
			
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);
			ImGui::Checkbox("Dielectric", &material.dielectric);
			if (material.dielectric)
			{
				ImGui::DragFloat("Refractive Index", &material.refractiveIndex, 0.01f, 1.0f, 100.0f);
			}
			ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.1f, 0.0f, FLT_MAX);

			i++;
			ImGui::Separator();
			ImGui::PopID();
		}

		if (ImGui::Button("New Material"))
		{
			m_Scene.Materials.push_back(Material{});
		}
		ImGui::End();


		ImGui::Begin("Scene");
		//Save and Load Scene
		constexpr auto dir = "../Scenes/scene.bin";
		if (ImGui::Button("Save Scene"))
		{
			scene::SaveScene(m_Scene, dir);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Scene"))
		{
			if (scene::LoadScene(m_Scene, dir))
			{
				m_Renderer.ResetFrameIndex();
			}
		}

		ImGui::Separator();

		ImGui::DragFloat3("Light Direction", glm::value_ptr(m_Scene.LightDir), 0.1f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Background Color", glm::value_ptr(m_Scene.BackgroundColor));

		ImGui::Separator();
		ImGui::Text("Spheres");

		i = 0;
		for (Sphere& sphere : m_Scene.Spheres)
		{	
			ImGui::PushID(i); 
			ImGui::Text("Sphere %u", i +1);
			if (ImGui::Button("Delete Sphere"))
			{
				m_Scene.Spheres.erase(m_Scene.Spheres.begin() + i);
			}
			ImGui::DragFloat3("Sphere Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Sphere Radius", &sphere.Radius, 0.1f, 0.0f, 10.0f);

			ImGuiSliderFlags flags = (0 == m_Scene.Materials.size() - 1) ? ImGuiSliderFlags_ReadOnly : 0;
			ImGui::DragInt("Material Index", (int*) &sphere.MaterialIndex, 0.1f, 0, m_Scene.Materials.size() - 1, "%d", flags);

			i++;
			ImGui::Separator();
			ImGui::PopID();
		}
		if (ImGui::Button("New Sphere"))
		{
			m_Scene.Spheres.push_back(Sphere{});
		}

		ImGui::Separator();
		ImGui::Separator();

		ImGui::Text("Triangles");

		//i = 0;
		for (Triangle& triangle : m_Scene.Triangles)
		{
			ImGui::PushID(i);
			ImGui::Text("Triangle %u", i + 1);
			if (ImGui::Button("Delete Triangle"))
			{
				m_Scene.Triangles.erase(m_Scene.Triangles.begin() + i);
			}
			ImGui::DragFloat3("Triangle Position", glm::value_ptr(triangle.Position), 0.1f);
			ImGui::DragFloat3("Triangle Vec1", glm::value_ptr(triangle.vec1), 0.1f);
			ImGui::DragFloat3("Triangle Vec2", glm::value_ptr(triangle.vec2), 0.1f);
			

			ImGuiSliderFlags flags = (0 == m_Scene.Materials.size() - 1) ? ImGuiSliderFlags_ReadOnly : 0;
			ImGui::DragInt("Material Index", (int*)&triangle.MaterialIndex, 0.1f, 0, m_Scene.Materials.size() - 1, "%d", flags);

			i++;
			ImGui::Separator();
			ImGui::PopID();
		}
		if (ImGui::Button("New Triangle"))
		{
			m_Scene.Triangles.push_back(Triangle{});
		}


		ImGui::End();



		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if(image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0)); 


		ImGui::End();
		ImGui::PopStyleVar();

		Render(); // Render every frame
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;

	float m_LastCameraTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Raytracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	//app->SetMenubarCallback([app]()
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Exit"))
	//		{
	//			app->Close();
	//		}
	//		ImGui::EndMenu();
	//	}
	//});
	return app;
}