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

void Renderer::Render(const Scene& scene, const Camera& camera)
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
			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[pixelCoordinate] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);

	m_Time += 0.02f;
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	if (scene.Spheres.size() == 0)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.Spheres)
	{
		float radius = sphere.Radius;
		glm::vec3 sphereCenter = sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		glm::vec3 cc = ray.Origin - sphereCenter;
		float b = 2 * glm::dot(cc, ray.Direction);
		float c = glm::dot(cc, cc) - radius * radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0)
			continue;

		// Compute point on sphere
		// float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
		float t = (-b - sqrtf(discriminant)) / (2.0f * a);
		if (t > 0 && t < hitDistance)
		{
			hitDistance = t;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// Light move on a circle at y = 5 and x^2 + z^2 = 5.0f
	glm::vec3 lightDirection = glm::normalize(glm::vec3(- 1.0f, -1.0f, -1.0f));

	glm::vec3 point = ray.Origin + hitDistance * ray.Direction;
	glm::vec3 normal = point - closestSphere->Position;
	normal = glm::normalize(normal);
	// normal = point / closestSphere->Radius;

	// Shade depends on normal dot (point to light) ray
	// glm::vec3 lightToPoint = lightDirection;  // lightOrigin - point;
	// lightToPoint = glm::normalize(lightToPoint);
	float pointExposition = glm::max(glm::dot(-lightDirection, normal), 0.05f);

	return glm::vec4(pointExposition * closestSphere->Albedo, 1.0f);
}
