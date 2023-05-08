#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material
{
	glm::vec3 Albedo = glm::vec3(1.0f);
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	float Specular = 1.0f;
};


struct Sphere
{
	glm::vec3 Position = glm::vec3(0.0f);
	float Radius = 1.0f;

	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};