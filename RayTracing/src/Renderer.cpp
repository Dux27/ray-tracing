#include "Renderer.hpp"

void Renderer::Render()
{
	for (uint32_t i = 0; i < m_FinalImage -> GetHeight() * m_FinalImage -> GetWidth(); i++)
	{
		int x = i % m_FinalImage -> GetWidth();
		int y = i / m_FinalImage -> GetWidth();
		m_ImageData[i] = CreateCircle(x, y, 0, 0, 0.4);
		
		// RGBA is reversed: ABGR
		// m_ImageData[i] = Walnut::Random::UInt();
		// m_ImageData[i] |= 0xff000000;
	}

	// This is the bridge. It copies CPU-calculated pixels across the PCIe bus
	// into the GPU's memory so Vulkan can display it.
	m_FinalImage->SetData(m_ImageData);
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage -> GetWidth() == width && m_FinalImage -> GetHeight() == height)
			return;

		// TODO: Write me own Resize() here
		m_FinalImage -> Resize(width, height);
	}
	else
	{
		// If there is no image create one
		m_FinalImage = std::make_shared<Walnut::Image>(width, height,
							  Walnut::ImageFormat::RGBA);
	}

	// If there is image and it has been resized delete old buffer and reallocate a new one with new size
	delete[] m_ImageData;
	// Reallocating the pixel buffer to heap 
	m_ImageData = new uint32_t[height * width];
}

uint32_t Renderer::CreateCircle(uint32_t x, uint32_t y, float a, float b, float r)
{
	uint32_t empty = 0xff000000;
	uint32_t filled = 0xffff0000;

	float aspect_ratio = float(m_FinalImage -> GetWidth()) / float(m_FinalImage -> GetHeight());

	// -> normalization (percentage-based) 
	// -> multiply by the size of new range
	// -> substract to move to origin
	float uv_x = (float(x) / m_FinalImage -> GetWidth()) * 2.0f - 1;
	float uv_y = (float(y) / m_FinalImage -> GetHeight()) * 2.0f - 1;

	uv_x -= a;
	uv_y -= b;
	
	// Anchor y axis to [-1, 1]. Scale of x axis is dynamicly adjusted
	uv_x *= aspect_ratio;
	
	if ((uv_x * uv_x) + (uv_y * uv_y) <= r * r)
	{
		return filled;
	}
	return empty;
}
