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

	glm::vec4* mergedImageData = new glm::vec4[m_FinalImage->GetWidth() * m_FinalImage->GetHeight()];
#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(),
		[this, mergedImageData](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(),
			[this, y, mergedImageData](uint32_t x)
				{
					mergedImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(x, y);
					/*glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
					finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f));
					mergedImageData[x + y * m_FinalImage->GetWidth()] = finalColor;*/
				}
			);
		}
	);
#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			mergedImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(x, y);
			/*glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
			finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f));
			MergedImageData[x + y * m_FinalImage->GetWidth()] = finalColor;*/
		}
	}
#endif

#define ANTIALIASING 1
#if ANTIALIASING
	const uint32_t SAMPLES_PER_PIXEL = 50;
#else
	const uint32_t SAMPLES_PER_PIXEL = 0;
#endif

#if MT
	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(),
		[this, mergedImageData, SAMPLES_PER_PIXEL](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(),
			[this, y, mergedImageData, SAMPLES_PER_PIXEL](uint32_t x)
				{
					glm::vec4 PixelColor = mergedImageData[x + y * m_FinalImage->GetWidth()];
					for (int sample = 0; sample < SAMPLES_PER_PIXEL; sample++) {
						uint32_t u = x + Walnut::Random::UInt(-5, 5);
						uint32_t v = y + Walnut::Random::UInt(-5, 5);

						if (u < 0)
							u = 0;
						else if (u >= m_FinalImage->GetWidth())
							u = m_FinalImage->GetWidth() - 1;
						if (v < 0)
							v = 0;
						else if (v >= m_FinalImage->GetHeight())
							v = m_FinalImage->GetHeight() - 1;

						PixelColor += mergedImageData[u + v * m_FinalImage->GetWidth()];
					}

					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += PixelColor / (float)(SAMPLES_PER_PIXEL + 1);
					glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;
					finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f));
					// Gamma 2 correction (sqrt)
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(glm::sqrt(finalColor));
				}
			);
		}
	);
#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 PixelColor = mergedImageData[x + y * m_FinalImage->GetWidth()];
			for (int sample = 0; sample < SAMPLES_PER_PIXEL; sample++) {
				uint32_t u = x + Walnut::Random::UInt(-1, 1);
				uint32_t v = y + Walnut::Random::UInt(-1, 1);

				if (u < 0)
					u = 0;
				else if (u >= m_FinalImage->GetWidth())
					u = m_FinalImage->GetWidth() - 1;
				if (v < 0)
					v = 0;
				else if (v >= m_FinalImage->GetHeight())
					v = m_FinalImage->GetHeight() - 1;
				
				PixelColor += mergedImageData[u + v * m_FinalImage->GetWidth()];
			}

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += PixelColor / (float) (SAMPLES_PER_PIXEL + 1);
			glm::vec4 finalColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float) m_FrameIndex;
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
	delete[] mergedImageData;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	glm::vec3 color(0.0f);
	float colorMultiplier = 0.0f;

	Renderer::HitPayload payload;

	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		TraceRay(ray, payload);

		if (payload.ObjectIndex < 0)
		{
			color = colorMultiplier * color + (1.0f - colorMultiplier) * skyColor;
			break;
		}

		// Shade depends on normal dot (point to light) ray
		// glm::vec3 lightToPoint = lightDirection;  // lightOrigin - point;
		// lightToPoint = glm::normalize(lightToPoint);
		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		float pointExposition = glm::max(glm::dot(-lightDirection, payload.WorldNormal), 0.05f);
		Material mat = m_ActiveScene->Materials[sphere.MaterialIndex];
		color = colorMultiplier * color + (1.0f - colorMultiplier) * pointExposition * mat.Albedo;

		// Blend rebound with current color
		colorMultiplier = 1.0f - (1.0f - colorMultiplier) * (0.1f + 0.5f * mat.Metallic);

		float r = 0.0001f;
		float refractionIndex = mat.Specular;

		if (glm::dot(ray.Direction, payload.WorldNormal) < 0)
			r *= -1.0;
		else
			refractionIndex = 1.0f / mat.Specular;

		double cos_theta = fmin(glm::dot(-glm::normalize(ray.Direction), payload.WorldNormal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool reflect = refractionIndex * sin_theta > 1.0;

		// Use Schlick's approximation for reflectance.
		float r0 = (1 - refractionIndex) / (1 + refractionIndex);
		r0 = r0 * r0;
		float reflectance = r0 + (1 - r0) * pow((1 - cos_theta), 5);
		reflect = reflect || reflectance > 0.5f * (Walnut::Random::Float() + 1.0f);

		if (reflect)
		{
			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
			glm::vec3 randomBounceVec(2.0f);  // = Walnut::Random::Vec3(-0.5f, 0.5f);
			while (glm::dot(randomBounceVec, randomBounceVec) >= 1.0f)
				randomBounceVec = Walnut::Random::Vec3(-1.0f, 1.0f);
			if (glm::dot(randomBounceVec, payload.WorldNormal) < 0.0) // In the same hemisphere as the normal
				randomBounceVec *= -1.0f;
			randomBounceVec = glm::normalize(randomBounceVec);
			// Reflect(v, n) = v - 2 * dot(v, n) * n;
			ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal) + mat.Roughness * randomBounceVec;
			// ray.Direction = randomBounceVec;
		}
		else
		{
			ray.Origin = payload.WorldPosition + payload.WorldNormal * r;
			ray.Direction = glm::refract(glm::normalize(ray.Direction), payload.WorldNormal, refractionIndex);
		}
	}

	return glm::vec4(color, 1.0f);

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
