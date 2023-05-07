#include "Renderer.h"

#include "Walnut/Random.h"


namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}


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
	m_AspectRatio = width / (float) height;
}

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();
	// Render pixels
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			uint32_t pixelCoordinate = x + y * m_FinalImage->GetWidth();
			ray.Direction = camera.GetRayDirections()[pixelCoordinate];
			glm::vec4 color = TraceRay(ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[pixelCoordinate] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);

	m_Time += 0.02f;
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	// Normalize here has an impact of performances
	// rayDirection = glm::normalize(rayDirection);
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

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2 * glm::dot(ray.Origin, ray.Direction);
	float c = glm::dot(ray.Origin, ray.Origin) - radius * radius;

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// Light move on a circle at y = 5 and x^2 + z^2 = 5.0f
	glm::vec3 lightOrigin(5.0f * cosf(m_Time), 5.0, 5.0f * sinf(m_Time));

	// Compute point on sphere
	// float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
	float t = (-b - sqrtf(discriminant)) / (2.0f * a);
	// float t = (t1 < t2)? t1: t2;
	glm::vec3 point = ray.Origin + t * ray.Direction;
	// Since sphere is centered on 0,0,0, point is also the normal
	// glm::vec3 normal = glm::normalize(point);
	glm::vec3 normal = point / radius;

	// Shade depends on normal dot (point to light) ray
	glm::vec3 lightToPoint = lightOrigin - point;
	lightToPoint = glm::normalize(lightToPoint);
	float pointExposition = glm::max(glm::dot(lightToPoint, normal), 0.05f);

	uint8_t r = pointExposition;
	return glm::vec4(pointExposition, 0.0f, 0.0f, 1.0f);
}
