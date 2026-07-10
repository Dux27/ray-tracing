#include "Renderer.hpp"

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage -> GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage -> GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage -> GetWidth(), (float)y / (float)m_FinalImage -> GetHeight() };
			m_ImageData[x + y * m_FinalImage -> GetWidth()] = PerPixel(coord);
		}
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

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	return CreateCircle(coord, {0.0, 0.0}, 0.4);
}

uint32_t Renderer::CreateCircle(glm::vec2 coord, glm::vec2 pos, float r)
{
	uint32_t empty = 0xff000000;
	uint32_t filled = 0xffff0000;

	float aspectRatio = float(m_FinalImage -> GetWidth()) / float(m_FinalImage -> GetHeight());

	glm::vec2 imageSize = {m_FinalImage -> GetWidth(), m_FinalImage -> GetHeight()};
	// -> normalization (percentage-based) is done in the main loop
	// -> multiply by the size of new range
	// -> substract to move to origin
	glm::vec2 uv = coord * 2.0f - 1.0f;

	uv -= pos;
	
	// Anchor y axis to [-1, 1]. Scale of x axis is dynamicly adjusted
	uv.x *= aspectRatio;
	
	if ((uv.x * uv.x) + (uv.y * uv.y) <= r * r)
	{
		return filled;
	}
	return empty;
}
