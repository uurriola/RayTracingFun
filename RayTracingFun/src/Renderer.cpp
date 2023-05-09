#include "Renderer.h"

#include "Walnut/Random.h"

#include <execution>
#include <random>


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
	
	static float RandomFloat()
	{
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	static glm::vec2 RandomUnitPointOnCircle()
	{
		glm::vec2 randomVec(2.0f);
		while (glm::dot(randomVec, randomVec) >= 1.0f)
			randomVec = glm::vec2(RandomFloat(), RandomFloat());
		return glm::normalize(randomVec);
	}

	static glm::vec3 RandomUnitVector()
	{
		glm::vec3 randomVec(2.0f);
		while (glm::dot(randomVec, randomVec) >= 1.0f)
			randomVec = Walnut::Random::Vec3(-1.0f, 1.0f);
		return glm::normalize(randomVec);
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
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
	ResetFrameIndex();

	m_ImageHorizontalIterator.resize(width);
	m_ImageVerticalIterator.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIterator[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIterator[i] = i;
}


void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(),
			[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
					finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(finalColor);
				}
			);
		}
	);
#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
			finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(finalColor);
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;

	uint32_t rayCountPerPixel = 2;
	float blurRatio = 0.001f;
	float focusRatio = 0.01f;
	glm::vec3 incomingLight{ 0.0f };
	for (uint32_t i = 0; i < rayCountPerPixel; i++)
	{
		glm::vec4 randomDelta = glm::vec4(focusRatio * Utils::RandomUnitPointOnCircle(), 0.0f, 1.0f);
		glm::vec3 deltaOrigin = m_ActiveCamera->GetInverseProjection() * randomDelta;
		ray.Origin = m_ActiveCamera->GetPosition() + deltaOrigin;
		// ray.Origin += glm::vec3(randomDirection.x, randomDirection.y, randomDirection.z);

		glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
		coord = coord * 2.0f - 1.0f; // -1 -> 1

		glm::vec2 randomDirection = blurRatio * Utils::RandomUnitPointOnCircle();
		glm::vec4 target = m_ActiveCamera->GetInverseProjection() * glm::vec4(coord.x + randomDirection.x, coord.y + randomDirection.y, 1, 1);
		ray.Direction = glm::vec3(m_ActiveCamera->GetInverseView() * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space

		// glm::vec2 randomDirection = Utils::RandomUnitPointOnCircle();
		// ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()] + glm::vec3(randomDirection, 0.0f);
		incomingLight += TracePath(ray);
	}

	return glm::vec4(incomingLight / (float) rayCountPerPixel, 1.0f);

}


