#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f)
	{
		{
			Material mat;
			mat.Albedo = { 1.0f, 0.0f, 0.0f };
			mat.Roughness = 1.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 1.0f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 1.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 0.0f, 0.0f, 1.0f };
			mat.Roughness = 0.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.7f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 1.0f, 0.0f, 1.0f };
			mat.Roughness = 0.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.7f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 0.0f, 1.0f, 0.3f };
			mat.Roughness = 0.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.7f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 0.8f, 0.2f, 0.3f };
			mat.Roughness = 1.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.0f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 0.2f, 0.8f, 0.3f };
			mat.Roughness = 1.0f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.0f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		{
			Material mat;
			mat.Albedo = { 0.4f, 0.2f, 0.5f };
			mat.Roughness = 0.7f;
			mat.Metallic = 0.0f;
			mat.Specular = 0.5f;
			mat.EmissionColor = { 1.0f, 1.0f, 1.0f };
			mat.EmissionStrength = 0.0f;
			m_Scene.Materials.push_back(mat);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.0f, 23.0f, -5.0f };
			sphere.Radius = 20.0f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { -5.0f,0.0f, -5.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 6;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { -2.5f,0.0f, -5.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f,0.0f, -5.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 2;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 2.5f,0.0f, -5.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 3;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 5.0f,0.0f, -5.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 5;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f, -502.0f, -5.0f };
			sphere.Radius = 500.0f;
			sphere.MaterialIndex = 4;
			m_Scene.Spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
			Render();
		
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();
		ImGui::End();

		ImGui::Begin("Scene");
		if (ImGui::Button("Add sphere"))
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}
		for (int i = 0; i < m_Scene.Spheres.size(); i++)
		{
			// ImGui requires ID to differentiate controls with the same name
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f, 0.0f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);
			ImGui::Separator();

			ImGui::PopID();
		}
		if (ImGui::Button("Add material"))
		{
			Material mat;
			mat.Albedo = { 1.0f, 0.0f, 0.0f };
			mat.Roughness = 1.0f;
			mat.Metallic = 0.0f;
			m_Scene.Materials.push_back(mat);
		}
		for (int i = 0; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);
			Material& mat= m_Scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.Albedo));
			ImGui::DragFloat("Roughness", &mat.Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &mat.Metallic, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Specular", &mat.Specular, 0.01f, 0.0f, 1.0f);
			ImGui::Checkbox("Refract", &mat.Refract);
			if (mat.Refract)
				ImGui::DragFloat("Refraction index", &mat.RefractionIndex, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("Emission color", glm::value_ptr(mat.EmissionColor));
			ImGui::DragFloat("Emission strength", &mat.EmissionStrength, 0.01f, 0.0f, 1.0f);
			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
		
		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();
		Render();
	}

	void Render()
	{
		Timer timer;

		// Renderer resize
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		// Renderer render
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Camera m_Camera;
	Renderer m_Renderer;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing Fun";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}