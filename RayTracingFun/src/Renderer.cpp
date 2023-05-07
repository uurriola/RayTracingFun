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
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	// Render pixels
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);

	m_Time += 0.02f;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	// Light move on a circle at y = 5 and x^2 + z^2 = 5.0f
	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	glm::vec3 color(0.0f);
	float colorMultiplier = 1.0f;

	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			color += glm::vec3(0.0f, 0.0f, 0.0f) * colorMultiplier;
			break;
		}

		// Shade depends on normal dot (point to light) ray
		// glm::vec3 lightToPoint = lightDirection;  // lightOrigin - point;
		// lightToPoint = glm::normalize(lightToPoint);
		float pointExposition = glm::max(glm::dot(-lightDirection, payload.WorldNormal), 0.05f);
		glm::vec3 sphereColor = m_ActiveScene->Spheres[payload.ObjectIndex].Albedo;
		color += pointExposition * sphereColor * colorMultiplier;

		// Blend rebound with current color
		colorMultiplier *= 0.7f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
	}

	return glm::vec4(color, 1.0f);

}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int hitObjectIndex = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i ++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
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
			hitObjectIndex = i;
		}
	}

	if (hitObjectIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, hitObjectIndex);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];
	payload.WorldPosition = ray.Origin + hitDistance * ray.Direction;
	payload.WorldNormal = glm::normalize(payload.WorldPosition - closestSphere.Position);
	// normal = point / closestSphere->Radius;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