glm::vec3 Renderer::TracePath(Ray& ray)
{
	glm::vec3 skyColor = glm::vec3(0.1f, 0.1f, 0.5f);

	glm::vec3 incomingLight(0.0f);
	glm::vec3 rayColor(1.0f);

	Renderer::HitPayload payload;

	int bounces = 8;
	for (int i = 0; i < bounces; i++)
	{
		TraceRay(ray, payload);

		if (payload.ObjectIndex < 0)
		{
			// rayColor *= skyColor;
			incomingLight += skyColor * rayColor;
			break;
		}

		// Shade depends on normal dot (point to light) ray
		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		Material mat = m_ActiveScene->Materials[sphere.MaterialIndex];

		glm::vec3 emittedLight = mat.EmissionColor * mat.EmissionStrength;
		glm::vec3 rayColorMultiplier{ 1.0f };

		// Compute next ray
		bool reflect = !mat.Refract;
		float r = 0.0001f;
		float refractionIndex = mat.RefractionIndex;
		if (mat.Refract)
		{
			if (glm::dot(ray.Direction, payload.WorldNormal) < 0)
				r *= -1.0;
			else
				refractionIndex = 1.0f / mat.RefractionIndex;

			double cos_theta = fmin(glm::dot(-glm::normalize(ray.Direction), payload.WorldNormal), 1.0);
			double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

			reflect = refractionIndex * sin_theta > 1.0;

			// Use Schlick's approximation for reflectance.
			float r0 = (1 - refractionIndex) / (1 + refractionIndex);
			r0 = r0 * r0;
			float reflectance = r0 + (1 - r0) * pow((1 - cos_theta), 5);
			reflect = reflect || (reflectance > Utils::RandomFloat());
		}

		if (reflect)
		{
			glm::vec3 randomBounceVec = Utils::RandomUnitVector();
			if (glm::dot(randomBounceVec, payload.WorldNormal) < 0.0) // In the same hemisphere as the normal
				randomBounceVec *= -1.0f;

			glm::vec3 diffuseDirection = glm::normalize(payload.WorldNormal + randomBounceVec);
			// Reflect(v, n) = v - 2 * dot(v, n) * n;
			glm::vec3 specularDirection = glm::reflect(ray.Direction, payload.WorldNormal);

			float specularBounce = (float)(mat.Specular > Utils::RandomFloat());

			float bounceBlend = (1.0f - mat.Roughness) * specularBounce;
			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.00001f;
			ray.Direction = (1.0f - bounceBlend) * diffuseDirection + bounceBlend * specularDirection;
			// rayColorMultiplier = (1.0f - specularBounce) * mat.Albedo + specularBounce * mat.SpecularColor;
			rayColorMultiplier = mat.Albedo;
		}
		else
		{
			ray.Origin = payload.WorldPosition + payload.WorldNormal * r;
			ray.Direction = glm::refract(glm::normalize(ray.Direction), payload.WorldNormal, refractionIndex);
		}

		// Blend colors
		incomingLight += emittedLight * rayColor;
		rayColor *= rayColorMultiplier;
	}
	return incomingLight;
}


void Renderer::TraceRay(const Ray& ray, Renderer::HitPayload& payload)
{
	/*
	Consider the equation a * t^2 + b * t + c = 0 we wrote below
	discriminant = b^2 - 4 * a * c
	solutions are (-b +- sqrt(discriminant)) / (2 * a)
	with b = 2 * h
	discriminant = 4 * (h^2 - a * c)
	solutions are (-2 * h +- 2 * sqrt(h^2 - a * c)) / (2 * a) = (-h +- sqrt(h^2 - a * c)) / a

	*/
	float a = glm::dot(ray.Direction, ray.Direction);

	payload.HitDistance = std::numeric_limits<float>::max();
	payload.ObjectIndex = -1;
	for (int i = 0; i < m_ActiveScene->Spheres.size(); i ++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		float radius = sphere.Radius;

		glm::vec3 cc = ray.Origin - sphere.Position;
		float h = glm::dot(cc, ray.Direction);
		// float b = 2 * glm::dot(cc, ray.Direction);
		float c = glm::dot(cc, cc) - radius * radius;

		float discriminant = h * h - a * c;

		if (discriminant < 0)
			continue;

		// Compute point on sphere
		// float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
		float t = (- h - sqrtf(discriminant)) / a;
		if (t > 0 && t < payload.HitDistance)
		{
			payload.HitDistance = t;
			payload.ObjectIndex = i;
		}
	}

	if (payload.ObjectIndex < 0)
		Miss(ray, payload);
	else
		ClosestHit(ray, payload);
}

void Renderer::ClosestHit(const Ray& ray, Renderer::HitPayload& payload)
{
	const Sphere& closestSphere = m_ActiveScene->Spheres[payload.ObjectIndex];
	payload.WorldPosition = ray.Origin + payload.HitDistance * ray.Direction;
	// payload.WorldNormal = glm::normalize(payload.WorldPosition - closestSphere.Position);
	payload.WorldNormal = (payload.WorldPosition - closestSphere.Position) / closestSphere.Radius;

}

void Renderer::Miss(const Ray& ray, Renderer::HitPayload& payload)
{
	payload.HitDistance = -1.0f;
}
