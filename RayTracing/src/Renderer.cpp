#include "Renderer.hpp"
#include "glm/geometric.hpp"

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// Conversion to uv: from [0, 1] to [-1, 1]
			// -> normalization (percentage-based)
			// -> multiply by the size of new range
			// -> substract to move to origin
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage -> GetHeight() };
			coord = coord * 2.0f - 1.0f;

			float aspectRatio = float(m_FinalImage -> GetWidth()) / float(m_FinalImage -> GetHeight());
			// Anchor y axis to [-1, 1]. Scale of x axis is dynamicly adjusted
			coord.x *= aspectRatio;

			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
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
	// Scalar broadcasting; the value will be broadcasted to all three axis
	// Origin at (0,0,0) means target screen coordinates double as the ray direction vectors
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	// rayDirection = glm::normalize(rayDirection);		// Transform to an unit vector
	
	return CreateSphere(rayDirection, {0.0, 0.0, 0.0}, 0.5f);
}

uint32_t Renderer::CreateSphere(glm::vec3 rayDirection, glm::vec3 pos, float radius)
{
	// Combines color channels into a single 32-bit integer layout:
	// It only works for coord in [0, 1] format 
	// 0x FF (Alpha) 00 (Blue) 00 (Green) 00 (Red)
	// - 0xff000000: Sets Alpha to 255 so the pixel is 100% opaque.
	// - (b << 16) : Shifts the Blue value 2 bytes over into the Blue slot.
	// - r         : Places the Red value directly into the lowest byte slot.
	//
	// uint8_t red_ch = (uint8_t)(rayDirection.x * 255.0f);
	// uint8_t blue_ch = (uint8_t)(rayDirection.y * 255.0f);
	// uint32_t filled = 0xff000000 | (blue_ch << 16) | red_ch;

	uint32_t empty = 0xff000000;

	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);

	// (x - a); (y - b); (z - c)
	glm::vec3 origin = rayOrigin - pos;
	
	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2)
	// where
	// a = ray origin
	// b = ray direction / step instruction
	// r = radius
	// t = hit distance / time (like one step per one time unit, so how many steps to hit)
	
	// a, b, c like in delta equation, not like in formula above
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - (radius * radius);

	// delta 
	float discriminant  = b * b - 4.0f * a * c;
	
	if (discriminant >= 0)
	{
		float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a); 

		// Depth based shading: closer hits (smaller t1) become brighter, farther hits fade to black.
		// 1. Invert distance so close = bright (1.0) and far = dark (0.0).
		// 2. Clamp between 0 and 1 to prevent color glitches.
		// 3. Scale to 0-255 for standard 8-bit color.
		float shadeScale = 0.4f;
		float depthFraction = glm::clamp(1.0f - (t1 * shadeScale), 0.0f, 1.0f);
		
		// Multiplying it by itself makes high values stay bright, and low values plunge to 0.0
		float intenseDepth = depthFraction * depthFraction;
		uint8_t depth = (uint8_t)(intenseDepth * 255.0f);

		return 0xff000000 | (depth << 16); 
	}

	return empty;
}
