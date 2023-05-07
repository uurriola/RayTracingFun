#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Sphere
{
	glm::vec3 Position = glm::vec3(0.0f);
	float Radius = 1.0f;

	glm::vec3 Albedo = glm::vec3(1.0f);
};

struct Scene
{
	std::vector<Sphere> Spheres;
};