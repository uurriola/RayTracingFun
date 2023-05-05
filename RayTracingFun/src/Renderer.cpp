#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
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
}

void Renderer::Render()
{
	// Render pixels
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float) x / (float) m_FinalImage->GetWidth(),  (float) y / (float) m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;
			PerPixel(coord);

			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}
	m_FinalImage->SetData(m_ImageData);

	m_Time += 0.05f;
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint32_t color = Walnut::Random::UInt();
	color |= 0xff000000;
	color = 0xff00ffff;

	// Coord z = -1 per definition
	glm::vec3 rayOrigin(0.0f, 0.0, 5.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);
	float radius = 2.0f;

	// Draw a sphere at 0,0
	// Compute rays from camera to pixel and check if it hit the sphere
	// Sphere equation is
	// (bx^2 + by^2 + bz^2) t^2 + 2 (axbx + ayby + azbz) t + (ax^2 + ay^2 + az^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// Compute discrimant b^2 - 4ac: if >= 0, we hit

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2 * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant >= 0)
	{
		// Light move on a circle at y = 5 and x^2 + z^2 = 5.0f
		glm::vec3 lightOrigin(5.0f * cosf(m_Time), 5.0, 5.0f * sinf(m_Time));

		// Compute point on sphere
		float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
		float t2 = (-b - sqrtf(discriminant)) / (2.0f * a);
		float t = (t1 > t2)? t1: t2;
		glm::vec3 point = rayOrigin + t * rayDirection;
		// Since sphere is centered on 0,0,0, point is also the normal
		glm::vec3 normal = glm::normalize(point);
		// Shade depends on normal dot (point to light) ray
		glm::vec3 lightToPoint = lightOrigin - point;
		lightToPoint = glm::normalize(lightToPoint);
		float pointExposition = glm::dot(lightToPoint, normal);
		pointExposition = (pointExposition < 0.1f) ? 0.1f : pointExposition;

		uint8_t r = (uint8_t)(225.0f) * pointExposition;
		return 0xff000000 | r;
	}
	else
	{
		return 0xff000000;
	}

	// uint8_t r = (uint8_t)(coord.x * 225.0f);
	// uint8_t g = (uint8_t)(coord.y * 225.0f);
	// return 0xff000000 | (g << 8) | r;
}
