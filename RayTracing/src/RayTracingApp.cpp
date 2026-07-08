#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "imgui.h"
#include <cstdint>
#include <memory>

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last reder time: %.3fms", m_LastRenderTimer);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::End();

		// Push to the internal state stack
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		// Display the image if its not null
		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), 
				     {(float)m_Image->GetWidth(), (float)m_Image->GetHeight()});

		ImGui::End();
		// Clear internal state stack at the end of every frame
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Walnut::Timer timer;

		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight())
		{
			m_Image = std::make_shared<Walnut::Image>(m_ViewportWidth, m_ViewportHeight,
					     			  Walnut::ImageFormat::RGBA);
			// Clear the old buffer; if its null it will do nothing
			delete[] m_ImageData;
			// Reallocating the pixel buffer to heap 
			m_ImageData = new uint32_t[m_ViewportHeight * m_ViewportWidth];
		}

		for (uint32_t i = 0; i < m_ViewportHeight * m_ViewportWidth; i++)
		{
			// RGBA is reversed: ABGR
			m_ImageData[i] = Walnut::Random::UInt();
			m_ImageData[i] |= 0xff000000;
		}
		
		// This is the bridge. It copies CPU-calculated pixels across the PCIe bus
		// into the GPU's memory so Vulkan can display it.
		m_Image->SetData(m_ImageData);

		m_LastRenderTimer = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Walnut::Image> m_Image;
	// Buffer of pixel data
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	
	float m_LastRenderTimer = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

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
